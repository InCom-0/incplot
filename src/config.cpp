#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <expected>
#include <filesystem>

#include <optional>
#include <ranges>
#include <string_view>
#include <utility>

#include <indicators/color.hpp>
#include <indicators/cursor_control.hpp>
#include <indicators/progress_bar.hpp>

#include <config.hpp>
#include <incstd/core/filesys.hpp>

namespace incom::terminal_plot::config {

namespace fs = std::filesystem;

std::vector<std::byte> download_fileRaw(std::string_view url, bool indicator) {
    auto cb_writer = [](std::string_view data, intptr_t userdata) -> bool {
        std::vector<std::byte> *pf = reinterpret_cast<std::vector<std::byte> *>(userdata);
        auto v = std::views::transform(data, [](auto const &item) { return static_cast<std::byte>(item); });
        pf->insert(pf->end(), v.begin(), v.end());
        return true;
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
                    option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};

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
    if (indicator) { session.SetProgressCallback(cpr::ProgressCallback{cb_progress, 0}); }

    std::vector<std::byte> res{};
    if (auto resLength = session.GetDownloadFileLength(); resLength > 0) {
        res.reserve(static_cast<size_t>(resLength));
        show_console_cursor(false);
        session.Download(cpr::WriteCallback{cb_writer, reinterpret_cast<intptr_t>(&res)});
        show_console_cursor(true);
    }
    return res;
}


std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette16 &against) {
    colorCount_toValidate = std::min(static_cast<size_t>(colorCount_toValidate),
                                     std::tuple_size_v<typename std::remove_cvref_t<decltype(against)>>);
    for (size_t i = 0; i < colorCount_toValidate; ++i) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(i);
        if (not queryRes.has_value()) { return std::unexpected(queryRes.error()); }
        else if (queryRes.value() != against[i]) { return false; }
    }

    // If all pass then its validated
    return true;
}

std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette256 &against) {
    colorCount_toValidate = std::min(static_cast<size_t>(colorCount_toValidate),
                                     std::tuple_size_v<typename std::remove_cvref_t<decltype(against)>>);
    for (size_t i = 0; i < colorCount_toValidate; ++i) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(i);
        if (not queryRes.has_value()) { return std::unexpected(queryRes.error()); }
        else if (queryRes.value() != against[i]) { return false; }
    }

    // If all pass then its validated
    return true;
}
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(
    std::vector<std::uint8_t> colorIDs_toValidate, const inccol::palette256 &against) {
    for (auto colToVal : colorIDs_toValidate) {
        auto queryRes = inccons::ColorQuery::get_paletteIdx(colToVal);
        if (not queryRes.has_value()) { return std::unexpected(queryRes.error()); }
        else if (queryRes.value() != against[colToVal]) { return false; }
    }

    // If all pass then its validated
    return true;
}


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

std::string get_showSchemes(std::expected<sqlpp::sqlite3::connection, dbErr> &db) {
    std::string res(1, '\n');
    res.append("Integrated color schemes:\n");
    res.append(get_showInternalSchemes());

    if (db.has_value() && config::db::validate_configDB(db.value())) {
        res.append("\n\n").append("Config database color schemes:\n").append(get_showCongfigDBSchemes(db.value()));
    }
    res.append("\n");
    return res;
}

namespace db {
namespace detail {
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

std::expected<sqlpp::sqlite3::connection, dbErr> create_dbConnection_rw(fs::path const &pathToDb) {
    auto connConfig              = std::make_shared<sqlpp::sqlite3::connection_config>();
    connConfig->path_to_database = pathToDb.generic_string();
    connConfig->flags            = SQLITE_OPEN_READWRITE;

    sqlpp::sqlite3::connection dbOnDisk;
    try {
        dbOnDisk.connect_using(connConfig); // This can throw an exception.
    }
    catch (const sqlpp::sqlite3::exception &e) {
        return std::unexpected(dbErr::connectionError);
    }
    return dbOnDisk;
}
} // namespace detail

constexpr inline inccol::inc_sRGB decode_color(uint32_t const colInInt) {
    // Consider what to do when the uint32_t is not in the expected range for colors
    // Even if it isn't it is not catastrophic
    return inccol::inc_sRGB((colInInt >> 16) & 0xFF, (colInInt >> 8) & 0xFF, colInInt & 0xFF);
}
constexpr uint32_t encode_color(inccol::inc_sRGB const &srgb) {
    return (static_cast<uint32_t>(srgb.r) << 16) | (static_cast<uint32_t>(srgb.g) << 8) |
           (static_cast<uint32_t>(srgb.b));
}

std::expected<sqlpp::sqlite3::connection, dbErr> get_configConnection(const std::string_view &appName,
                                                                      const std::string_view &configFileName) {
    auto configPath_exp = incstd::filesys::find_configFile(std::string(appName), std::string(configFileName));
    if (configPath_exp.has_value()) {
        // Must use default 'in code' config since the sqlite config file is unavailable or somehow corrupted
        return detail::create_dbConnection_rw(configPath_exp.value());
    }
    else { return std::unexpected(dbErr::notFound); }
}

bool validate_configDB(sqlpp::sqlite3::connection &db) {
    if (not validate_SQLite_tableExistence(db, "schemes")) { return false; }
    if (not validate_SQLite_tableExistence(db, "scheme_palette")) { return false; }
    if (not validate_SQLite_tableExistence(db, "default_scheme")) { return false; }
    return true;
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


std::expected<scheme256, dbErr> get_scheme256(sqlpp::sqlite3::connection &dbConn, size_t const id) {
    return detail::get_scheme<scheme256>(dbConn, id);
}
std::expected<scheme16, dbErr> get_scheme16(sqlpp::sqlite3::connection &dbConn, size_t const id) {
    return detail::get_scheme<scheme16>(dbConn, id);
}
std::expected<scheme256, dbErr> get_scheme256(sqlpp::sqlite3::connection &dbConn, std::string const &name) {
    auto schemeID = detail::get_schemeID(dbConn, name);
    if (schemeID) { return detail::get_scheme<scheme256>(dbConn, schemeID.value()); }
    return std::unexpected(schemeID.error());
}
std::expected<scheme16, dbErr> get_scheme16(sqlpp::sqlite3::connection &dbConn, std::string const &name) {
    auto schemeID = detail::get_schemeID(dbConn, name);
    if (schemeID) { return detail::get_scheme<scheme16>(dbConn, schemeID.value()); }
    return std::unexpected(schemeID.error());
}


std::expected<scheme256, dbErr> get_defaultScheme256(sqlpp::sqlite3::connection &dbConn) {
    return detail::get_defaultScheme<scheme256>(dbConn);
}

std::expected<scheme16, dbErr> get_defaultScheme16(sqlpp::sqlite3::connection &dbConn) {
    return detail::get_defaultScheme<scheme16>(dbConn);
}

std::expected<size_t, dbErr> upsert_scheme256(sqlpp::sqlite3::connection &dbConn, scheme256 const &scheme) {
    return detail::upsert_scheme<scheme256>(dbConn, scheme);
}

std::expected<size_t, dbErr> upsert_scheme16(sqlpp::sqlite3::connection &dbConn, scheme16 const &scheme) {
    return detail::upsert_scheme<scheme16>(dbConn, scheme);
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

} // namespace db

} // namespace incom::terminal_plot::config