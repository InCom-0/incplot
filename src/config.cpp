#include <config.hpp>
#include <expected>

namespace incom::terminal_plot::config {
using namespace incstd::color;
using namespace incstd::console;

inccol::palette256 get_lastUsedPalette() {
    return inccol::palette256{};
}

std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(std::uint8_t colorCount_toValidate,
                                                                            const inccol::palette256 &against) {
    for (size_t i = 0; i < colorCount_toValidate; ++i) {
        auto queryRes = ColorQuery::queryPaletteIndex(i);
        if (not queryRes.has_value()) { return std::unexpected(queryRes.error()); }
        else if (queryRes.value() != against[i]) { return false; }
    }

    // If all pass then its validated
    return true;
}
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(
    std::vector<std::uint8_t> colorIDs_toValidate, const inccol::palette256 &against) {
    for (auto colToVal : colorIDs_toValidate) {
        auto queryRes = ColorQuery::queryPaletteIndex(colToVal);
        if (not queryRes.has_value()) { return std::unexpected(queryRes.error()); }
        else if (queryRes.value() != against[colToVal]) { return false; }
    }

    // If all pass then its validated
    return true;
}


} // namespace incom::terminal_plot::config