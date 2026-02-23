#include <cstring>
#include <print>
#include <string_view>

#include <auto/versiondef.h>

#include <args.hpp>
#include <config.hpp>
#include <incfontdisc/incfontdisc.hpp>
#include <incplot.hpp>
#include <incstd/core/filesys.hpp>
#include <incstd/incstd_console.hpp>


#include <uri.h>


using namespace std::literals;


int main(int argc, char *argv[]) {

    // auto allFnts = incfontdisc::list_fonts();

    auto parsedURI = incplot::URI("./termix/tmp.txt", true);


    // Create and populate ArgumentParser
    argparse::ArgumentParser ap(std::string(incplot::config::appName), INCPLOT_VERSION_MEDIUM,
                                argparse::default_arguments::all);
    argparse::ArgumentParser subap_setup("setup", INCPLOT_VERSION_MEDIUM, argparse::default_arguments::help);
    incplot::cl_args::finishAp(ap, subap_setup);
    incplot::cl_args::populateAp(ap, argc, argv);

    // Set the right character page of the terminal
    incom::standard::console::set_cocp();

    if (ap.is_subcommand_used(subap_setup)) {
        std::cout << "Used \n";

        if (auto setupCommand_res = incplot::cl_args::process_setupCommand(subap_setup)) {
            std::string toPrint;
            for (auto const &oneLine : setupCommand_res.value()) {
                toPrint.append(oneLine);
                toPrint.push_back('\n');
            }
            std::cout << toPrint;
            return 0;
        }

        else { return -1; }
    }

    // Get connection to configDB
    if (ap.get<bool>("-s")) {
        auto dbCon =
            incplot::config::db::get_configConnection(incplot::config::appName, incplot::config::configFileName);
        std::cout << incplot::config::get_showSchemes(dbCon);
        return 0;
    }

    // We create the dpCtors (ie. create the instructions from what was parsed by ArgumentParser)
    auto dpctrs = incplot::cl_args::get_dpCtorStruct(ap);
    if (not dpctrs.has_value()) {
        std::print("{}\n\n{}{}\n{}\n{}\n", "Error occurred during parsing command line arguments.",
                   "The error code is: ", "EC", "COMMENT", "... exiting");
        std::exit(1);
    }

    // STDIN IS IN TERMINAL (that is there is no input 'piped in')
    if (incom::standard::console::is_stdin_inTerminal()) {
        std::print("{}\n{}\n{}\n\n{}\n", "The user needs to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }
    // Get the input data and store it in std::string
    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());


    // STDOUT IS NOT IN TERMINAL
    if (not incom::standard::console::is_stdout_inTerminal()) {
        // Then the user needs to specify width
        if (not ap.present<int>("-w").has_value()) {
            std::print(
                "{}\n{}\n{}\n", "Incplot's output was not connected to console screen buffer.",
                "Incplot requires '-w' command line argument for this usecase, but it was not given by the user.",
                " ... exiting");
            std::exit(1);
        }
    }
    else {
        auto const [rowsInTerm, colsInTerm] = incom::standard::console::get_rowColCount();
        for (auto &dpctr : dpctrs.value()) {
            dpctr.availableWidth  = colsInTerm;
            dpctr.availableHeight = rowsInTerm;
        }
    }

    // Do the thing ... ie. make all the plots and print them
    // TODO: need to somehow handle multiple plots emitted in HTML mode
    for (auto const &dpctr : dpctrs.value()) { std::cout << incplot::make_plot_collapseUnExp(dpctr, input) << '\n'; }
    return 0;
}