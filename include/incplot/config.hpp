#pragma once

#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <string_view>
#include <system_error>

#include <sqlpp23/sqlite3/sqlite3.h>
#include <sqlpp23/sqlpp23.h>

#include <cpr/cpr.h>
#include <incplot/args.hpp>
#include <incplot/err.hpp>
#include <incstd/incstd_color.hpp>


#include <archive.h>
#include <archive_entry.h>

#include <incplot/sqlitedefs.hpp>
#include <vector>


namespace incom::terminal_plot::config {

namespace inccol  = incstd::color;
namespace inccons = incstd::console;

using namespace incstd::console::color_schemes;

inline const color_schemes::scheme16 default_scheme16 = incstd::console::color_schemes::windows_terminal::campbell;
inline constexpr std::string_view    appName{"incplot"sv};
inline constexpr std::string_view    configFileName{"configDB.sqlite"sv};
inline constexpr std::string_view    configSeedFileName{"configDB.seed.sqlite"sv};
inline constexpr std::string_view    portableMarkerName{".incplot-portable"sv};

inline constexpr std::string_view    fromTerminalSchemeName{"__fromTerminalScheme"sv};


inline constexpr size_t html_defaultFontSize = 16uz;
inline constexpr size_t html_minFontSize     = 1uz;
inline constexpr size_t html_maxFontSize     = 256uz;

inline constexpr std::string_view html_fallbackFont_URLsource =
    "https://github.com/ryanoasis/nerd-fonts/releases/latest/download/Iosevka.tar.xz"sv;
inline constexpr std::string_view html_fallbackFont_filePathInURLsource = "IosevkaNerdFont-Regular.ttf"sv;

inline constexpr std::array<std::string_view, 13> schemes_defaultSchemesNames{
    "dimidium"sv,       "campbell"sv,        "campbell_ps"sv,    "cga"sv,        "dark_plus"sv,   "one_half_dark"sv,
    "one_half_light"sv, "solarized_light"sv, "solarized_dark"sv, "tango_dark"sv, "tango_light"sv, "ibm_5153"sv,
    "monochrome"sv};

inline constexpr float html_fontFamilyMatch_minScore = 0.8f;
inline constexpr float html_fontFaceMatch_minScore   = 0.8f;


std::vector<std::byte> download_fileRaw(std::string_view url, bool indicator = true);

std::expected<std::filesystem::path, std::error_code> get_portableMarkerDir();

template <typename FUNC>
std::expected<std::vector<std::vector<std::byte>>, incerr_c> extract_fromArchive(std::span<const std::byte> rawMemory,
                                                                                 FUNC const &filter_func) {
    std::vector<std::vector<std::byte>> res{};

    struct archive       *a;
    struct archive_entry *entry;
    int                   ok_r;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    // archive_read_open_memory(a, downloaded.data(), downloaded.size());
    ok_r = archive_read_open_memory(a, rawMemory.data(), rawMemory.size_bytes());
    if (ok_r != ARCHIVE_OK) {
        return std::unexpected(incerr_c::make(incom::terminal_plot::Unexp_AP::FONT_LIBARCHIVE_cannotOpenArchive));
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        // printf("%s\n", archive_entry_pathname(entry));

        if (filter_func(entry)) {
            auto const size64 = archive_entry_size(entry);
            if (size64 < 0) {
                return std::unexpected(
                    incerr_c::make(incom::terminal_plot::Unexp_AP::FONT_LIBARCHIVE_cannotReadArchiveEntrySize));
            }
            res.push_back(std::vector<std::byte>(static_cast<std::size_t>(size64)));
            std::size_t offset = 0;

            while (offset < res.back().size()) {
                auto const n = archive_read_data(a, res.back().data() + offset, res.back().size() - offset);
                if (n == 0) { break; }
                if (n < 0) {
                    return std::unexpected(
                        incerr_c::make(incom::terminal_plot::Unexp_AP::FONT_LIBARCHIVE_unknownErrorWhenReading));
                }
                offset += static_cast<std::size_t>(n);
            }
            res.back().resize(offset);
        }
        archive_read_data_skip(a); // Note 2
    }
    ok_r = archive_read_free(a);   // Note 3
    if (ok_r != ARCHIVE_OK) { std::exit(1); }

    return res;
}

std::expected<bool, incerr_c> validate_terminalPaletteSameness(std::uint8_t             colorCount_toValidate,
                                                               const inccol::palette16 &against);
std::expected<bool, incerr_c> validate_terminalPaletteSameness(std::uint8_t              colorCount_toValidate,
                                                               const inccol::palette256 &against);
std::expected<bool, incerr_c> validate_terminalPaletteSameness(std::vector<std::uint8_t> colorIDs_toValidate,
                                                               const inccol::palette256 &against);


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

std::expected<std::optional<std::string>, dbErr> check_schemeExistsInDB(sqlpp::sqlite3::connection &dbConn,
                                                                        scheme256 const            &scheme);
std::expected<std::optional<std::string>, dbErr> check_schemeExistsInDB(sqlpp::sqlite3::connection &dbConn,
                                                                        scheme16 const             &scheme);

std::expected<std::vector<std::byte>, dbErr> get_default_font(sqlpp::sqlite3::connection &dbConn);
std::expected<size_t, dbErr> set_default_font(sqlpp::sqlite3::connection &dbConn, std::span<std::byte> ttf_font_raw);
std::expected<size_t, dbErr> remove_default_font(sqlpp::sqlite3::connection &dbConn);


} // namespace db

} // namespace incom::terminal_plot::config