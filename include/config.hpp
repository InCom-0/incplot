#pragma once

#include <cstdint>

#include <incstd/incstd_color.hpp>
#include <incstd/incstd_console.hpp>
#include <sqlitedefs.hpp>



namespace incom::terminal_plot::config {

namespace inccol  = incstd::color;
namespace inccons = incstd::console;

inccol::palette256 get_lastUsedPalette();

std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(size_t colorCount_toValidate,
                                                                            const inccol::palette256 &against);
std::expected<bool, inccons::err_terminal> validate_terminalPaletteSameness(
    std::vector<std::uint8_t> colorIDs_toValidate, const inccol::palette256 &against);


} // namespace incom::terminal_plot::config