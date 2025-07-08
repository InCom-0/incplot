#include <print>
#include <unistd.h>

#include <cross_platform.hpp>
#include <incplot.hpp>
#include <auto/versiondef.h>


int main(int argc, char *argv[]) {

    argparse::ArgumentParser ap("incplot", INCPLOT_VERSION_MEDIUM, argparse::default_arguments::all);
    incplot::CL_Args::finishAp(ap);
    auto dpctrs = incplot::CL_Args::get_dpCtorStruct(ap, argc, argv);

    // NOT RUNNING IN CONSOLE TERMINAL
    if (not is_inTerminal()) {
        std::print("{}\n{}\n{}\n", "Console screen buffer size equals 0.",
                   "Please run from inside terminal console window", " ... exiting");
        std::exit(1);
    }

    // STD INPUT IS NOT PIPE
    if (isatty(fileno(stdin))) {
        std::print("{}\n{}\n{}\n\n{}\n", "You need to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }

    auto [rowsInTerm, colsInTerm] = get_rowColCount();
    for (auto &dpctr : dpctrs) {
        dpctr.availableWidth  = colsInTerm;
        dpctr.availableHeight = rowsInTerm;
    }

    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    for (auto const &dpctr : dpctrs) { std::print("{}\n", incplot::make_plot_collapseUnExp(dpctr, input)); }

    return 0;
}