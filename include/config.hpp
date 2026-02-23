#pragma once

#include <cstddef>
#include <expected>
#include <string_view>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>

#include <args.hpp>
#include <cpr/cpr.h>
#include <incstd/incstd_color.hpp>
#include <incstd/incstd_console.hpp>

#include <archive.h>
#include <archive_entry.h>

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
inline constexpr size_t html_minFontSize     = 1uz;
inline constexpr size_t html_maxFontSize     = 256uz;

inline constexpr std::string_view html_fallbackFont_URLsource =
    "https://github.com/ryanoasis/nerd-fonts/releases/latest/download/Iosevka.tar.xz"sv;
inline constexpr std::string_view html_fallbackFont_filePathInURLsource = "IosevkaNerdFont-Regular.ttf"sv;

inline constexpr float html_fontFamilyMatch_minScore = 0.8f;
inline constexpr float html_fontFaceMatch_minScore  = 0.8f;

enum class dbErr : size_t {
    impossibleNumberOfRecords = 1,
    impossibleValue,
    notFound,
    connectionError,
    dbAppearsCorrupted,
    missingData,
    unknownError,
};


std::vector<std::byte> download_fileRaw(std::string_view url, bool indicator = true);

template <typename FUNC>
std::expected<std::vector<std::vector<std::byte>>, int> extract_fromArchive(std::span<const std::byte> rawMemory,
                                                                            FUNC const                &filter_func) {
    std::expected<std::vector<std::vector<std::byte>>, int> res = std::vector<std::vector<std::byte>>{};

    struct archive       *a;
    struct archive_entry *entry;
    int                   ok_r;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    // archive_read_open_memory(a, downloaded.data(), downloaded.size());
    ok_r = archive_read_open_memory(a, rawMemory.data(), rawMemory.size_bytes());
    if (ok_r != ARCHIVE_OK) {
        res = std::unexpected(-1);
        goto RET;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        // printf("%s\n", archive_entry_pathname(entry));

        if (filter_func(entry)) {
            auto const size64 = archive_entry_size(entry);
            if (size64 < 0) {
                res = std::unexpected(-1);
                goto RET;
            }
            res.value().push_back(std::vector<std::byte>(static_cast<std::size_t>(size64)));
            std::size_t offset = 0;

            while (offset < res.value().back().size()) {
                auto const n =
                    archive_read_data(a, res.value().back().data() + offset, res.value().back().size() - offset);
                if (n == 0) { break; }
                if (n < 0) {
                    res = std::unexpected(-1);
                    goto RET;
                }
                offset += static_cast<std::size_t>(n);
            }
            res.value().back().resize(offset);
        }
        archive_read_data_skip(a); // Note 2
    }
    ok_r = archive_read_free(a);   // Note 3
    if (ok_r != ARCHIVE_OK) { std::exit(1); }

RET:
    return res;
}

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


std::expected<std::vector<std::byte>, dbErr> get_default_font(sqlpp::sqlite3::connection &dbConn);
std::expected<size_t, dbErr> set_default_font(sqlpp::sqlite3::connection &dbConn, std::span<std::byte> ttf_font_raw);


} // namespace db

} // namespace incom::terminal_plot::config