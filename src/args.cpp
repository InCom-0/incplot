#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <typeindex>

#include <opentype-sanitiser.h>
#include <ots-memory-stream.h>

#include <incplot-lib/plot_structures.hpp>
#include <incplot/args.hpp>
#include <incplot/config.hpp>

#include <incfontdisc/incfontdisc.hpp>
#include <incstd/core/filesys.hpp>
#include <incstd/core/typegen.hpp>
#include <incstd/incstd_console.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {
namespace incplot = incom::terminal_plot;

namespace {

std::expected<std::vector<std::byte>, incerr_c> sanitize_fontOTS(std::span<const std::byte> bytes) {
    using enum incplot::Unexp_AP;
    if (bytes.empty()) { return std::unexpected(incerr_c::make(FONT_OTS_emptyInput)); }

    class QuietOtsContext final : public ots::OTSContext {
    public:
        void Message(int, const char *, ...) override {}
    };

    const auto inSize = bytes.size();
    size_t     limit  = inSize;
    if (inSize <= (std::numeric_limits<size_t>::max() / 4)) { limit = inSize * 4; }

    ots::ExpandingMemoryStream stream(inSize, limit);
    QuietOtsContext            context;

    auto *inData = reinterpret_cast<const uint8_t *>(bytes.data());
    if (! context.Process(&stream, inData, inSize)) {
        return std::unexpected(incerr_c::make(FONT_OTS_verificationProcessFailed));
    }

    auto outSizeOff = stream.Tell();
    if (outSizeOff < 0) { return std::unexpected(incerr_c::make(FONT_OTS_negativeOutputOffset)); }

    const auto outSize = static_cast<size_t>(outSizeOff);
    if (outSize == 0) { return std::unexpected(incerr_c::make(FONT_OTS_emptyOutput)); }

    std::vector<std::byte> sanitized(outSize);
    std::memcpy(sanitized.data(), stream.get(), outSize);
    return sanitized;
}

std::expected<std::vector<std::byte>, incerr_c> download_usingCPR(incstd::web::URI const &uri) {
    cpr::Session session;
    session.SetUrl(cpr::Url{uri.toString()});

    auto cb_writer = [](std::string_view data, intptr_t userdata) -> bool {
        std::vector<std::byte> *pf = reinterpret_cast<std::vector<std::byte> *>(userdata);
        auto v = std::views::transform(data, [](auto const &item) { return static_cast<std::byte>(item); });
        pf->insert(pf->end(), v.begin(), v.end());
        return true;
    };

    std::expected<std::vector<std::byte>, incerr_c> res = std::vector<std::byte>{};
    if (auto resLength = session.GetDownloadFileLength(); resLength > 0) {
        res.value().reserve(static_cast<size_t>(resLength));
    }
    else { res.value().reserve(4096 * 1024); }

    cpr::Response respo = session.Download(cpr::WriteCallback{cb_writer, reinterpret_cast<intptr_t>(&res)});
    if (respo.error.code != cpr::ErrorCode::OK) { res = std::unexpected(incerr_c::make(respo.error.code)); }

    return res;
}


std::optional<bool> prompt_YesNo(std::string_view question, bool defaultNo = true) {
    std::istream *in = nullptr;
    std::ifstream tty_in;

    if (incom::standard::console::is_stdin_inTerminal()) { in = &std::cin; }
    else {
#ifdef _WIN32
        tty_in.open("CONIN$");
#else
        tty_in.open("/dev/tty");
#endif
        if (tty_in.is_open()) { in = &tty_in; }
    }

    if (in == nullptr) { return std::nullopt; }

    auto trim = [](std::string &s) {
        auto not_space = [](unsigned char ch) { return ! std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    };

    for (;;) {
        std::cerr << question << (defaultNo ? " [y/N]: " : " [Y/n]: ") << std::flush;

        std::string line;
        if (! std::getline(*in, line)) { return std::nullopt; }

        trim(line);
        if (line.empty()) { return defaultNo ? std::optional<bool>(false) : std::optional<bool>(true); }

        std::transform(line.begin(), line.end(), line.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        if (line == "y" || line == "yes") { return true; }
        if (line == "n" || line == "no") { return false; }

        std::cerr << "Please answer with 'y' or 'n'.\n";
    }
}
} // namespace

std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStructs(
    argparse::ArgumentParser const &ap, std::expected<sqlpp::sqlite3::connection, incerr_c> &dbCon) {
    using enum incplot::Unexp_AP;

    DesiredPlot::DP_CtorStruct nonDifferentiated;

    // #####################################################################
    // ### Value and size options ###
    // #####################################################################
    if (auto optVal = ap.present<int>("-x")) { nonDifferentiated.lts_colID = optVal.value(); }
    if (auto optVal = ap.present<std::vector<int>>("-y")) {
        nonDifferentiated.v_colIDs = std::vector<size_t>(optVal.value().begin(), optVal.value().end());
    }
    if (auto optVal = ap.present<int>("-c")) { nonDifferentiated.c_colID = optVal.value(); }
    if (auto stddev = ap.present<int>("-e")) {
        if (stddev != 0) { nonDifferentiated.filter_outsideStdDev = stddev.value(); }
        else { nonDifferentiated.filter_outsideStdDev = std::nullopt; }
    }
    else { nonDifferentiated.filter_outsideStdDev = Config::filter_withinStdDevMultiple_default; }

    // Size options
    if (auto wdt = ap.present<int>("-w")) { nonDifferentiated.tar_width = wdt.value(); }
    if (auto hgt = ap.present<int>("-t")) { nonDifferentiated.tar_height = hgt.value(); }

    // #####################################################################
    // ### Scheme and color options ###
    // #####################################################################
    auto schemeGetter = [&]() -> void {
        auto setSchemeFromTermOrDefault = [&]() -> bool {
            auto schm_opt = config::scheme::get_schemeFromTerminal();
            if (schm_opt) {
                // Use the scheme obtained from the terminal
                nonDifferentiated.colScheme     = schm_opt.value();
                nonDifferentiated.forceRGB_bool = false;
            }
            else {
                // Can't get scheme out of terminal using ANSI control sequences
                // Must revert to default
                nonDifferentiated.colScheme     = config::default_scheme16;
                nonDifferentiated.forceRGB_bool = true;
            }
            return not(nonDifferentiated.forceRGB_bool.value());
        };
        if (dbCon.has_value() && config::db::validate_configDB(dbCon.value())) {
            if (ap.is_used("-l")) {
                // Path of explicitly specified theme
                auto schm_name = ap.get<std::string>("-l");
                auto schmExp   = config::db::get_scheme16(dbCon.value(), schm_name);

                if (not schmExp.has_value()) {
                    if (schmExp.error() == config::dbErr::notFound) {
                        nonDifferentiated.additionalInfo.push_back(
                            std::format("The specified scheme: '{}' was not found in the config database.", schm_name));
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(
                            std::string("Internal error of incplot config databse, which is probably unfixable by the "
                                        "user. Incplot will continue to work with limited functionality."));
                    }
                    // Set the default scheme
                    nonDifferentiated.colScheme     = config::default_scheme16;
                    nonDifferentiated.forceRGB_bool = true;
                }
                else {
                    nonDifferentiated.colScheme     = schmExp.value();
                    nonDifferentiated.forceRGB_bool = true;
                }
            }
            else {
                // Path of 'pure default'
                auto lus_exp = config::db::get_defaultScheme16(dbCon.value());
                if (lus_exp) {
                    auto validated = config::validate_terminalPaletteSameness(3, lus_exp.value().palette);
                    if (not validated.has_value()) {
                        // Can't validate beacuse can't get color information out of terminal
                        // Must use the 'unvalidated' scheme from the config
                        nonDifferentiated.colScheme     = lus_exp.value();
                        nonDifferentiated.forceRGB_bool = true;
                        nonDifferentiated.additionalInfo.push_back(std::format(
                            "{}\n\n{}{}\n{}{}\n{}{}\n\n{}", "Error encountered while querying terminal for colors."sv,
                            "The error category is: "sv, validated.error().category().name(), "The error code is: "sv,
                            validated.error().message(), "Error comment: "sv, validated.error().get_customMessage(),
                            "Using default color scheme from config."sv));
                    }
                    else {
                        if (validated.value()) {
                            // HAPPY PATH
                            nonDifferentiated.colScheme     = lus_exp.value();
                            nonDifferentiated.forceRGB_bool = false;
                        }
                        else {
                            // Validation result == false
                            if (setSchemeFromTermOrDefault()) {
                                nonDifferentiated.colScheme.name = config::fromTerminalSchemeName;
                                auto upsertedSchm =
                                    config::db::upsert_scheme16(dbCon.value(), nonDifferentiated.colScheme);
                            }
                            else {
                                nonDifferentiated.additionalInfo.push_back(std::string(
                                    "Error encountered while querying terminal for colors.\nUsing fallback default color "
                                    "scheme."));
                            }
                        }
                    }
                }
                else if (lus_exp.error() == config::dbErr::notFound) {
                    if (setSchemeFromTermOrDefault()) {
                        nonDifferentiated.colScheme.name = "__fromTerminalScheme";
                        auto upsertedSchm = config::db::upsert_scheme16(dbCon.value(), nonDifferentiated.colScheme);

                        if (upsertedSchm) {
                            auto res = config::db::update_default(dbCon.value(), upsertedSchm.value());
                        }
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Error encountered while querying terminal for colors.\nUsing fallback default color "
                            "scheme."));
                    }
                }
                else {
                    // Can't reach configDB or it is otherwise somehow corrupted
                    nonDifferentiated.additionalInfo.push_back(
                        std::string("Internal error of incplot config databse, which is probably unfixable by the "
                                    "user. Incplot will continue to work with limited functionality."));
                    if (not setSchemeFromTermOrDefault()) {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Error encountered while querying terminal for colors.\nUsing fallback default color "
                            "scheme."));
                    }
                }
            }
        }
        else {
            // Path of 'noDB' available or corrupted configDB
            nonDifferentiated.additionalInfo.push_back(
                std::string("Incplot config database does not seem to exist on your system or its internal structure "
                            "is somehow corrupted. Incplot will continue to work with limited functionality."));
            if (not setSchemeFromTermOrDefault()) {
                nonDifferentiated.additionalInfo.push_back(
                    std::string("Error encountered while querying terminal for colors.\nUsing fallback default color "
                                "scheme."));
            }
        }
    };

    if (ap.get<bool>("-d")) {
        nonDifferentiated.colScheme     = color_schemes::defaultScheme16;
        nonDifferentiated.forceRGB_bool = true;
    }
    else if (ap.get<bool>("-m")) {
        nonDifferentiated.colScheme     = color_schemes::other_sources::monochrome;
        nonDifferentiated.forceRGB_bool = true;
    }
    else { schemeGetter(); }
    if (ap.get<bool>("-r")) { nonDifferentiated.forceRGB_bool = true; }


    // #####################################################################
    // ### HTML mode options ###
    // #####################################################################
    if (ap.get<bool>("-o") || ap.get<bool>("-j")) {
        nonDifferentiated.htmlMode_bool       = (not ap.get<bool>("-j"));
        nonDifferentiated.htmlModeCanvas_bool = ap.get<bool>("-j");

        if (dbCon.has_value() && config::db::validate_configDB(dbCon.value())) {
            auto exp_fallBack_font = incom::terminal_plot::config::db::get_default_font(dbCon.value());

            // Happy path
            if (exp_fallBack_font.has_value()) {
                nonDifferentiated.htmlMode_ttfs_lastResort.push_back(std::move(exp_fallBack_font.value()));
            }

            else {
                // Less happy path, database is OK, but there is no default font in it
                if (exp_fallBack_font.error() == incom::terminal_plot::config::dbErr::notFound) {
                    auto consent =
                        prompt_YesNo(std::format("No fallback font configured for HTML output (normal on first time "
                                                 "use).\nDownload a fallback font now (Iosevka Nerd Font from {0} )?",
                                                 incom::terminal_plot::config::html_fallbackFont_URLsource),
                                     false);

                    if (! consent.has_value()) {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Unable to prompt for fallback font download. Proceeding without a fallback font."));
                    }
                    else if (consent.value()) {
                        auto downloaded = incom::terminal_plot::config::download_fileRaw(
                            incom::terminal_plot::config::html_fallbackFont_URLsource);

                        auto extracted =
                            incom::terminal_plot::config::extract_fromArchive(downloaded, [](archive_entry *item) {
                                if (archive_entry_pathname(item) ==
                                    incom::terminal_plot::config::html_fallbackFont_filePathInURLsource) {
                                    return true;
                                }
                                else { return false; }
                            });

                        // if (extracted.has_value() && extracted.value().size() == 1)
                        if (extracted.has_value()) {
                            if (auto sanitized = sanitize_fontOTS(extracted.value().front())) {
                                // Put the fallback into configDB
                                if (auto dfExp = incom::terminal_plot::config::db::set_default_font(
                                        dbCon.value(), sanitized.value())) {
                                    // Only if that succeeds do we set that font as 'last resort'
                                    nonDifferentiated.htmlMode_ttfs_lastResort.push_back(extracted.value().front());
                                    // std::cout << "LastResort set from download\n";
                                }
                                else { return std::unexpected(incerr_c::make(dfExp.error())); }
                            }
                            else { return std::unexpected(sanitized.error()); }
                        }
                        else {
                            // configDB does not behave as expected ... should never ever happen
                            return std::unexpected(extracted.error());
                        }
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(
                            std::string("User declined fallback font download. Proceeding without a fallback font."));
                    }
                }

                // Very bad, some database error that cannot be solved
                // Proceed but without fallback font ... all bets are off, inform the user
                else {
                    nonDifferentiated.additionalInfo.push_back(
                        std::string("Internal error of incplot config database while querying for a fallback font for "
                                    "html output. This is probably unfixable by the "
                                    "user. Incplot will continue to work with limited functionality."));
                }
            }
        }
        else {
            nonDifferentiated.additionalInfo.push_back(
                std::string("Internal error of incplot config database while querying for a fallback font for html "
                            "output. This is probably unfixable by the "
                            "user. Incplot will continue to work with limited functionality."));
        }
    }

    // Font option
    if (auto optVal = ap.present<std::vector<std::string>>("-f")) {
        // Only fetch font if we are actually outputting into HTML (otherwise skip)
        if (ap.get<bool>("-o") || ap.get<bool>("-j")) {
            if (optVal.value().size() == 0 || optVal.value().size() > 2) {
                // This should never ever happen as it is checked by argparse already
                return std::unexpected(incerr_c::make(DPCTOR_UnknownError));
            }
            std::string &reqFamilyName     = optVal.value().front();
            bool         styleSpecified_is = (optVal.value().size() == 2);

            auto uri = incstd::web::URI(reqFamilyName, false);

            if (not uri.getScheme().empty()) {
                // Use CPR

                if (auto sanitized = download_usingCPR(uri).and_then(sanitize_fontOTS)) {
                    nonDifferentiated.htmlMode_ttfs_toSubset.push_back(std::move(sanitized.value()));
                    // std::print("Using CPR\n");
                }
                else { return std::unexpected(sanitized.error()); }
            }
            else {
                // Use filesystem path
                auto pthToFont = std::filesystem::path(reqFamilyName);
                if (auto ca = incom::standard::filesys::check_access(pthToFont)) {
                    // File found, proceed to check whether we can read it
                    if (ca->readable) {
                        if (auto res = incstd::filesys::get_file_bytes(pthToFont.generic_string())) {
                            if (auto sanitized = sanitize_fontOTS(res.value())) {
                                nonDifferentiated.htmlMode_ttfs_toSubset.push_back(std::move(sanitized.value()));
                                // std::print("Using FS path\n");
                            }
                            else {
                                // std::print("Using FS path bad\n");
                                return std::unexpected(sanitized.error());
                            }
                        }
                        else { return std::unexpected(incerr_c::make(FONT_unknownErrorOnFileRead)); }
                    }
                    else {
                        // Unreadable file which exists
                        return std::unexpected(incerr_c::make(FONT_noReadAccessToFontFile));
                    }
                }

                // File not found if tried as path ... Try as if it were system font name
                else {
                    auto matched = incfontdisc::match_fonts(incfontdisc::FontQuery{
                        .family = reqFamilyName,
                        .style =
                            (styleSpecified_is ? std::optional<std::string>{optVal.value().at(1)} : std::nullopt)});

                    if (matched.has_value()) {
                        bool fontOK = false;
                        if (matched->family_score < config::html_fontFamilyMatch_minScore) {
                            nonDifferentiated.additionalInfo.push_back(
                                std::format("{}{}\n{}\n{}{}\n{}", "Tried selecting system font by the name of: "sv,
                                            reqFamilyName, "However, no such font exists on the system."sv,
                                            "The closest matching font on the system is: "sv, matched->font.family,
                                            "The font above will NOT be used."));
                        }

                        else if (matched->face_score < config::html_fontFaceMatch_minScore && styleSpecified_is) {
                            nonDifferentiated.additionalInfo.push_back(std::format(
                                "{}\n{}{}{}{}\n{}", "No such font/style exists on the system."sv,
                                "The closest matching font/style on the system is: "sv, matched->font.family, " : "sv,
                                matched->font.style, "The font above will NOT be used."));
                        }
                        else if (matched->face_score != 1.0f) {
                            nonDifferentiated.additionalInfo.push_back(std::format(
                                "{}\n{}{}{}{}\n{}",
                                "Requested font/style has a close enough but non-exact match on the system."sv,
                                "The closest matching font/style on the system is: "sv, matched->font.family, " : "sv,
                                matched->font.style, "The font above will be used."));
                            fontOK = true;
                        }
                        else { fontOK = true; }

                        if (fontOK) {
                            if (auto fnt = incfontdisc::load_font_data(matched->font.id)) {
                                // GET THE FONT
                                nonDifferentiated.htmlMode_ttfs_toSubset.push_back(std::move(fnt.value()));
                            }
                            else {
                                // Font data cannot be loaded, this should never happen and points to system failure
                                return std::unexpected(incerr_c::make(FONT_systemFontDiscoveredButImpossibleToLoad));
                            }
                        }
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(std::format(
                            "{}\n{}\n{}", "System font discovery failed"sv, "The specified font will not be used"sv,
                            "Incplot will continue to work without this feature"sv));
                    }
                }
            }
        }
        else {
            // No need to fetch the font if we are not actually outputting into HTML
            nonDifferentiated.additionalInfo.push_back(
                std::format("{}\n{}", "A font was specified while in non-HTML mode.",
                            "Fonts cannot be used in terminal only mode. Use either '-o' or '-j' to output HTML."));
        }
    }

    if (auto optVal = ap.present<size_t>("-z")) {
        if (optVal.value() > incom::terminal_plot::config::html_maxFontSize) {
            nonDifferentiated.additionalInfo.push_back(
                std::format("Font size ('-z') was set over its maximum allowed value which is {0}px. Clamping the font "
                            "size value to {0}px.",
                            incom::terminal_plot::config::html_maxFontSize));

            nonDifferentiated.htmlMode_fontSize = incom::terminal_plot::config::html_maxFontSize;
        }
        else if (optVal.value() < incom::terminal_plot::config::html_minFontSize) {
            nonDifferentiated.additionalInfo.push_back(std::format(
                "Font size ('-z') was set below its minimum allowed value which is {0}px. Clamping the font "
                "size value to {0}px.",
                incom::terminal_plot::config::html_minFontSize));

            nonDifferentiated.htmlMode_fontSize = incom::terminal_plot::config::html_minFontSize;
        }
        else { nonDifferentiated.htmlMode_fontSize = optVal.value(); }
    }
    else { nonDifferentiated.htmlMode_fontSize = incom::terminal_plot::config::html_defaultFontSize; }

    if (auto optVal = ap.present<std::vector<int>>("-n")) {
        if (optVal.value().size() < 3 || optVal.value().size() > 3) {
            // This should never ever happen as it is checked by argparse already
            return std::unexpected(incerr_c::make(DPCTOR_UnknownError));
        }

        if (std::ranges::any_of(optVal.value(), [](int oneVal) -> bool { return (oneVal < 0 || oneVal > 255); })) {
            return std::unexpected(incerr_c::make(COLOR_outOfBoundColorValue));
        }
        nonDifferentiated.colScheme_HTMLbackgroundOverride =
            inc_sRGB(optVal.value().at(0), optVal.value().at(1), optVal.value().at(2));
    }


    // Check whether we have at least some font available
    if (ap.get<bool>("-o") || ap.get<bool>("-j")) {
        if (nonDifferentiated.htmlMode_ttfs_lastResort.empty()) {
            nonDifferentiated.additionalInfo.push_back(
                std::format("{}\n{}", "No fallback font is available for use in HTML output mode.",
                            "Without a proper fallback including all the glyphs used by incplot, it is highly unlikely "
                            "that the output html will display correctly without artifacts."));
        }
    }


    // #####################################################################
    // ### Plot type options ###
    // #####################################################################
    std::vector<DesiredPlot::DP_CtorStruct> res;
    auto                                    addOne = [&](std::optional<std::type_index> const &&sv_opt) {
        res.push_back(nonDifferentiated);
        res.back().plot_type_name = sv_opt;
    };

    using namespace incom::standard::typegen;
    if (ap.get<bool>("-B")) { addOne(get_typeIndex<plot_structures::BarV>()); }
    if (ap.get<bool>("-S")) { addOne(get_typeIndex<plot_structures::Scatter>()); }
    if (ap.get<bool>("-L")) { addOne(get_typeIndex<plot_structures::Multiline>()); }
    if (ap.get<bool>("-V")) { addOne(get_typeIndex<plot_structures::BarVM>()); }
    if (ap.get<bool>("-H")) { addOne(get_typeIndex<plot_structures::BarHM>()); }
    if (ap.get<bool>("-T")) { addOne(get_typeIndex<plot_structures::BarHS>()); }
    if (res.empty()) { addOne(std::nullopt); }

    return res;
}

