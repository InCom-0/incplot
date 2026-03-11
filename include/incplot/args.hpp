#pragma once

#include <expected>

#include <argparse/argparse.hpp>
#include <cpr/cpr.h>
#include <opentype-sanitiser.h>
#include <ots-memory-stream.h>

#include <incplot-lib/desired_plot.hpp>
#include <incplot-lib/err.hpp>

#include <incplot/err.hpp>
#include <incstd/web/uri.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {
namespace incplot = incom::terminal_plot;

std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStructs(argparse::ArgumentParser const &ap);
std::expected<std::vector<DesiredPlot::DP_CtorStruct>, incerr_c> get_dpCtorStructs();

std::expected<std::vector<std::string>, incerr_c> process_setupCommand(argparse::ArgumentParser const &setup_ap);

void finishAp(argparse::ArgumentParser &out_ap, argparse::ArgumentParser &subap_setup);
void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv);

}; // namespace cl_args
} // namespace terminal_plot
} // namespace incom