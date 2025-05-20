#include <print>
#include <unistd.h>

#include <incplot.hpp>
#include <cross_platform.hpp>

int main(int argc, char *argv[]) {

    // NOT RUNNING IN CONSOLE TERMINAL
    if (not is_inTerminal()) {
        std::print("{}",
                   "Console screen buffer size equals 0.\nPlease run from inside terminal console window ... exiting");
        std::exit(1);
    }

    // STD INPUT IS NOT PIPE
    if (isatty(fileno(stdin))) {
        std::print("{}", "STD INPUT is not 'pipe' ... exiting");
        std::exit(1);
    }

    argparse::ArgumentParser ap("incplot", "1.0", argparse::default_arguments::help);
    incplot::CL_Args::finishAp(ap);


    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    for (auto const &dpctr : incplot::CL_Args::get_dpCtorStruct(ap, argc, argv)) {
        std::print("{}\n", incplot::make_plot_collapseUnExp(dpctr, input));
    }

    return 0;
}