#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>

#include <functional>
#include <incplot-lib.hpp>
#include <incplot-lib/err.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <fcntl.h>
#include <unistd.h>
#endif


#include <auto/platform_folder.hpp>
#include <indicators/color.hpp>
#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>

#include <incplot/config.hpp>
#include <incstd/console/colorschemes.hpp>
#include <incstd/core/filesys.hpp>


namespace incom::terminal_plot::config {

namespace fs = std::filesystem;

bool check_devBuildMarkerExists() {
    auto exeDirExp = incstd::filesys::get_curExeDir();
    if (! exeDirExp.has_value()) { return false; }

    fs::path const markerPath = exeDirExp.value() / std::string(devBuildMarkerFilename);

    std::error_code ec;
    if (! fs::exists(markerPath, ec)) { return false; }
    if (ec) { return false; }

    return true;
}

std::expected<fs::path, std::error_code> create_cPath_fromSamePrefix(fs::path srcPath, std::string_view srcPath_postfix,
                                                                     std::string_view resPath_postFix) {
    try {
        std::error_code ec;
        srcPath = fs::weakly_canonical(srcPath, ec);
        if (ec) { return std::unexpected(ec); }

        const fs::path fp_pf  = fs::path(srcPath_postfix).lexically_normal();
        const fs::path tp_pf  = fs::path(resPath_postFix).lexically_normal();
        fs::path       prefix = srcPath;

        if (! fp_pf.empty() && fp_pf != ".") {
            // TODO: Possibly figure out a better way to do this, this seems like a very hacky solution
            for (auto fakeRbegin = fp_pf.end(); fakeRbegin != fp_pf.begin();) {
                --fakeRbegin;
                auto ar = prefix.filename().generic_string();
                if (*fakeRbegin == prefix.filename()) {}
                else { return std::unexpected(std::make_error_code(std::errc::invalid_argument)); }

                if (prefix.has_parent_path()) { prefix = prefix.parent_path(); }
                else { return std::unexpected(std::make_error_code(std::errc::invalid_argument)); }
            }

            // std::filesystem-only validation (no custom suffix matcher).
            if ((prefix / fp_pf).lexically_normal() != srcPath.lexically_normal()) {
                return std::unexpected(std::make_error_code(std::errc::invalid_argument));
            }
        }

        fs::path result = fs::weakly_canonical(prefix / tp_pf, ec);
        if (ec) { return std::unexpected(ec); }
        return result;
    }
    catch (const std::bad_alloc &) {
        return std::unexpected(std::make_error_code(std::errc::not_enough_memory));
    }
    catch (const std::filesystem::filesystem_error &e) {
        if (e.code()) { return std::unexpected(e.code()); }
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
    catch (...) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
}


std::expected<fs::path, incerr_c> get_pth2InstalledRes(std::string_view                relInstallDirPth_toPrefix,
                                                       std::string_view                absInstallDirPth_whenInstalled,
                                                       std::string_view                relPth_toTarget,
                                                       std::optional<std::string_view> toInsert_projNameFolder,
                                                       std::optional<std::string_view> relPth_toDevMarker) {
    namespace fs = std::filesystem;

    if constexpr (incplot::platform_folders::portable_layout) {
        // If we are in 'portable layout mode', everything is always located in the dir of the excutable

        // Check if local version exists ... if it does then proceed
        // If there isnt a match overwrite with the seedDB and proceed
        return incstd::filesys::get_curExeDir()
            .transform_error([](auto const &err) { return incerr_c::make(dbErr::unknownError); })
            .transform([&](auto const &pth) { return pth / relPth_toTarget; });
    }
    else {
        auto exeDir_exp = incstd::filesys::get_curExeDir();
        if (exeDir_exp
                .transform([&](auto const &pth) {
                    if (not relPth_toDevMarker) { return false; }
                    else { return fs::exists(pth / relPth_toDevMarker.value()); }
                })
                .value_or(false)) {

            // If there is 'devBuildMaker', we look into the build directory
            // We base the returned path on mimicking install folder structure
            return create_cPath_fromSamePrefix(exeDir_exp.value(), incplot::platform_folders::rel_bindir,
                                               relInstallDirPth_toPrefix)
                .transform_error([](auto const &err) { return incerr_c::make(dbErr::unknownError); })
                .transform([&](auto const &pth) {
                    return pth / (toInsert_projNameFolder ? toInsert_projNameFolder.value() : ""sv) / relPth_toTarget;
                });
        }

        // If there is NOT 'devBuildMaker', it means we should be properly installed
        // Return 'baked-in' where the file is supposed to be installed
        else {
            auto res = fs::path(absInstallDirPth_whenInstalled);
            if (not res.is_absolute()) { return std::unexpected(incerr_c::make(dbErr::unknownError)); }

            return res / (toInsert_projNameFolder ? toInsert_projNameFolder.value() : ""sv) / relPth_toTarget;
        }
    }
}


std::vector<std::byte> download_fileRaw(std::string_view url, bool indicator) {
    auto cb_writer = [](std::string_view data, intptr_t userdata) -> bool {
        std::vector<std::byte> *pf = reinterpret_cast<std::vector<std::byte> *>(userdata);
        auto v = std::views::transform(data, [](auto const &item) { return static_cast<std::byte>(item); });
        pf->insert(pf->end(), v.begin(), v.end());
        return true;
    };

    class ScopedStdoutToConsole {
    public:
        ScopedStdoutToConsole() {
#if defined(_WIN32)
            oldOut_ = GetStdHandle(STD_OUTPUT_HANDLE);
            conOut_ = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                  OPEN_EXISTING, 0, nullptr);

            if (conOut_ != INVALID_HANDLE_VALUE) {
                // Make indicators' GetStdHandle(STD_OUTPUT_HANDLE) see a console
                switched_ = SetStdHandle(STD_OUTPUT_HANDLE, conOut_) != 0;
            }
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
            // Keep any prior stdout bytes on the original destination before redirecting.
            (void)std::fflush(stdout);
            oldOut_ = ::dup(STDOUT_FILENO);
            if (oldOut_ == -1) { return; }

            conOut_ = ::open("/dev/tty", O_WRONLY);
            if (conOut_ == -1) { return; }

            switched_ = (::dup2(conOut_, STDOUT_FILENO) != -1);
#else
        switched_ = false;
#endif
        }

        ~ScopedStdoutToConsole() {
#if defined(_WIN32)
            if (switched_ && oldOut_ && oldOut_ != INVALID_HANDLE_VALUE) { SetStdHandle(STD_OUTPUT_HANDLE, oldOut_); }
            if (conOut_ != INVALID_HANDLE_VALUE) { CloseHandle(conOut_); }
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
            // Ensure cursor control escape sequences are flushed to tty before restoring stdout.
            (void)std::fflush(stdout);
            if (switched_ && oldOut_ != -1) { (void)::dup2(oldOut_, STDOUT_FILENO); }
            if (conOut_ != -1) { (void)::close(conOut_); }
            if (oldOut_ != -1) { (void)::close(oldOut_); }
#else
        // No-op on unsupported platforms.
#endif
        }

        [[nodiscard]] bool is_active() const { return switched_; }

    private:
#if defined(_WIN32)
        HANDLE oldOut_{INVALID_HANDLE_VALUE};
        HANDLE conOut_{INVALID_HANDLE_VALUE};
#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
        int oldOut_{-1};
        int conOut_{-1};
#else
    int oldOut_{-1};
    int conOut_{-1};
#endif
        bool   switched_{false};
    };

    using namespace indicators;

    ProgressBar bar{option::BarWidth{50},
                    option::Start{"["},
                    option::Fill{"="},
                    option::Lead{">"},
                    option::Remainder{"-"},
                    option::End{"]"},
                    option::ForegroundColor{indicators::Color::cyan},
                    option::ShowElapsedTime{true},
                    option::ShowRemainingTime{true},
                    option::ShowPercentage{true},
                    option::PrefixText{"Downloading: "},
                    option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
                    option::Stream{std::cerr}};

    auto cb_progress = [&](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow, cpr::cpr_off_t, cpr::cpr_off_t,
                           intptr_t) -> bool {
        if (downloadTotal < 1 || downloadNow < 1 || bar.is_completed()) {}
        else {
            double pgr = static_cast<double>(downloadNow) / static_cast<double>(downloadTotal) * 100;
            if (pgr == 100) { bar.set_option(option::PostfixText{"Finished!"}); }
            bar.set_progress(pgr);
        }
        return true;
    };

    cpr::Session session;
    session.SetUrl(cpr::Url{url});

    std::vector<std::byte> res{};
    if (auto resLength = session.GetDownloadFileLength(); resLength > 0) {

        std::optional<ScopedStdoutToConsole> stdoutHack;
        if (indicator) {
            stdoutHack.emplace();
            indicator = stdoutHack->is_active();
        }
        if (indicator) { session.SetProgressCallback(cpr::ProgressCallback{cb_progress, 0}); }
        res.reserve(static_cast<size_t>(resLength));
        if (indicator) { show_console_cursor(false); }
        session.Download(cpr::WriteCallback{cb_writer, reinterpret_cast<intptr_t>(&res)});
        if (indicator) { show_console_cursor(true); }
    }
    return res;
}


std::expected<bool, incerr_c> validate_terminalPaletteSameness(std::uint8_t             colorCount_toValidate,
                                                               const inccol::palette16 &against) {
    colorCount_toValidate = std::min(static_cast<size_t>(colorCount_toValidate),
                                     std::tuple_size_v<typename std::remove_cvref_t<decltype(against)>>);
    for (size_t i = 0; i < colorCount_toValidate; ++i) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(i);
        if (not queryRes.has_value()) { return std::unexpected(incerr_c::make(queryRes.error())); }
        else if (queryRes.value() != against[i]) { return false; }
    }

