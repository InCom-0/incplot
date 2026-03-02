#pragma once

#include <cpr/error.h>
#include <incerr.hpp>
#include <incplot-lib/config.hpp>


namespace incom {
namespace terminal_plot {
using namespace std::literals;
using incerr_c = incerr::incerr_code;

enum class Unexp_AP {
    OK                          = 0,
    FONT_incfontdiscSystemError = 1,
    FONT_noReadAccessToFontFile,
    FONT_systemFontDiscoveredButImpossibleToLoad,
    FONT_CPR_error,

    FONT_unknownErrorOnFileRead,
    FONT_OTS_emptyInput,
    FONT_OTS_verificationProcessFailed,
    FONT_OTS_negativeOutputOffset,
    FONT_OTS_emptyOutput,

    SETUP_schemeGrab_notSupportedOnWindows,
    SETUP_schemeGrab_withoutName,
    SETUP_schemeGrab_nameTooLong,
    SETUP_schemeGrab_nameSameAsBuildinScheme,
    SETUP_schemeGrab_errorWhenQueryingTerminal,

    
    SETUP_fbFont_,
    SETUP_fbFont_1,
    SETUP_fbFont_noReadAccessToFontFile,
    SETUP_fbFont_unknownErrorOnFileRead,
    SETUP_fbFont_unknownError,
    DPCTOR_UnknownError,
};
namespace config {
enum class dbErr : size_t {
    OK                        = 0,
    impossibleNumberOfRecords = 1,
    impossibleValue,
    notFound,
    connectionError,
    dbAppearsCorrupted,
    missingData,
    unknownError,
};

} // namespace config

} // namespace terminal_plot
} // namespace incom

INCERR_REGISTER(incom::terminal_plot::Unexp_AP, incom::terminal_plot);
INCERR_REGISTER(incom::terminal_plot::config::dbErr, incom::terminal_plot::config);
INCERR_REGISTER(cpr::ErrorCode, cpr);

#undef INCERR_REGISTER