std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStructs() {
    return std::vector<DesiredPlot::DP_CtorStruct>{DesiredPlot::DP_CtorStruct{.plot_type_name = std::nullopt}};
}

std::expected<std::vector<std::string>, incerr_c> process_setupCommand(
    argparse::ArgumentParser const &setup_ap, std::expected<sqlpp::sqlite3::connection, incerr_c> &dbCon) {
    std::expected<std::vector<std::string>, incerr_c> res{};

    if (not dbCon.has_value()) { return std::unexpected(incerr_c::make(incplot::config::dbErr::connectionError)); }

    if (auto optVal = setup_ap.present<std::string>("-g")) {
        auto const &schmNameRef = optVal.value();
        if (schmNameRef.empty()) {
            return std::unexpected(incerr_c::make(incplot::Unexp_AP::SETUP_schemeGrab_withoutName));
        }
        else if (schmNameRef.size() > 128) {
            return std::unexpected(incerr_c::make(incplot::Unexp_AP::SETUP_schemeGrab_nameTooLong));
        }

        // The argument is there, the string isn't empty or too long.
        if (std::ranges::find(incplot::config::schemes_defaultSchemesNames, schmNameRef) !=
            incplot::config::schemes_defaultSchemesNames.end()) {
            return std::unexpected(incerr_c::make(incplot::Unexp_AP::SETUP_schemeGrab_nameSameAsBuildinScheme));
        }
        auto schm_opt = config::scheme::get_schemeFromTerminal();
        if (not schm_opt.has_value()) {
            return std::unexpected(incerr_c::make(incplot::Unexp_AP::SETUP_schemeGrab_errorWhenQueryingTerminal));
        }
        schm_opt->name = schmNameRef;

        if (auto maybe_alreadyExists = incplot::config::db::check_schemeExistsInDB(dbCon.value(), schm_opt.value())) {
            // If the optional inside the expected has value
            if (maybe_alreadyExists.value()) {
                res->push_back(std::format("Identical scheme already exists.\nScheme name: {}\n\n",
                                           maybe_alreadyExists.value().value()));
            }
            // If the optional is empty
            else {
                auto upsertRes = incom::terminal_plot::config::db::upsert_scheme16(dbCon.value(), schm_opt.value());

                if (upsertRes.has_value()) {
                    res->push_back(std::format("Scheme inserted\nNew scheme ID: {}\nNew scheme name: {}",
                                               upsertRes.value(), schmNameRef));
                }
                else { return std::unexpected(incerr_c::make(upsertRes.error())); }
            }
        }
        // else = the expected contains an error
        else {
            // Never happens because that function never returns unexpected
            return std::unexpected(incerr_c::make(maybe_alreadyExists.error()));
        }
    }

    if (setup_ap.is_used("-b")) {
        if (auto optVal = setup_ap.present<std::vector<std::string>>("-b")) {
            using enum incplot::Unexp_AP;

            // This should never ever happen as it is checked by argparse already
            if (optVal.value().size() > 2) { return std::unexpected(incerr_c::make(SETUP_fbFont_unknownError)); }

            // When the fallback font is somehow specified do the following
            else {
                std::string &reqFamilyName     = optVal.value().front();
                bool         styleSpecified_is = (optVal.value().size() == 2);

                auto uri = incstd::web::URI(reqFamilyName, false);

                if (not uri.getScheme().empty()) {
                    // Use CPR

                    if (auto sanitized = download_usingCPR(uri).and_then(sanitize_fontOTS)) {
                        //  Input the found font into the database
                        if (auto dfExp = config::db::set_default_font(dbCon.value(), sanitized.value())) {}
                        else { return std::unexpected(incerr_c::make(dfExp.error())); }
                    }
                    else { return std::unexpected(sanitized.error()); }
                }
                else {
                    // Use filesystem path
                    auto pthToFont = std::filesystem::path(reqFamilyName);
                    if (auto ca = incom::standard::filesys::check_access(pthToFont)) {
                        // File found, proceed to check whether we can read it
                        if (ca->readable) {
                            if (auto res = incstd::filesys::get_file_bytes(pthToFont.generic_string())) {
                                if (auto sanitized = sanitize_fontOTS(res.value())) {
                                    //  Input the found font into the database
                                    if (auto dfExp = config::db::set_default_font(dbCon.value(), sanitized.value())) {}
                                    else { return std::unexpected(incerr_c::make(dfExp.error())); }
                                }
                                else { return std::unexpected(sanitized.error()); }
                            }
                            else { return std::unexpected(incerr_c::make(SETUP_fbFont_unknownErrorOnFileRead)); }
                        }
                        else {
                            // Unreadable file which exists
                            return std::unexpected(incerr_c::make(SETUP_fbFont_noReadAccessToFontFile));
                        }
                    }

                    // File not found if tried as path ... Try as if it were system font name
                    else {
                        auto matched = incfontdisc::match_fonts(incfontdisc::FontQuery{
                            .family = reqFamilyName,
                            .style =
                                (styleSpecified_is ? std::optional<std::string>{optVal.value().at(1)} : std::nullopt)});

                        if (matched.has_value()) {
                            bool fontOK = false;
                            if (matched->family_score < config::html_fontFamilyMatch_minScore) {
                                res->push_back(
                                    std::format("{}{}\n{}\n{}{}\n{}", "Tried selecting system font by the name of: "sv,
                                                reqFamilyName, "However, no such font exists on the system."sv,
                                                "The closest matching font on the system is: "sv, matched->font.family,
                                                "The font above will NOT be used as fallback font."));
                            }

                            else if (matched->face_score < config::html_fontFaceMatch_minScore && styleSpecified_is) {
                                res->push_back(std::format("{}\n{}{}{}{}\n{}",
                                                           "No such font/style exists on the system."sv,
                                                           "The closest matching font/style on the system is: "sv,
                                                           matched->font.family, " : "sv, matched->font.style,
                                                           "The font above will NOT be used as fallback font."sv));
                            }
                            else if (matched->face_score != 1.0f || matched->family_score != 1.0f) {
                                res->push_back(std::format(
                                    "Requested font/style has a close enough but non-exact match on the system."sv));
                                fontOK = true;
                            }
                            else {
                                fontOK = true;
                                // Nothing, we have an exact font match
                            }

                            if (fontOK) {
                                if (auto fnt = incfontdisc::load_font_data(matched->font.id)) {
                                    //  Input the found font into the database
                                    if (auto dfExp = config::db::set_default_font(dbCon.value(), fnt.value())) {
                                        res->push_back(std::format(
                                            "{}{} : {} {}", "Font: "sv, matched->font.family, matched->font.style,
                                            "was stored in the database as a fallback font for future use"sv));
                                    }
                                    else { return std::unexpected(incerr_c::make(dfExp.error())); }
                                }
                                else {
                                    // Font data cannot be loaded, this should never happen and points to system failure
                                    return std::unexpected(
                                        incerr_c::make(FONT_systemFontDiscoveredButImpossibleToLoad));
                                }
                            }
                        }
                        else {
                            res->push_back(std::format("{}\n{}\n{}", "System font discovery failed"sv,
                                                       "The specified font will not be used"sv,
                                                       "Incplot will continue to work without this feature"sv));
                        }
                    }
                }
            }
        }
        else {
            // This is 'removing' the default font from the database (to allow redownloading the default one)
            if (auto remExp = config::db::remove_default_font(dbCon.value())) {
                res->push_back(std::format("Fallback font was removed.\nThis allows re-downloading of the default "
                                           "fallback when using '-o' or '-j' arguments."));
            }
            else { return std::unexpected(incerr_c::make(remExp.error())); }
        }
    }
    return res;
}


