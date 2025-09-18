#include <print>

#include <auto/versiondef.h>
#include <cross_platform.hpp>
#include <incplot.hpp>


int main(int argc, char *argv[]) {

    argparse::ArgumentParser ap("incplot", INCPLOT_VERSION_MEDIUM, argparse::default_arguments::all);
    incplot::CL_Args::finishAp(ap);
    auto dpctrs = incplot::CL_Args::get_dpCtorStruct(ap, argc, argv);

    // STDIN IS IN TERMINAL (that is there is no input 'piped in')
    if (incom::crossplatform::console::is_stdin_inTerminal()) {
        std::print("{}\n{}\n{}\n\n{}\n", "The user needs to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }

    // STDOUT IS NOT IN TERMINAL
    if (not incom::crossplatform::console::is_stdout_inTerminal()) {
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
        auto const [rowsInTerm, colsInTerm] = incom::crossplatform::console::get_rowColCount();
        for (auto &dpctr : dpctrs) {
            dpctr.availableWidth  = colsInTerm;
            dpctr.availableHeight = rowsInTerm;
        }
    }

    incom::crossplatform::console::set_cocp();

    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    for (auto const &dpctr : dpctrs) { std::cout << incplot::make_plot_collapseUnExp(dpctr, input) << '\n'; }

    return 0;
}