    // If all pass then its validated
    return true;
}

std::expected<bool, incerr_c> validate_terminalPaletteSameness(std::uint8_t              colorCount_toValidate,
                                                               const inccol::palette256 &against) {
    colorCount_toValidate = std::min(static_cast<size_t>(colorCount_toValidate),
                                     std::tuple_size_v<typename std::remove_cvref_t<decltype(against)>>);
    for (size_t i = 0; i < colorCount_toValidate; ++i) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(i);
        if (not queryRes.has_value()) { return std::unexpected(incerr_c::make(queryRes.error())); }
        else if (queryRes.value() != against[i]) { return false; }
    }

    // If all pass then its validated
    return true;
}
std::expected<bool, incerr_c> validate_terminalPaletteSameness(std::vector<std::uint8_t> colorIDs_toValidate,
                                                               const inccol::palette256 &against) {
    for (auto colToVal : colorIDs_toValidate) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(colToVal);
        if (not queryRes.has_value()) { return std::unexpected(incerr_c::make(queryRes.error())); }
        else if (queryRes.value() != against[colToVal]) { return false; }
    }

    // If all pass then its validated
    return true;
}
namespace scheme {
inccons::color_schemes::scheme256 get_defaultColScheme256() {
    return color_schemes::defaultScheme256;
}
inccons::color_schemes::scheme16 get_defaultColScheme16() {
    return color_schemes::defaultScheme16;
}
inccons::color_schemes::scheme256 get_monochromeColScheme256() {
    // TEMP
    return inccons::color_schemes::scheme256{};
}
inccons::color_schemes::scheme16 get_monochromeColScheme16() {
    // TEMP
    return color_schemes::other_sources::monochrome;
}

std::optional<incstd::console::color_schemes::scheme16> get_schemeFromTerminal() {
    incstd::console::color_schemes::scheme16 res;
    for (size_t id = 0; id < std::tuple_size_v<decltype(res.palette)>; ++id) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(id);
        if (queryRes.has_value()) { res.palette[id] = std::move(queryRes.value()); }
        else { return std::nullopt; } // Aborts if cannot obtain any of the colors
    }

