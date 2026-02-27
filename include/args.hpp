#pragma once

#include <expected>

#include <argparse/argparse.hpp>
#include <cpr/cpr.h>
#include <opentype-sanitiser.h>
#include <ots-memory-stream.h>

#include <incplot/desired_plot.hpp>
#include <incplot/err.hpp>

#include <uri.hpp>
#include <err.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {
namespace incplot = incom::terminal_plot;

std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStruct(argparse::ArgumentParser const &ap);
std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStruct();

std::expected<std::vector<std::string>, incerr_c> process_setupCommand(argparse::ArgumentParser const &setup_ap);

void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup);
void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv);

}; // namespace cl_args
} // namespace terminal_plot
} // namespace incom