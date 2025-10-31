#include <print>

#include <auto/versiondef.h>

#include <args.hpp>
#include <config.hpp>
#include <incplot.hpp>
#include <incstd/core/filesys.hpp>
#include <incstd/incstd_console.hpp>

using namespace std::literals;


int main(int argc, char *argv[]) {

    // Create and populate ArgumentParser
    argparse::ArgumentParser ap(std::string(incplot::config::appName), INCPLOT_VERSION_MEDIUM,
                                argparse::default_arguments::all);
    incplot::cl_args::finishAp(ap);
    incplot::cl_args::populateAp(ap, argc, argv);

    // Set the right character page of the terminal
    incom::standard::console::set_cocp();

    // STDIN IS IN TERMINAL (that is there is no input 'piped in')
    if (incom::standard::console::is_stdin_inTerminal()) {
        std::print("{}\n{}\n{}\n\n{}\n", "The user needs to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }
    // Get the input data and store it in std::string
    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    // If the user wants just to display the available schemes we do that and exit
    if (ap.get<bool>("-s")) {
        auto dbCon =
            incplot::config::db::get_configConnection(incplot::config::appName, incplot::config::configFileName);
        std::cout << incplot::config::get_showSchemes(dbCon);
        return 0;
    }

    // If not the above then we create the dpCtors (ie. create the instructions from what was parsed by ArgumentParser)
    auto dpctrs = incplot::cl_args::get_dpCtorStruct(ap);


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
        for (auto &dpctr : dpctrs) {
            dpctr.availableWidth  = colsInTerm;
            dpctr.availableHeight = rowsInTerm;
        }
    }

    // Do the thing ... ie. make all the plots and print them
    for (auto const &dpctr : dpctrs) { std::cout << incplot::make_plot_collapseUnExp(dpctr, input) << '\n'; }
    return 0;
}