void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup) {
    out_ap.add_description("Draw coloured plots using unicode symbols inside terminal.\n"
                           "Can also output the plot as visually identical self-contained HTML document.\n\n"
                           "Pipe in data in JSON, JSON Lines, NDJSON, CSV or TSV formats.\n"
                           "Automatically infers what to display and how based on the shape of the data piped in.\n"
                           "All arguments are optional.");

    out_ap.add_group("Plot type options");
    out_ap.add_argument("-B", "--barV").help("Draw vertical [B]ar plot [flag]").flag().nargs(0);
    out_ap.add_argument("-S", "--scatter").help("Draw [S]catter plot [flag]").flag().nargs(0);
    out_ap.add_argument("-L", "--line").help("Draw (multi)[L]ine plot [flag]").flag().nargs(0);
    out_ap.add_argument("-V", "--barVM").help("Draw [V]ertical multibar plot [flag]").flag().nargs(0);
    out_ap.add_argument("-H", "--barHM").help("Draw [H]orizontal multibar plot [flag]").flag().nargs(0);
    out_ap.add_argument("-T", "--barHS").help("Draw horizontal s[T]acked bar plot [flag]").flag().nargs(0);

    // Value options
    out_ap.add_group("Values options");
    out_ap.add_argument("-x", "--main").help("Specify primary axis by column ID").nargs(1).scan<'d', int>();
    out_ap.add_argument("-y", "--values")
        .help("Specify secondary axis values by column IDs")
        .nargs(1, 6)
        .scan<'d', int>();
    out_ap.add_argument("-c", "--category")
        .help("Specify the column used to group the data (for Scatter plots)")
        .nargs(1)
        .scan<'d', int>();
    out_ap.add_argument("-e", "--filter-extremes")
        .help(std::format("Specify a multiple of standard deviation above and below which data is filtered ('0' means "
                          "no filtering) [default = {}]",
                          Config::filter_withinStdDevMultiple_default))
        .nargs(1)
        .scan<'d', int>();

    // Size options
    out_ap.add_group("Size options");
    out_ap.add_argument("-w", "--width").help("Requested width (in characters)").nargs(1).scan<'d', int>();
    out_ap.add_argument("-t", "--height").help("Requested height (in characters)").nargs(1).scan<'d', int>();

    // HTML Related
    out_ap.add_group("HTML output related:");
    auto &mexGrp_html = out_ap.add_mutually_exclusive_group();
    mexGrp_html.add_argument("-o", "--html")
        .help("Convert output into self-contained pure text html [flag]")
        .flag()
        .nargs(0);
    mexGrp_html.add_argument("-j", "--html-canvas")
        .help("Convert output into self-contained html, render using 2D canvas script [flag]")
        .flag()
        .nargs(0);

    out_ap.add_argument("-f", "--font")
        .help("Font to use inside html. Either: Family name of system installed font or Path to font file "
              "or URI to font file. Specify style name as second arg if/where relevant (ignored otherwise).")
        .nargs(1, 2);
    out_ap.add_argument("-z", "--font-size")
        .help("Specify font size to use (in pixels) [default = 12]")
        // .default_value<int>(12)
        .scan<'d', size_t>()
        .nargs(1);
    out_ap.add_argument("-n", "--override-background")
        .help("Override background color (RGB 0-255 values)")
        .nargs(3)
        .scan<'d', int>();


    // Schemes and colors options
    auto &mexGrp_color = out_ap.add_mutually_exclusive_group();
    out_ap.add_group("Schemes and colors options:");
    mexGrp_color.add_argument("-l", "--color-scheme")
        .help("Specify color scheme by name (the scheme must exist in the config database)")
        .default_value<std::string>("")
        .nargs(1);
    mexGrp_color.add_argument("-d", "--default-colors")
        .help("Draw with [d]efault colors (dimidium theme)")
        .flag()
        .nargs(0);
    mexGrp_color.add_argument("-m", "--monochrome").help("Draw in [m]onochromatic colors").flag().nargs(0);

    out_ap.add_argument("-s", "--show-schemes").help("Show all available color [s]chemes and exit").flag().nargs(0);
    out_ap.add_argument("-r", "--force-rgb")
        .help("Always use the RGB SGR way to output colors (virtually never necessary, exists for hypothetical "
              "compatibility reasons only)")
        .flag()
        .nargs(0);


    subap_setup.add_description("Various setup and maintenance functionality:");
    subap_setup.add_argument("-b", "--input-fallback-font")
        .help("Change the fallback font (identical argument logic as the --font argument in top level). Pass with no "
              "arguments to remove the fallback font (allows redownloading the default).")
        .nargs(0, 2);
    subap_setup.add_argument("-g", "--grab-termscheme")
        .help("Grabs current terminal color scheme and saves it. Optionally provide name for later usage using "
              "'-l'.")
        .nargs(1);

    // Add the pre-existing subparser (the parser subap_setup intantiated object exists in main)
    out_ap.add_subparser(subap_setup);
}

void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv) {
    try {
        out_ap.parse_args(argc, argv);
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << out_ap;
        std::exit(1);
    }
}
} // namespace cl_args
} // namespace terminal_plot
} // namespace incom