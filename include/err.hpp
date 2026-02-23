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
};

// INCPLOT_LIB_API std::string_view incerr_msg_dispatch(Unexp_plotSpecs &&e);
// std::string_view incerr_msg_dispatch(Unexp_plotDrawer &&e);


} // namespace terminal_plot
} // namespace incom

INCERR_REGISTER(incom::terminal_plot::Unexp_AP, incom::terminal_plot);

#undef INCERR_REGISTER
