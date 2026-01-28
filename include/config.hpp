#pragma once

#include <cstddef>
#include <expected>
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

using namespace incstd::console::color_schemes;

inline const color_schemes::scheme16 default_scheme16 = incstd::console::color_schemes::windows_terminal::campbell;
inline constexpr std::string_view    appName("incplot"sv);
inline constexpr std::string_view    configFileName("configDB.sqlite"sv);
inline constexpr std::string_view    fromTerminalSchemeName("__fromTerminalScheme"sv);

inline constexpr size_t html_defaultFontSize = 12uz; 
inline constexpr size_t html_minFontSize = 1uz; 
inline constexpr size_t html_maxFontSize = 256uz; 


enum class dbErr {
    impossibleNumberOfRecords = 1,
    impossibleValue,
    notFound,
    connectionError,
    dbAppearsCorrupted,
    missingData,
    unknownError,
};


std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette16 &against);
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette256 &against);
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(
    std::vector<std::uint8_t> colorIDs_toValidate, const inccol::palette256 &against);


scheme256 get_defaultColScheme256();
scheme16  get_defaultColScheme16();
scheme256 get_monochromeColScheme256();
scheme16  get_monochromeColScheme16();

std::optional<scheme16> get_schemeFromTerminal();

std::string get_showInternalSchemes();
std::string get_showCongfigDBSchemes(sqlpp::sqlite3::connection &db);

std::string get_showSchemes(std::expected<sqlpp::sqlite3::connection, dbErr> &db);

namespace db {

constexpr inccol::inc_sRGB decode_color(uint32_t const colInInt);
constexpr uint32_t         encode_color(inccol::inc_sRGB const &srgb);

bool                                             validate_configDB(sqlpp::sqlite3::connection &db);
std::expected<sqlpp::sqlite3::connection, dbErr> get_configConnection(const std::string_view &appName,
                                                                      const std::string_view &configFileName);

bool        validate_SQLite_tableExistence(sqlpp::sqlite3::connection &db, std::string const &tableName);
// Must provide all colName and all colTypes in the right order
// Otherwise the func will return false
// template <typename SQLPP23TableInfo_t>
inline bool validate_SQLite_tableColNamesTypes(sqlpp::sqlite3::connection &db, std::string const &tableName,
                                               std::vector<std::string_view> const &colNames,
                                               std::vector<std::string_view> const &colTypes);

std::expected<scheme256, dbErr> get_scheme256(sqlpp::sqlite3::connection &dbConn, size_t const id);
std::expected<scheme16, dbErr>  get_scheme16(sqlpp::sqlite3::connection &dbConn, size_t const id);

std::expected<scheme256, dbErr> get_scheme256(sqlpp::sqlite3::connection &dbConn, std::string const &name);
std::expected<scheme16, dbErr>  get_scheme16(sqlpp::sqlite3::connection &dbConn, std::string const &name);

std::expected<scheme256, dbErr> get_defaultScheme256(sqlpp::sqlite3::connection &dbConn);
std::expected<scheme16, dbErr>  get_defaultScheme16(sqlpp::sqlite3::connection &dbConn);

// size_t is whether a scheme of the same name was overwritten (true is overwritten, false if plain insert)
std::expected<size_t, dbErr> upsert_scheme256(sqlpp::sqlite3::connection &dbConn, scheme256 const &scheme);
std::expected<size_t, dbErr> upsert_scheme16(sqlpp::sqlite3::connection &dbConn, scheme16 const &scheme);

// Returned size_t is the id of the newly default scheme
std::expected<size_t, dbErr> update_default(sqlpp::sqlite3::connection &dbConn, std::string const &name);
std::expected<size_t, dbErr> update_default(sqlpp::sqlite3::connection &dbConn, size_t const id);

// Bool is true if the default was set (ie the default scheme was not set to NULL)
std::expected<bool, dbErr>   clear_defaultScheme(sqlpp::sqlite3::connection &dbConn);
// size_t is the ID of the scheme that was deleted (also sets the default scheme to NULL)
std::expected<size_t, dbErr> delete_defaultScheme(sqlpp::sqlite3::connection &dbConn);

std::expected<size_t, dbErr> delete_scheme(sqlpp::sqlite3::connection &dbConn, std::string const &name);
std::expected<size_t, dbErr> delete_scheme(sqlpp::sqlite3::connection &dbConn, size_t const id);


std::expected<size_t, dbErr> set_default_font(sqlpp::sqlite3::connection &dbConn, std::span<std::byte> ttf_font_raw);




} // namespace db

} // namespace incom::terminal_plot::config