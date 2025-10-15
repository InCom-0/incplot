#pragma once

#include <expected>
#include <filesystem>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>

#include <incstd/incstd_color.hpp>
#include <incstd/incstd_console.hpp>
#include <sqlitedefs.hpp>


namespace incom::terminal_plot::config {

namespace inccol  = incstd::color;
namespace inccons = incstd::console;
namespace fs      = std::filesystem;

enum class dbErr {
    impossibleNumberOfRecords = 1,
    impossibleValue,
    notFound,
    unknownError,
};

constexpr inccol::inc_sRGB decode_color(uint32_t const colInInt);
constexpr uint32_t         encode_color(inccol::inc_sRGB const &colInInt);


std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette256 &against);
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(
    std::vector<std::uint8_t> colorIDs_toValidate, const inccol::palette256 &against);

bool        validate_SQLite_tableExistence(sqlpp::sqlite3::connection &db, std::string const &tableName);
// Must provide all colName and all colTypes in the right order
// Otherwise the func will return false
// template <typename SQLPP23TableInfo_t>
inline bool validate_SQLite_tableColNamesTypes(sqlpp::sqlite3::connection &db, std::string const &tableName,
                                               std::vector<std::string_view> const &colNames,
                                               std::vector<std::string_view> const &colTypes);
bool        validate_configDB(sqlpp::sqlite3::connection &db);


std::expected<inccons::color_schemes::scheme256, dbErr> get_lastUsedScheme(sqlpp::sqlite3::connection &db);


} // namespace incom::terminal_plot::config