#include <print>
#include <unistd.h>

#include <cross_platform.hpp>
#include <incplot.hpp>


int main(int argc, char *argv[]) {

    // NOT RUNNING IN CONSOLE TERMINAL
    if (not is_inTerminal()) {
        std::print("{}\n{}\n{}\n", "Console screen buffer size equals 0.",
                   "Please run from inside terminal console window", " ... exiting");
        std::exit(1);
    }

    argparse::ArgumentParser ap("incplot", "1.0", argparse::default_arguments::help);
    incplot::CL_Args::finishAp(ap);
    auto const &dpctrs = incplot::CL_Args::get_dpCtorStruct(ap, argc, argv);

    // STD INPUT IS NOT PIPE
    if (isatty(fileno(stdin))) {
        std::print("{}\n{}\n{}\n\n{}\n", "You need to 'pipe in' data on the standard input.",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }


    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    for (auto const &dpctr : dpctrs) { std::print("{}\n", incplot::make_plot_collapseUnExp(dpctr, input)); }

    return 0;
}