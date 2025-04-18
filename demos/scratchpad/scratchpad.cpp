#include <io.h>
#include <print>
#include <windows.h>

#include <incplot.hpp>
#include <incplot/args.hpp>


int main(int argc, char *argv[]) {
    using json = nlohmann::json;


    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    auto width  = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    auto height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);

    // NOT RUNNING IN CONSOLE TERMINAL
    if (csbi.dwSize.X == 0 || csbi.dwSize.Y == 0) {
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


    std::string input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());
    auto        ds = incplot::Parser::parse_NDJSON_intoDS(input);


    for (auto const &dpctr : incplot::CL_Args::get_dpCtorStruct(ap, argc, argv)) {
        auto dp_autoGuessed = incplot::DesiredPlot(dpctr).guess_missingParams(ds);

        if (not dp_autoGuessed.has_value()) {
            std::print("{0}{1}{2}", "Autoguessing of 'DesiresPlot' parameters for: ",
                       dpctr.plot_type_name.has_value() ? dpctr.plot_type_name.value() : "[Unspecified plot type]",
                       " failed.\n");
            continue;
        }

        auto plotDrawer = incplot::make_plotDrawer(dp_autoGuessed.value(), ds);
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