    // Does not abort if can't get foreground, background, cursor ... uses colors from scheme16 above instead
    auto queryRes = inccons::ColorQuery::get_foreground();
    if (queryRes.has_value()) { res.foreground = std::move(queryRes.value()); }
    else { res.foreground = res.palette.at(static_cast<int>(ANSI_Color16::Bright_White)); }

    queryRes = inccons::ColorQuery::get_background();
    if (queryRes.has_value()) { res.backgrond = std::move(queryRes.value()); }
    else { res.backgrond = res.palette.at(static_cast<int>(ANSI_Color16::Black)); }

    queryRes = inccons::ColorQuery::get_cursorCol();
    if (queryRes.has_value()) { res.cursor = std::move(queryRes.value()); }
    else { res.cursor = res.palette.at(static_cast<int>(ANSI_Color16::Bright_White)); }

    // Can't query selection color (to my knowledge)
    res.selection = res.cursor;
    return res;
}

std::string get_showScheme(scheme16 const &scheme) {
    constexpr std::string_view colorBlock = "██"sv;
    ANSI::SGR_builder          bldr;

    bldr.add_string(scheme.name.value_or(""));
    bldr.add_string("\n");

    for (size_t i = 0uz; i < 8uz; ++i) { bldr.color_fg(scheme.palette[i]).add_string(colorBlock); }
    bldr.reset_all();
    bldr.add_string("\n");

    for (size_t i = 8uz; i < 16uz; ++i) { bldr.color_fg(scheme.palette[i]).add_string(colorBlock); }
    bldr.reset_all();
    bldr.add_string("\n");

    bldr.color_fg(scheme.foreground).add_string(colorBlock);
    bldr.color_fg(scheme.backgrond).add_string(colorBlock);
    bldr.color_fg(scheme.cursor).add_string(colorBlock);
    bldr.color_fg(scheme.selection).add_string(colorBlock);

    bldr.reset_all();
    bldr.add_string("\n");

    return std::move(bldr).get();
}

std::string get_showInternalSchemes() {
    return get_showScheme(defaultScheme16).append("\n").append(get_showScheme(other_sources::monochrome));
}
std::string get_showCongfigDBSchemes(sqlpp::sqlite3::connection &db) {
    std::string        res;
    sqltables::Schemes sch{};

    for (auto const &schm_row : db(sqlpp::select(sch.schemeId, sch.name).from(sch).where(true))) {
        if (schm_row.name == fromTerminalSchemeName) { continue; }
        else {
            auto schm = db::get_scheme16(db, schm_row.schemeId.value());
            if (not schm.has_value()) { continue; }
            else {
                res.append(get_showScheme(schm.value()));
                res.push_back('\n');
            }
        }
    }
    return res;
}

std::string get_showSchemes(std::expected<sqlpp::sqlite3::connection, incerr_c> &db) {
    std::string res(1, '\n');
    res.append("Integrated color schemes:\n");
    res.append(get_showInternalSchemes());

    if (db.has_value() && config::db::validate_configDB(db.value())) {
        res.append("\n\n").append("Config database color schemes:\n").append(get_showCongfigDBSchemes(db.value()));
    }
    res.append("\n");
    return res;
}
} // namespace scheme

