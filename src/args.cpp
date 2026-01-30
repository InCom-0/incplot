#include <algorithm>
#include <cctype>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <typeindex>

#include <args.hpp>
#include <config.hpp>
#include <incplot/plot_structures.hpp>
#include <incstd/core/typegen.hpp>
#include <incstd/incstd_console.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {
namespace {
std::optional<bool> prompt_yes_no(std::string_view question, bool defaultNo = true) {
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

std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct(argparse::ArgumentParser const &ap) {

    DesiredPlot::DP_CtorStruct nonDifferentiated;

    // Value options
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

    // Scheme and color options
    auto schemeGetter = [&]() -> void {
        auto setSchemeFromTermOrDefault = [&]() -> bool {
            auto schm_opt = config::get_schemeFromTerminal();
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
        auto dbConn = config::db::get_configConnection(config::appName, config::configFileName);
        if (dbConn.has_value() && config::db::validate_configDB(dbConn.value())) {
            if (ap.is_used("-l")) {
                // Path of explicitly specified theme
                auto schm_name = ap.get<std::string>("-l");
                auto schmExp   = config::db::get_scheme16(dbConn.value(), schm_name);

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
                auto lus_exp = config::db::get_defaultScheme16(dbConn.value());
                if (lus_exp) {
                    auto validated = config::validate_terminalPaletteSameness(3, lus_exp.value().palette);
                    if (not validated.has_value()) {
                        // Can't validate beacuse can't get color information out of terminal
                        // Must use the 'unvalidated' scheme from the config
                        nonDifferentiated.colScheme     = lus_exp.value();
                        nonDifferentiated.forceRGB_bool = true;
                        nonDifferentiated.additionalInfo.push_back(
                            std::string("Error encountered while querying terminal for colors.\nUsing default color "
                                        "scheme from config."));
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
                                    config::db::upsert_scheme16(dbConn.value(), nonDifferentiated.colScheme);
                            }
                            else {
                                nonDifferentiated.additionalInfo.push_back(std::string(
                                    "Error encountered while querying terminal for colors.\nUsing fall default color "
                                    "scheme."));
                            }
                        }
                    }
                }
                else if (lus_exp.error() == config::dbErr::notFound) {
                    if (setSchemeFromTermOrDefault()) {
                        nonDifferentiated.colScheme.name = "__fromTerminalScheme";
                        auto upsertedSchm = config::db::upsert_scheme16(dbConn.value(), nonDifferentiated.colScheme);

                        if (upsertedSchm) {
                            auto res = config::db::update_default(dbConn.value(), upsertedSchm.value());
                        }
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Error encountered while querying terminal for colors.\nUsing fall default color "
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


    // HTML mode options
    if (ap.get<bool>("-o")) {
        nonDifferentiated.htmlMode_bool = true;
        auto dbConn                     = config::db::get_configConnection(config::appName, config::configFileName);
        if (dbConn.has_value() && config::db::validate_configDB(dbConn.value())) {
            auto exp_fallBack_font = incom::terminal_plot::config::db::get_default_font(dbConn.value());

            // Happy path
            if (exp_fallBack_font.has_value()) {
                nonDifferentiated.htmlMode_ttfs_lastResort.push_back(std::move(exp_fallBack_font.value()));
            }

            else {
                // Less happy path, database is OK, but there is no default font in it
                // TODO: If we don't have a fallback font available we need to request to download it, download it,
                // store it in configDB, recheck, inform.
                if (exp_fallBack_font.error() == incom::terminal_plot::config::dbErr::notFound) {
                    auto consent =
                        prompt_yes_no(std::format("No fallback font configured for HTML output (normal on first time "
                                                  "use).\nDownload a fallback font now (Iosevka Nerd Font from {0} )?",
                                                  incom::terminal_plot::config::html_fallbackFont_URLsource),
                                      false);

                    if (! consent.has_value()) {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Unable to prompt for fallback font download. Proceeding without a fallback font."));
                    }
                    else if (consent.value()) {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "User approved fallback font download. (Download workflow not implemented yet.)"));
                        auto downloaded = incom::terminal_plot::config::download_fileRaw(
                            incom::terminal_plot::config::html_fallbackFont_URLsource);

                            // TODO: Need to extract the font we want from the file
                        std::vector<std::byte> extracted;

                        // Put the fallback into configDB
                        if (incom::terminal_plot::config::db::set_default_font(dbConn.value(), extracted).has_value()) {
                            nonDifferentiated.htmlMode_ttfs_lastResort.push_back(extracted);
                        }
                        else {
                            // configDB does not behave as expected ... should never ever happen
                            nonDifferentiated.additionalInfo.push_back(
                                std::string("Incplot config database is somehow corrupted on saving fallback font. "
                                            "Incplot will continue to work with limited functionality."));
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

    if (auto optVal = ap.present<std::vector<std::string>>("-f")) {
        // TODO: Functions to get the font: 1) from system, 2) from path, 3) from URL
        // TODO: Also need some way to differentiate between these
    }
    if (auto optVal = ap.present<size_t>("-z")) {
        if (optVal.value() > incom::terminal_plot::config::html_maxFontSize) {
            nonDifferentiated.additionalInfo.push_back(
                std::format("Font size ('-z') was set over its maximum allowed value which is {0}px. Claming the font "
                            "size value to {0}px.",
                            incom::terminal_plot::config::html_maxFontSize));

            nonDifferentiated.htmlMode_fontSize = incom::terminal_plot::config::html_maxFontSize;
        }
        else if (optVal.value() < incom::terminal_plot::config::html_minFontSize) {
            nonDifferentiated.additionalInfo.push_back(
                std::format("Font size ('-z') was set below its minimum allowed value which is {0}px. Claming the font "
                            "size value to {0}px.",
                            incom::terminal_plot::config::html_minFontSize));

            nonDifferentiated.htmlMode_fontSize = incom::terminal_plot::config::html_minFontSize;
        }
        else { nonDifferentiated.htmlMode_fontSize = optVal.value(); }
    }
    else { nonDifferentiated.htmlMode_fontSize = incom::terminal_plot::config::html_defaultFontSize; }


    // Plot type options
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

std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct() {
    return std::vector<DesiredPlot::DP_CtorStruct>{DesiredPlot::DP_CtorStruct{.plot_type_name = std::nullopt}};
}

void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup) {
    out_ap.add_description(
        "Draw coloured plots using unicode symbols inside terminal.\nCan also output the plot as visually identical "
        "self-contained html "
        "document.\n\nAutomatically infers what to display and "
        "how based on the shape of the data piped in.\nPipe in data in JSON, JSON Lines, NDJSON, CSV or TSV formats. "
        "All "
        "arguments "
        "are optional");

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
    out_ap.add_argument("-o", "--html")
        .help("Convert output into self-contained html [flag] (it is advisable to pipe the result into a file or into "
              "some other process when using this mode)")
        .flag()
        .nargs(0);
    out_ap.add_argument("-f", "--font")
        .help("Font to use inside html. Either: Family name of system installed font or Path to font file "
              "or Url to font file. Specify style name as second arg if/where relevant.")
        .nargs(1, 2);
    out_ap.add_argument("-z", "--font-size")
        .help("Specify font size to use (in pixels) [default = 12]")
        // .default_value<int>(12)
        .scan<'d', size_t>()
        .nargs(1);


    // Schemes and colors options
    auto &mex_grp = out_ap.add_mutually_exclusive_group();
    out_ap.add_group("Schemes and colors options:");
    mex_grp.add_argument("-l", "--color-scheme")
        .help("Specify color scheme by name (the scheme must exist in the config database)")
        .default_value<std::string>("")
        .nargs(1);
    mex_grp.add_argument("-d", "--default-colors").help("Draw with [d]efault colors (dimidium theme)").flag().nargs(0);
    out_ap.add_argument("-s", "--show-schemes").help("Show all available color [s]chemes and exit").flag().nargs(0);
    mex_grp.add_argument("-m", "--monochrome").help("Draw in [m]onochromatic colors").flag().nargs(0);

    out_ap.add_argument("-r", "--force-rgb")
        .help("Always use the RGB SGR way to output colors (virtually never necessary, exists for hypothetical "
              "compatibility reasons only)")
        .flag()
        .nargs(0);


    subap_setup.add_description("Various setup and maintenance functionality:");
    subap_setup.add_argument("-b", "--input-fallback-font")
        .help("Change the fallback font (identical argument logic as the --font argument in top level)")
        .nargs(1, 2);
    subap_setup.add_argument("-g", "--grab-termscheme")
        .help("Grabs current terminal color scheme and saves it. Optionally provide name for later usage using "
              "'-l'.")
        .nargs(0, 1);

    // TODO: Wierdly this does not actually create a working subparser ... need to investigate later.
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