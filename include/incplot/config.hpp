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
namespace fs      = std::filesystem;

using namespace incstd::console::color_schemes;


inline const scheme16             default_scheme16 = windows_terminal::campbell;
inline constexpr std::string_view appName{"incplot"sv};
inline constexpr std::string_view configDBFileName{"configDB.sqlite"sv};
inline constexpr std::string_view configSeedDBFileName{"configDB.seed.sqlite"sv};
inline constexpr std::string_view devBuildMarkerFilename{".incplot-dev-build"sv};
inline constexpr std::string_view fromTerminalSchemeName{"__fromTerminalScheme"sv};

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

bool                                     check_devBuildMarkerExists();
std::expected<fs::path, std::error_code> create_cPath_fromSamePrefix(fs::path srcPath, std::string_view srcPath_postfix,
                                                                     std::string_view resPath_postFix);

// Obtains a path to some resource installed with the program
// Args:
// 1) Install dir (corresponds to eg. CMAKE_INSTALL_DATADIR) ... this is relative to 'prefix' (eg. CMAKE_INSTALL_PREFIX)
// 2) Absolute install dir (corresponds to eg. CMAKE_INSTALL_FULL_DATADIR)
// 3) Relative path from the install dir
// 4) Possibility to insert 'project name' into the search (before 3) )
// 5) Relative path from executable dir  to 'dev marker' that gets checked
//      a. If it exists, we are in 'dev mode' (project not installed, all resources located in build staging area)
//      b. If it does not exist, it is assumed we are in 'installed mode'
std::expected<fs::path, incerr_c> get_pth2InstalledRes(
    std::string_view relInstallDirPth_toPrefix, std::string_view absInstallDirPth_whenInstalled,
    std::string_view relPth_toTarget, std::optional<std::string_view> toInsert_projNameFolder = std::nullopt,
    std::optional<std::string_view> relPth_toDevMarker = std::nullopt);

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

namespace scheme {
scheme256 get_defaultColScheme256();
scheme16  get_defaultColScheme16();
scheme256 get_monochromeColScheme256();
scheme16  get_monochromeColScheme16();

std::optional<scheme16> get_schemeFromTerminal();

std::string get_showInternalSchemes();
std::string get_showCongfigDBSchemes(sqlpp::sqlite3::connection &db);

std::string get_showSchemes(std::expected<sqlpp::sqlite3::connection, incerr_c> &db);
} // namespace scheme

namespace db {

std::expected<sqlpp::sqlite3::connection, incerr_c> create_dbConnection_rw(fs::path const &pathToDb);
std::expected<sqlpp::sqlite3::connection, incerr_c> create_dbConnection_ro(fs::path const &pathToDb);

std::expected<fs::path, incerr_c> getPath_configDB();
std::expected<fs::path, incerr_c> getPath_configSeedDB();
std::expected<bool, incerr_c>     check_is_configDBCurrent(fs::path const &configDB, fs::path const &seedConfigDB);

bool                              validate_configDB(sqlpp::sqlite3::connection &db);
std::expected<fs::path, incerr_c> validate_configDBversion(fs::path const &configDBpth);

bool        validate_SQLite_tableExistence(sqlpp::sqlite3::connection &db, std::string const &tableName);
// Must provide all colName and all colTypes in the right order
// Otherwise the func will return false
// template <typename SQLPP23TableInfo_t>
inline bool validate_SQLite_tableColNamesTypes(sqlpp::sqlite3::connection &db, std::string const &tableName,
                                               std::vector<std::string_view> const &colNames,
                                               std::vector<std::string_view> const &colTypes);


constexpr inccol::inc_sRGB decode_color(uint32_t const colInInt);
constexpr uint32_t         encode_color(inccol::inc_sRGB const &srgb);

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