namespace db {
// Anonymous namespace for stuff used internally in this translation unit
namespace {
template <typename T>
std::expected<T, dbErr> get_defaultScheme(sqlpp::sqlite3::connection &db) {
    T res{};
    using namespace sqltables;

    DefaultScheme ds{};
    SchemePalette sp{};
    Schemes       sch{};

    for (size_t i = 0; auto const &dsID : db(sqlpp::select(ds.schemeId).from(ds))) {
        if (i++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        if (not dsID.schemeId.has_value()) { return std::unexpected(dbErr::notFound); }

        for (size_t      j = 0;
             auto const &schm : db(sqlpp::select(sch.name, sch.fgColor, sch.bgColor, sch.cursorColor, sch.selColor)
                                       .from(sch)
                                       .where(sch.schemeId == dsID.schemeId.value()))) {
            if (j++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
            res.name       = schm.name;
            res.foreground = decode_color(static_cast<uint32_t>(schm.fgColor));
            res.backgrond  = decode_color(static_cast<uint32_t>(schm.bgColor));
            res.cursor     = decode_color(static_cast<uint32_t>(schm.cursorColor));
            res.selection  = decode_color(static_cast<uint32_t>(schm.selColor));
        }

        auto palColors =
            db(sqlpp::select(sp.indexInPalette, sp.color).from(sp).where(sp.schemeId == dsID.schemeId.value()));
        for (size_t k = 0; auto const &palCol : palColors) {
            // More than 256 colors => impossible
            if (k >= std::tuple_size<decltype(res.palette)>{}) { break; }
            else if (palCol.indexInPalette < 0) { return std::unexpected(dbErr::impossibleValue); }
            // Color index not between 0 and 255 => impossible
            else if (palCol.indexInPalette >= std::tuple_size<decltype(res.palette)>{}) { continue; }
            // Color value is not legit
            else if (palCol.color < 0 || palCol.color > 16777215) { return std::unexpected(dbErr::impossibleValue); }

            // Happy path
            res.palette.at(palCol.indexInPalette) = decode_color(static_cast<uint32_t>(palCol.color));
            k++;
        }
        return res;
    }
    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}

std::expected<size_t, dbErr> get_schemeID(sqlpp::sqlite3::connection &db, std::string const &schemeName) {
    using namespace sqltables;
    Schemes sch{};
    for (auto const &schm : db(sqlpp::select(sch.schemeId).from(sch).where(sch.name == schemeName))) {
        return schm.schemeId.value();
    }
    return std::unexpected(dbErr::notFound);
}

template <typename T>
std::expected<T, dbErr> get_scheme(sqlpp::sqlite3::connection &db, size_t schemeID) {
    T res{};
    using namespace sqltables;

    SchemePalette sp{};
    Schemes       sch{};

    {
        size_t j = 0;
        for (auto const &schm :
             db(sqlpp::select(sch.schemeId, sch.name, sch.fgColor, sch.bgColor, sch.cursorColor, sch.selColor)
                    .from(sch)
                    .where(sch.schemeId == static_cast<int64_t>(schemeID)))) {
            if (j++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
            res.name       = schm.name;
            res.foreground = decode_color(static_cast<uint32_t>(schm.fgColor));
            res.backgrond  = decode_color(static_cast<uint32_t>(schm.bgColor));
            res.cursor     = decode_color(static_cast<uint32_t>(schm.cursorColor));
            res.selection  = decode_color(static_cast<uint32_t>(schm.selColor));
        }
        if (j != 1) { return std::unexpected(dbErr::notFound); }
    }

    for (size_t k = 0; auto const &palCol : db(sqlpp::select(sp.schemeId, sp.indexInPalette, sp.color)
                                                   .from(sp)
                                                   .where(sp.schemeId == static_cast<int64_t>(schemeID)))) {
        // More than 256 colors => impossible
        if (k >= std::tuple_size<decltype(res.palette)>{}) { break; }
        else if (palCol.indexInPalette < 0) { return std::unexpected(dbErr::impossibleValue); }
        // Color index not between 0 and 255 => impossible
        else if (palCol.indexInPalette >= std::tuple_size<decltype(res.palette)>{}) { continue; }
        // Color value is not legit
        else if (palCol.color < 0 || palCol.color > 16777215) { return std::unexpected(dbErr::impossibleValue); }

        // Happy path
        res.palette.at(palCol.indexInPalette) = decode_color(static_cast<uint32_t>(palCol.color));
        k++;
    }
    return res;

    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}

template <typename T>
std::expected<size_t, dbErr> upsert_scheme(sqlpp::sqlite3::connection &db, T const &scheme) {
    using namespace sqltables;
    SchemePalette sp{};
    Schemes       sch{};

    // Upsert into the 'schemes' table
    for (size_t i = 0; auto const &schID : db(sqlpp::sqlite3::insert_into(sch)
                                                  .set(sch.name = (scheme.name.has_value() ? scheme.name.value() : ""),
               sch.fgColor = encode_color(scheme.foreground), sch.bgColor = encode_color(scheme.backgrond),
               sch.cursorColor = encode_color(scheme.cursor), sch.selColor = encode_color(scheme.selection))
                                                  .returning(sch.schemeId)
                                                  .on_conflict(sch.name)
                                                  .do_update(sch.fgColor = encode_color(scheme.foreground),
  sch.bgColor = encode_color(scheme.backgrond), sch.cursorColor = encode_color(scheme.cursor),
  sch.selColor = encode_color(scheme.selection)))) {
        if (i++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }

        // Delete any color associated with the respective schemeID
        auto delRows = db(sqlpp::sqlite3::delete_from(sp).where(sp.schemeId == schID.schemeId.value())).affected_rows;
        auto multiInsert = sqlpp::sqlite3::insert_into(sp).columns(sp.schemeId, sp.indexInPalette, sp.color);

        for (size_t idInPal = 0; auto const &palCol : scheme.palette) {
            multiInsert.add_values(sp.schemeId       = schID.schemeId.value(),
                                   sp.indexInPalette = static_cast<int64_t>(idInPal++),
                                   sp.color          = encode_color(palCol));
        }
        // Execute multiinsert
        db(multiInsert);
        return schID.schemeId.value();
    }

    // This should be impossible as only reached if the 'schemes' table UPSERT for loop doesn't run at all
    return std::unexpected(dbErr::unknownError);
}
template <typename T>
std::expected<std::optional<std::string>, dbErr> check_schemeExistsInDB_T(sqlpp::sqlite3::connection &db,
                                                                          T const                    &scheme) {
    using namespace sqltables;

    SchemePalette sp{};
    Schemes       sch{};

    // For all color schemes
    for (auto const &schm :
         db(sqlpp::select(sch.schemeId, sch.name, sch.fgColor, sch.bgColor, sch.cursorColor, sch.selColor)
                .from(sch)
                .where(sch.name != incom::terminal_plot::config::fromTerminalSchemeName))) {
        // If all 4 base colors are the same
        if (scheme.foreground == decode_color(static_cast<uint32_t>(schm.fgColor)) &&
            scheme.backgrond == decode_color(static_cast<uint32_t>(schm.bgColor)) &&
            scheme.cursor == decode_color(static_cast<uint32_t>(schm.cursorColor)) &&
            scheme.selection == decode_color(static_cast<uint32_t>(schm.selColor))) {

            // Then also check all the colors in the palette
            std::array<bool, std::tuple_size_v<typename std::remove_cvref_t<decltype(scheme.palette)>>> checked{};
            bool allColorsIdentical =
                std::ranges::all_of(db(sqlpp::select(sp.schemeId, sp.indexInPalette, sp.color)
                                           .from(sp)
                                           .where(sp.schemeId == schm.schemeId)),
                                    [&](auto const &spColor) {
                                        if (spColor.indexInPalette >=
                                            std::tuple_size_v<typename std::remove_cvref_t<decltype(scheme.palette)>>) {
                                            return true;
                                        }
                                        else {
                                            checked.at(spColor.indexInPalette) = true;
                                            if (scheme.palette.at(spColor.indexInPalette) ==
                                                decode_color(static_cast<uint32_t>(spColor.color))) {
                                                return true;
                                            }
                                            else { return false; }
                                        }
                                    });
            // If all those are the same as well and no palette ID is missing in DB, then return the name of such
            // scheme in DB
            if (allColorsIdentical && std::ranges::fold_left(checked, true, std::logical_and{})) {
                return std::string{schm.name};
            }
        }
    }
    return std::nullopt;
}


std::expected<std::vector<std::array<std::string, 4>>, incerr_c> get_schemaRows(sqlpp::sqlite3::connection &db) {
    std::vector<std::array<std::string, 4>> rows;
    try {
        auto result = db(sqlpp::statement_t{} << sqlpp::verbatim("SELECT type, name, tbl_name, IFNULL(sql, '') AS sql "
                                                                 "FROM sqlite_schema "
                                                                 "WHERE name NOT LIKE 'sqlite_%' "
                                                                 "ORDER BY type, name, tbl_name, sql;")
                                              << with_result_type_of(sqlpp::select(
                                                     sqlpp::verbatim<sqlpp::text>("type").as(sqlpp::alias::a),
                                                     sqlpp::verbatim<sqlpp::text>("name").as(sqlpp::alias::b),
                                                     sqlpp::verbatim<sqlpp::text>("tbl_name").as(sqlpp::alias::c),
                                                     sqlpp::verbatim<sqlpp::text>("sql").as(sqlpp::alias::d))));

        for (auto const &row : result) {
            std::array<std::string, 4> schemaRow{};
            schemaRow[0] = std::string(row.a.value_or(""));
            schemaRow[1] = std::string(row.b.value_or(""));
            schemaRow[2] = std::string(row.c.value_or(""));
            schemaRow[3] = std::string(row.d.value_or(""));
            rows.push_back(std::move(schemaRow));
        }
    }
    catch (const sqlpp::sqlite3::exception &) {
        return std::unexpected(incerr_c::make(dbErr::dbAppearsCorrupted));
    }
    return rows;
}

std::expected<bool, incerr_c> schema_matches(sqlpp::sqlite3::connection &lhs, sqlpp::sqlite3::connection &rhs) {
    return get_schemaRows(lhs).and_then([&](std::vector<std::array<std::string, 4>> const &lhsRows) {
        return get_schemaRows(rhs).transform(
            [&](std::vector<std::array<std::string, 4>> const &rhsRows) { return lhsRows == rhsRows; });
    });
}

std::expected<int, incerr_c> copy_seedToUserDb(fs::path const &seedPath, fs::path const &targetPath) {
    std::error_code ec;
    fs::create_directories(targetPath.parent_path(), ec);
    if (ec) { return std::unexpected(incerr_c::make(dbErr::unknownError)); }

    fs::copy_file(seedPath, targetPath, fs::copy_options::overwrite_existing, ec);
    if (ec) { return std::unexpected(incerr_c::make(dbErr::unknownError)); }
    return 0;
}

} // namespace

std::expected<sqlpp::sqlite3::connection, incerr_c> create_dbConnection_rw(fs::path const &pathToDb) {
    auto connConfig              = std::make_shared<sqlpp::sqlite3::connection_config>();
    connConfig->path_to_database = pathToDb.generic_string();
    connConfig->flags            = SQLITE_OPEN_READWRITE;

    sqlpp::sqlite3::connection dbOnDisk;
    try {
        dbOnDisk.connect_using(connConfig); // This can throw an exception.
    }
    catch (const sqlpp::sqlite3::exception &e) {
        return std::unexpected(incerr_c::make(dbErr::connectionError));
    }
    return dbOnDisk;
}

std::expected<sqlpp::sqlite3::connection, incerr_c> create_dbConnection_ro(fs::path const &pathToDb) {
    auto connConfig              = std::make_shared<sqlpp::sqlite3::connection_config>();
    connConfig->path_to_database = pathToDb.generic_string();
    connConfig->flags            = SQLITE_OPEN_READONLY;

    sqlpp::sqlite3::connection dbOnDisk;
    try {
        dbOnDisk.connect_using(connConfig); // This can throw an exception.
    }
    catch (const sqlpp::sqlite3::exception &) {
        return std::unexpected(incerr_c::make(dbErr::connectionError));
    }
    return dbOnDisk;
}

std::expected<fs::path, incerr_c> getPath_configDB() {
    auto const localDataDir = incstd::filesys::locations::data_dir(false)
                                  .transform([](auto const &pth) { return pth.generic_string(); })
                                  .value_or("../");

    return incplot::config::get_pth2InstalledRes(incplot::platform_folders::rel_datadir, localDataDir,
                                                 incplot::config::configDBFileName, incplot::config::appName,
                                                 incplot::config::devBuildMarkerFilename);
}
std::expected<fs::path, incerr_c> getPath_configSeedDB() {
    return incplot::config::get_pth2InstalledRes(
        incplot::platform_folders::rel_datadir, incplot::platform_folders::install_datadir,
        incplot::config::configSeedDBFileName, incplot::config::appName, incplot::config::devBuildMarkerFilename);
}

std::expected<bool, incerr_c> check_is_configDBCurrent(fs::path const &configDB, fs::path const &seedConfigDB) {
    auto userDb_exp = create_dbConnection_ro(configDB);
    if (not userDb_exp.has_value()) { return std::unexpected(userDb_exp.error()); }
    auto seedDb_exp = create_dbConnection_ro(seedConfigDB);
    if (not seedDb_exp.has_value()) { return std::unexpected(seedDb_exp.error()); }

    auto schemaMatch_exp = schema_matches(userDb_exp.value(), seedDb_exp.value());
    if (not schemaMatch_exp.has_value()) { return std::unexpected(schemaMatch_exp.error()); }
    else if (not schemaMatch_exp.value()) { return false; }

    return true;
}

bool validate_configDB(sqlpp::sqlite3::connection &db) {
    if (not validate_SQLite_tableExistence(db, "schemes")) { return false; }
    if (not validate_SQLite_tableExistence(db, "scheme_palette")) { return false; }
    if (not validate_SQLite_tableExistence(db, "default_scheme")) { return false; }
    return true;
}

std::expected<fs::path, incerr_c> validate_configDBversion(fs::path const &configDBpth) {

    auto configSeedDBpth = getPath_configSeedDB();
    if (not configSeedDBpth.has_value()) { return std::unexpected(std::move(configSeedDBpth.error())); }

    if (not fs::exists(configSeedDBpth.value())) { return std::unexpected(incerr_c::make(dbErr::notFound)); }
    if (fs::exists(configDBpth)) {
        auto checkRes = check_is_configDBCurrent(configDBpth, configSeedDBpth.value());
        if (not checkRes.has_value()) { return std::unexpected(std::move(checkRes.error())); }
        if (checkRes.value() == true) { return configDBpth; }
    }

    // We overwrite the configDB with configSeedDB
    if (auto copyRes = copy_seedToUserDb(configSeedDBpth.value(), configDBpth); not copyRes) {
        return std::unexpected(std::move(copyRes.error()));
    }

    // ... and we return connection to it
    return configDBpth;
}


bool validate_SQLite_tableExistence(sqlpp::sqlite3::connection &db, std::string const &tableName) {
    using namespace sqlpp;

    // Table of 'tableName' does not exist in the db
    if (db(select(sqlpp::verbatim<sqlpp::text>("name").as(sqlpp::alias::a))
               .from(sqlpp::verbatim_table("sqlite_schema"))
               .where(sqlpp::verbatim<sqlpp::text>("name") == tableName))
            .empty()) {
        return false;
    }
    return true;
}

// Must provide all colName and all colTypes in the right order
// Otherwise the func will return false
// template <typename SQLPP23TableInfo_t>
bool validate_SQLite_tableColNamesTypes(sqlpp::sqlite3::connection &db, std::string const &tableName,
                                        std::vector<std::string_view> const &colNames,
                                        std::vector<std::string_view> const &colTypes) {
    using namespace sqlpp;

    // Table of 'tableName' does not exist in the db
    if (not validate_SQLite_tableExistence(db, tableName)) { return false; };

    auto pragmaInfo = incom::terminal_plot::config::sqltables::PragmaTableInfo{};
    auto rrr        = db(sqlpp::statement_t{} << sqlpp::verbatim("PRAGMA table_info(" + tableName + ");")
                                              << with_result_type_of(select(all_of(pragmaInfo))));

    for (size_t row_id = 0; auto const &row : rrr) {
        if (row_id < colNames.size() && row.name != colNames.at(row_id)) { return false; }
        if (row_id < colTypes.size() && row.type != colTypes.at(row_id)) { return false; }
    }
    return true;
}

constexpr inccol::inc_sRGB decode_color(uint32_t const colInInt) {
    // Consider what to do when the uint32_t is not in the expected range for colors
    // Even if it isn't it is not catastrophic
    return inccol::inc_sRGB((colInInt >> 16) & 0xFF, (colInInt >> 8) & 0xFF, colInInt & 0xFF);
}
constexpr uint32_t encode_color(inccol::inc_sRGB const &srgb) {
    return (static_cast<uint32_t>(srgb.r) << 16) | (static_cast<uint32_t>(srgb.g) << 8) |
           (static_cast<uint32_t>(srgb.b));
}

std::expected<scheme256, dbErr> get_scheme256(sqlpp::sqlite3::connection &dbConn, size_t const id) {
    return get_scheme<scheme256>(dbConn, id);
}
std::expected<scheme16, dbErr> get_scheme16(sqlpp::sqlite3::connection &dbConn, size_t const id) {
    return get_scheme<scheme16>(dbConn, id);
}
std::expected<scheme256, dbErr> get_scheme256(sqlpp::sqlite3::connection &dbConn, std::string const &name) {
    auto schemeID = get_schemeID(dbConn, name);
    if (schemeID) { return get_scheme<scheme256>(dbConn, schemeID.value()); }
    return std::unexpected(schemeID.error());
}
std::expected<scheme16, dbErr> get_scheme16(sqlpp::sqlite3::connection &dbConn, std::string const &name) {
    auto schemeID = get_schemeID(dbConn, name);
    if (schemeID) { return get_scheme<scheme16>(dbConn, schemeID.value()); }
    return std::unexpected(schemeID.error());
}


std::expected<scheme256, dbErr> get_defaultScheme256(sqlpp::sqlite3::connection &dbConn) {
    return get_defaultScheme<scheme256>(dbConn);
}

std::expected<scheme16, dbErr> get_defaultScheme16(sqlpp::sqlite3::connection &dbConn) {
    return get_defaultScheme<scheme16>(dbConn);
}

std::expected<size_t, dbErr> upsert_scheme256(sqlpp::sqlite3::connection &dbConn, scheme256 const &scheme) {
    return upsert_scheme<scheme256>(dbConn, scheme);
}

std::expected<size_t, dbErr> upsert_scheme16(sqlpp::sqlite3::connection &dbConn, scheme16 const &scheme) {
    return upsert_scheme<scheme16>(dbConn, scheme);
}

std::expected<size_t, dbErr> update_default(sqlpp::sqlite3::connection &dbConn, std::string const &name) {
    using namespace sqltables;
    Schemes       sch{};
    DefaultScheme dsch{};

    for (size_t id = 0; auto const &row : dbConn(sqlpp::select(sch.schemeId).from(sch).where(sch.name == name))) {
        if (id++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        else if (not row.schemeId.has_value()) { return std::unexpected(dbErr::impossibleValue); }

        dbConn(sqlpp::sqlite3::update(dsch).set(dsch.schemeId = row.schemeId.value()).where(true));
        return row.schemeId.value();
    }
    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}
std::expected<size_t, dbErr> update_default(sqlpp::sqlite3::connection &dbConn, size_t const schm_id) {
    using namespace sqltables;
    Schemes       sch{};
    DefaultScheme dsch{};

    for (size_t      id = 0;
         auto const &row :
         dbConn(sqlpp::select(sch.schemeId).from(sch).where(sch.schemeId == static_cast<int64_t>(schm_id)))) {
        if (id++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        else if (not row.schemeId.has_value()) { return std::unexpected(dbErr::impossibleValue); }

        dbConn(sqlpp::sqlite3::update(dsch).set(dsch.schemeId = static_cast<int64_t>(schm_id)).where(true));
        return schm_id;
    }

    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::notFound);
}

std::expected<bool, dbErr> clear_defaultScheme(sqlpp::sqlite3::connection &dbConn) {
    using namespace sqltables;

    DefaultScheme ds{};
    bool          res = false;
    for (size_t j = 0; auto const &defSchm : dbConn(sqlpp::select(ds.id, ds.schemeId).from(ds).where(true))) {
        if (j++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        res = defSchm.schemeId.has_value();
        dbConn(sqlpp::sqlite3::update(ds).set(ds.schemeId = std::nullopt).where(true));
        return res;
    }

    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}

std::expected<size_t, dbErr> delete_defaultScheme(sqlpp::sqlite3::connection &dbConn) {
    using namespace sqltables;

    DefaultScheme ds{};
    Schemes       sch{};
    for (size_t j = 0; auto const &defSchm : dbConn(sqlpp::select(ds.id, ds.schemeId).from(ds).where(true))) {
        if (j++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        else if (not defSchm.schemeId.has_value()) { return std::unexpected(dbErr::missingData); }
        dbConn(sqlpp::sqlite3::delete_from(sch).where(sch.schemeId == defSchm.schemeId.value()));
    }

    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}

std::expected<size_t, dbErr> delete_scheme(sqlpp::sqlite3::connection &dbConn, std::string const &name) {
    using namespace sqltables;
    Schemes sch{};

    for (size_t id = 0; auto const &row : dbConn(sqlpp::select(sch.schemeId).from(sch).where(sch.name == name))) {
        if (id++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        else if (not row.schemeId.has_value()) { return std::unexpected(dbErr::impossibleValue); }

        dbConn(sqlpp::sqlite3::delete_from(sch).where(sch.schemeId == row.schemeId.value()));
        return row.schemeId.value();
    }

    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}
std::expected<size_t, dbErr> delete_scheme(sqlpp::sqlite3::connection &dbConn, size_t const schm_id) {
    using namespace sqltables;
    Schemes sch{};

    for (size_t      i = 0;
         auto const &row :
         dbConn(sqlpp::select(sch.schemeId).from(sch).where(sch.schemeId == static_cast<int64_t>(schm_id)))) {
        if (i++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        else if (not row.schemeId.has_value()) { return std::unexpected(dbErr::impossibleValue); }

        dbConn(sqlpp::sqlite3::delete_from(sch).where(sch.schemeId == row.schemeId.value()));
        return schm_id;
    }


    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}

std::expected<std::optional<std::string>, dbErr> check_schemeExistsInDB(sqlpp::sqlite3::connection &dbConn,
                                                                        scheme256 const            &scheme) {
    return check_schemeExistsInDB_T(dbConn, scheme);
}
std::expected<std::optional<std::string>, dbErr> check_schemeExistsInDB(sqlpp::sqlite3::connection &dbConn,
                                                                        scheme16 const             &scheme) {
    return check_schemeExistsInDB_T(dbConn, scheme);
}


std::expected<std::vector<std::byte>, dbErr> get_default_font(sqlpp::sqlite3::connection &dbConn) {
    using namespace sqltables;
    DefaultFont df{};

    for (size_t i = 0; auto const &dfID : dbConn(sqlpp::select(df.content).from(df))) {
        if (i++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
        if (not dfID.content.has_value()) { return std::unexpected(dbErr::notFound); }
        auto view_ret =
            std::views::transform(dfID.content.value(), [](auto const item) { return static_cast<std::byte>(item); });
        return std::vector<std::byte>(view_ret.begin(), view_ret.end());
    }
    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}

std::expected<size_t, dbErr> set_default_font(sqlpp::sqlite3::connection &dbConn, std::span<std::byte> ttf_font_raw) {
    using namespace sqltables;
    DefaultFont dfnt_tbl{};

    auto rs = dbConn(sqlpp::sqlite3::update(dfnt_tbl)
                         .set(dfnt_tbl.content = std::span<std::uint8_t>(
                                  reinterpret_cast<std::uint8_t *>(ttf_font_raw.data()), ttf_font_raw.size()))
                         .where(true));

    if (rs.affected_rows == 1) { return 0uz; }
    else { return std::unexpected(dbErr::impossibleNumberOfRecords); }
}

std::expected<size_t, dbErr> remove_default_font(sqlpp::sqlite3::connection &dbConn) {
    using namespace sqltables;
    DefaultFont dfnt_tbl{};

    auto rs = dbConn(sqlpp::sqlite3::update(dfnt_tbl).set(dfnt_tbl.content = std::nullopt).where(true));

    if (rs.affected_rows == 1) { return 0uz; }
    else { return std::unexpected(dbErr::impossibleNumberOfRecords); }
}

} // namespace db

} // namespace incom::terminal_plot::config