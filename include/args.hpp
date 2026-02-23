#pragma once

#include <expected>

#include <argparse/argparse.hpp>
#include <incplot/desired_plot.hpp>

#include <err.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {

std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incom::terminal_plot::Unexp_AP> get_dpCtorStruct(argparse::ArgumentParser const &ap);
std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incom::terminal_plot::Unexp_AP> get_dpCtorStruct();

std::expected<std::vector<std::string>, incom::terminal_plot::Unexp_AP> process_setupCommand(argparse::ArgumentParser const &setup_ap);

void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup);
void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv);

}; // namespace cl_args
} // namespace terminal_plot
} // namespace incom