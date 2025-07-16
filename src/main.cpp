#include <print>

#include <auto/versiondef.h>
#include <cross_platform.hpp>
#include <incplot.hpp>


int main(int argc, char *argv[]) {

    argparse::ArgumentParser ap("incplot", INCPLOT_VERSION_MEDIUM, argparse::default_arguments::all);
    incplot::CL_Args::finishAp(ap);
    auto dpctrs = incplot::CL_Args::get_dpCtorStruct(ap, argc, argv);

    // NOT RUNNING IN CONSOLE TERMINAL
    if (not incom::crossplatform::console::is_inTerminal()) {
        std::print("{}\n{}\n{}\n", "Console screen buffer size equals 0.",
                   "Please run from inside terminal console window", " ... exiting");
        std::exit(1);
    }

    incom::crossplatform::console::set_cocp();

    // STD INPUT IS NOT PIPE
    if (incom::crossplatform::console::is_stdin_inTerminal()) {
        std::print("{}\n{}\n{}\n\n{}\n", "You need to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }

    auto [rowsInTerm, colsInTerm] = incom::crossplatform::console::get_rowColCount();
    for (auto &dpctr : dpctrs) {
        dpctr.availableWidth  = colsInTerm;
        dpctr.availableHeight = rowsInTerm;
    }

    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    for (auto const &dpctr : dpctrs) {
        std::cout << incplot::make_plot_collapseUnExp(dpctr, input) << '\n';
    }

    return 0;
}