#pragma once

#include <expected>

#include <argparse/argparse.hpp>
#include <incplot/desired_plot.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {

std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct(argparse::ArgumentParser const &ap);
std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct();

std::expected<std::vector<std::string>, int> process_setupCommand(argparse::ArgumentParser const &setup_ap);

void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup);
void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv);

}; // namespace cl_args
} // namespace terminal_plot
} // namespace incom