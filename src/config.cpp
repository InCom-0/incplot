#include <filesystem>

#include <config.hpp>
#include <incstd/core/filesys.hpp>
#include <optional>


namespace incom::terminal_plot::config {

namespace fs = std::filesystem;

constexpr inline inccol::inc_sRGB decode_color(uint32_t const colInInt) {
    // Consider what to do when the uint32_t is not in the expected range for colors
    // Even if it isn't it is not catastrophic
    return inccol::inc_sRGB((colInInt >> 16) & 0xFF, (colInInt >> 8) & 0xFF, colInInt & 0xFF);
}
constexpr uint32_t encode_color(inccol::inc_sRGB const &srgb) {
    return (static_cast<uint32_t>(srgb.r) << 16) | (static_cast<uint32_t>(srgb.g) << 8) |
           (static_cast<uint32_t>(srgb.b));
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

bool validate_configDB(sqlpp::sqlite3::connection &db) {
    if (not validate_SQLite_tableExistence(db, "schemes")) { return false; }
    if (not validate_SQLite_tableExistence(db, "scheme_palette")) { return false; }
    if (not validate_SQLite_tableExistence(db, "default_scheme")) { return false; }
    return true;
}


std::expected<inccons::color_schemes::scheme256, dbErr> get_lastUsedScheme_exp(sqlpp::sqlite3::connection &db) {
    inccons::color_schemes::scheme256 res{};
    using namespace sqltables;

    DefaultScheme ds{};
    SchemePalette sp{};
    Schemes       sch{};

    for (size_t i = 0; auto const &dsID : db(sqlpp::select(ds.schemeId).from(ds))) {
        if (i++ != 0 || not dsID.schemeId.has_value()) { return std::unexpected(dbErr::impossibleNumberOfRecords); }

        auto schemeObj = db(sqlpp::select(sch.name, sch.fgColor, sch.bgColor, sch.cursorColor, sch.selColor)
                                .from(sch)
                                .where(sch.schemeId == dsID.schemeId.value()));

        for (size_t j = 0; auto const &schm : schemeObj) {
            if (j++ != 0) { return std::unexpected(dbErr::impossibleNumberOfRecords); }
            res.foreground = decode_color(static_cast<uint32_t>(schm.fgColor));
            res.backgrond  = decode_color(static_cast<uint32_t>(schm.bgColor));
            res.cursor     = decode_color(static_cast<uint32_t>(schm.cursorColor));
            res.selection  = decode_color(static_cast<uint32_t>(schm.selColor));
        }

        auto palColors =
            db(sqlpp::select(sp.indexInPalette, sp.color).from(sp).where(sp.schemeId == dsID.schemeId.value()));
        for (size_t k = 0; auto const &palCol : palColors) {
            // More than 256 colors => impossible
            if (k++ >= std::tuple_size<decltype(res.palette)>{}) {
                return std::unexpected(dbErr::impossibleNumberOfRecords);
            }
            // Color index not between 0 and 255 => impossible
            else if (palCol.indexInPalette < 0 || palCol.indexInPalette > 255) {
                return std::unexpected(dbErr::impossibleNumberOfRecords);
            }
            // Color value is not legit
            else if (palCol.color < 0 || palCol.color > 16777215) { return std::unexpected(dbErr::impossibleValue); }

            // Happy path
            res.palette.at(palCol.indexInPalette) = decode_color(static_cast<uint32_t>(palCol.color));
        }
        return res;
    }
    // If the above for loop does not run at all that means there are 0 records in the DefaultScheme table
    // That is impossible
    return std::unexpected(dbErr::impossibleNumberOfRecords);
}


inccons::color_schemes::scheme256 get_defaultColScheme256() {
    return color_schemes::defaultScheme256;
}
inccons::color_schemes::scheme16 get_defaultColScheme16() {
    return default_scheme16;
}
inccons::color_schemes::scheme256 get_monochromeColScheme256() {
    // TEMP
    return inccons::color_schemes::scheme256{};
}
inccons::color_schemes::scheme16 get_monochromeColScheme16() {
    // TEMP
    return inccons::color_schemes::scheme16{};
}

std::optional<incstd::console::color_schemes::scheme16> maybeGet_lastUsedScheme(
    const std::string_view &appName, const std::string_view &configFileName) {
    auto configPath_exp = incstd::filesys::find_configFile(std::string(appName), std::string(configFileName));
    if (configPath_exp.has_value()) {
        // Must use default 'in code' config since the sqlite config file is unavailable or somehow corrupted
        if (auto sqlConn_exp = incom::terminal_plot::config::create_dbConnection_rw(configPath_exp.value());
            sqlConn_exp.has_value()) {
            auto &sqlConn = sqlConn_exp.value();
            // Must use default 'in code' config since the sqlite configDB is unavailable or somehow corrupted
            if (incom::terminal_plot::config::validate_configDB(sqlConn)) {
                if (auto defScheme = incom::terminal_plot::config::get_lastUsedScheme_exp(sqlConn).transform(
                        color_schemes::conv_s256s16)) {
                    if (incom::terminal_plot::config::validate_terminalPaletteSameness(3, defScheme.value().palette)) {
                        return defScheme.value();
                    }
                }
            }
        }
    }
    // If we fallthrough anywhere, then we can't get 'lastUsed' scheme and return nullopt
    return std::nullopt;
}

inccons::color_schemes::scheme16 get_colorScheme(argparse::ArgumentParser const &ap, const std::string_view &appName,
                                                 const std::string_view &configFileName) {
    auto maybeDefault = [&]() -> std::optional<incstd::console::color_schemes::scheme16> {
        // Requesting default colors
        if (ap.get<bool>("-d")) { return get_defaultColScheme16(); }
        else { return std::nullopt; }
    };
    auto maybeMonochrome = [&]() -> std::optional<incstd::console::color_schemes::scheme16> {
        // Requesting monochrome mode
        if (ap.get<bool>("-m")) { return get_monochromeColScheme16(); }
        else { return std::nullopt; }
    };
    auto maybeGetSchemeFromTerminal = [&]() -> std::optional<incstd::console::color_schemes::scheme16> {
        incstd::console::color_schemes::scheme16 res;
        for (size_t id = 0; id < std::tuple_size_v<decltype(res.palette)>; ++id) {
            auto queryRes = inccons::ColorQuery::get_paletteIdx(id);
            if (queryRes.has_value()) { res.palette[id] = std::move(queryRes.value()); }
            else { return std::nullopt; }
        }
        auto queryRes = inccons::ColorQuery::get_foreground();
        if (queryRes.has_value()) { res.foreground = std::move(queryRes.value()); }
        queryRes = inccons::ColorQuery::get_background();
        if (queryRes.has_value()) { res.backgrond = std::move(queryRes.value()); }
        // Incomplete because the cursor colour and selection colors are not set ...
        // But for the time being it doesn't really matter.
        return res;
    };

    return maybeDefault()
        .or_else(maybeMonochrome)
        .or_else(std::bind(maybeGet_lastUsedScheme, appName, configFileName))
        .or_else(maybeGetSchemeFromTerminal)
        .value_or(color_schemes::defaultScheme16);
}


} // namespace incom::terminal_plot::config