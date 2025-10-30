#pragma once

#include <argparse/argparse.hpp>
#include <incplot/desired_plot.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {

std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct(argparse::ArgumentParser const &ap);
std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct();

void finishAp(argparse::ArgumentParser &out_ap);
void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv);

}; // namespace cl_args
} // namespace terminal_plot
} // namespace incom