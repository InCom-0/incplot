#pragma once

#include <argparse/argparse.hpp>
#include <incplot/desired_plot.hpp>


namespace incom {
namespace terminal_plot {
struct CL_Args {

    static std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct(argparse::ArgumentParser &inout_ap, int argc,
                                                                    const char *const *argv);
    static std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct();

    static void finishAp(argparse::ArgumentParser &out_ap);
};
} // namespace terminal_plot
} // namespace incom