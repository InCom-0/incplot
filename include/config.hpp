#pragma once

#include <expected>
#include <filesystem>
#include <string_view>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>

#include <args.hpp>
#include <incstd/incstd_color.hpp>
#include <incstd/incstd_console.hpp>

#include <sqlitedefs.hpp>


namespace incom::terminal_plot::config {

namespace inccol  = incstd::color;
namespace inccons = incstd::console;
namespace fs      = std::filesystem;

inline constexpr color_schemes::scheme16 default_scheme16 = incstd::console::color_schemes::windows_terminal::campbell;
inline constexpr std::string_view        appName("incplot"sv);
inline constexpr std::string_view        configFileName("configDB.sqlite"sv);

enum class dbErr {
    impossibleNumberOfRecords = 1,
    impossibleValue,
    notFound,
    connectionError,
    dbAppearsCorrupted,
    unknownError,
};

constexpr inccol::inc_sRGB decode_color(uint32_t const colInInt);
constexpr uint32_t         encode_color(inccol::inc_sRGB const &srgb);


std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette16 &against);
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette256 &against);
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(
    std::vector<std::uint8_t> colorIDs_toValidate, const inccol::palette256 &against);

std::expected<sqlpp::sqlite3::connection, dbErr> create_dbConnection_rw(fs::path const &pathToDb);
bool        validate_SQLite_tableExistence(sqlpp::sqlite3::connection &db, std::string const &tableName);
// Must provide all colName and all colTypes in the right order
// Otherwise the func will return false
// template <typename SQLPP23TableInfo_t>
inline bool validate_SQLite_tableColNamesTypes(sqlpp::sqlite3::connection &db, std::string const &tableName,
                                               std::vector<std::string_view> const &colNames,
                                               std::vector<std::string_view> const &colTypes);
bool        validate_configDB(sqlpp::sqlite3::connection &db);


std::expected<inccons::color_schemes::scheme256, dbErr> get_lastUsedScheme_exp(sqlpp::sqlite3::connection &db);
std::expected<inccons::color_schemes::scheme16, dbErr>  get_lastUsedScheme16_exp(sqlpp::sqlite3::connection &db);
std::expected<inccons::color_schemes::scheme256, dbErr> get_lastUsedScheme256_exp(sqlpp::sqlite3::connection &db);

inccons::color_schemes::scheme256 get_defaultColScheme256();
inccons::color_schemes::scheme16  get_defaultColScheme16();
inccons::color_schemes::scheme256 get_monochromeColScheme256();
inccons::color_schemes::scheme16  get_monochromeColScheme16();

std::expected<sqlpp::sqlite3::connection, dbErr> maybeGet_configConnection(const std::string_view &appName,
                                                                           const std::string_view &configFileName);
std::expected<incstd::console::color_schemes::scheme16, dbErr> maybeGet_lastUsedScheme_db(
    sqlpp::sqlite3::connection &dbConn);

inccons::color_schemes::scheme16 get_colorScheme(argparse::ArgumentParser const &ap, const std::string_view &appName,
                                                 const std::string_view &configFileName);

std::optional<incstd::console::color_schemes::scheme16> maybeGet_schemeFromTerminal();

} // namespace incom::terminal_plot::config