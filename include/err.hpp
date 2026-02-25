#pragma once

#include <incerr.hpp>
#include <incplot/config.hpp>


namespace incom {
namespace terminal_plot {
using namespace std::literals;
using incerr_c = incerr::incerr_code;

enum class Unexp_AP {
    // No value '0'
    FONT_incfontdiscSystemError = 1,
    FONT_noReadAccessToFontFile,
    FONT_systemFontDiscoveredButImpossibleToLoad,
    
    FONT_unknownErrorOnFileRead,
    FONT_OTS_emptyInput,
    FONT_OTS_verificationProcessFailed,
    FONT_OTS_negativeOutputOffset,
    FONT_OTS_emptyOutput,
    DPCTOR_UnknownError,
};


} // namespace terminal_plot
} // namespace incom

INCERR_REGISTER(incom::terminal_plot::Unexp_AP, incom::terminal_plot);

#undef INCERR_REGISTER
