#include <print>
#include <string_view>
#include <unistd.h>

#include <incplot.hpp>

int main(int argc, char *argv[]) {

    // NOT RUNNING IN CONSOLE TERMINAL
    if (not incplot::detail::is_inTerminal()) {
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
    auto              ds = incplot::parsers::Parser::parse(std::string_view(input));


    for (auto const &dpctr : incplot::CL_Args::get_dpCtorStruct(ap, argc, argv)) {
        auto dp_autoGuessed = incplot::DesiredPlot(dpctr).guess_missingParams(ds.value());


        if (not dp_autoGuessed.has_value()) {
            std::print("{0}{1}{2}", "Autoguessing of 'DesiresPlot' parameters for: ",
                       dpctr.plot_type_name.has_value() ? dpctr.plot_type_name.value() : "[Unspecified plot type]",
                       " failed.\n");
            continue;
        }

        auto plotDrawer = incplot::make_plotDrawer(dp_autoGuessed.value(), ds.value());
        if (not plotDrawer.has_value()) {
            std::print("{0}{1}{2}", "Creating 'Plot Structure' for: ",
                       dpctr.plot_type_name.has_value() ? dpctr.plot_type_name.value() : "[Unspecified plot type]",
                       " failed.\n");
            continue;
        }


        auto outExp = plotDrawer.value().validateAndDrawPlot();

        if (not outExp.has_value()) {
            std::print("{0}{1}{2}", "Invalid plot structure for: ",
                       dpctr.plot_type_name.has_value() ? dpctr.plot_type_name.value() : "[Unspecified plot type]",
                       ".");
            continue;
        }
        std::print("{}\n", outExp.value());
    }

    return 0;
}