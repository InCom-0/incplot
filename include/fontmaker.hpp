#pragma once

#include <expected>
#include <filesystem>
#include <fstream>
#include <istream>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <vector>


#include <incstd/incstd_console.hpp>


#include <sfntly/font.h>
#include <sfntly/font_factory.h>


namespace incom::terminal_plot {
using namespace sfntly;

class FontMaker {
private:
    sfntly::Font              m_pf;  // Primary Font
    std::vector<sfntly::Font> m_sfs; // Supplementary Fonts

public:
    enum class FMErr : size_t {
        noFile = 1,
        fontLoadingUnsuccessful,
        fontArrayIsEmpty,
    };


    FontMaker(sfntly::Font const &primaryFont, std::vector<sfntly::Font> const &supplementaryFonts)
        : m_pf(primaryFont), m_sfs(supplementaryFonts) {}
    FontMaker(sfntly::Font &&primaryFont, std::vector<sfntly::Font> &&supplementaryFonts)
        : m_pf(std::move(primaryFont)), m_sfs(std::move(supplementaryFonts)) {}


    static std::expected<FontMaker, FMErr> make(std::filesystem::path const              &pthTo_primaryFont,
                                                std::vector<std::filesystem::path> const &pthTo_supplementaryFonts,
                                                bool                                      sfsMustBeValid = true) {

        auto pf = fontFromPath(pthTo_primaryFont);
        if (not pf.has_value()) { return std::unexpected(pf.error()); }

        std::vector<sfntly::Font> sfs;

        for (auto const &onePth : pthTo_supplementaryFonts) {
            auto font = fontFromPath(onePth);
            if (not font.has_value()) {
                if (sfsMustBeValid) { return std::unexpected(font.error()); }
            }
            else { sfs.push_back(std::move(font.value())); }
        }

        return FontMaker(std::move(pf.value()), std::move(sfs));
    }


    static std::unordered_set<char32_t> extract_codepoints(const std::string_view &utf8) {
        std::unordered_set<char32_t> result;
        size_t                       i = 0;
        while (i < utf8.size()) {
            unsigned char c         = utf8[i];
            char32_t      codepoint = 0;
            size_t        extra     = 0;
            if (c < 0x80) { codepoint = c; }
            else if ((c >> 5) == 0x6) { // 2-byte
                codepoint = c & 0x1F;
                extra     = 1;
            }
            else if ((c >> 4) == 0xE) { // 3-byte
                codepoint = c & 0x0F;
                extra     = 2;
            }
            else if ((c >> 3) == 0x1E) { // 4-byte
                codepoint = c & 0x07;
                extra     = 3;
            }
            else { throw std::runtime_error("Invalid UTF-8"); }

            if (i + extra >= utf8.size()) { throw std::runtime_error("Truncated UTF-8"); }
            for (size_t j = 1; j <= extra; ++j) {
                unsigned char cc = utf8[i + j];
                if ((cc >> 6) != 0x2) { throw std::runtime_error("Invalid UTF-8 continuation"); }
                codepoint = (codepoint << 6) | (cc & 0x3F);
            }

            result.insert(codepoint);
            i += extra + 1;
        }
        return result;
    }


private:
    static std::expected<sfntly::Font, FMErr> fontFromPath(std::filesystem::path const &pthToFont) {

        // --- Read entire file into memory
        std::ifstream file(pthToFont, std::ios::binary | std::ios::ate);
        if (! file) { return std::unexpected(FMErr::noFile); }

        const auto size = static_cast<size_t>(file.tellg());
        file.seekg(0);

        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char *>(buffer.data()), size);

        // --- Let FontFactory parse it
        // std::unique_ptr<class Ty>FontArray *fonts = new FontArray;
        auto fonts = std::make_unique<FontArray>();

        auto factory = std::unique_ptr<FontFactory>(FontFactory::GetInstance());
        factory->LoadFonts(&buffer, fonts.get());

        if (! fonts) { return std::unexpected(FMErr::fontLoadingUnsuccessful); }
        else if (fonts->empty()) { return std::unexpected(FMErr::fontArrayIsEmpty); }
        return *fonts->at(0);
    }
};
} // namespace incom::terminal_plot