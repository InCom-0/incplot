#include <print>

#include <auto/versiondef.h>
#include <incplot.hpp>
#include <incstd/core/filesys.hpp>
#include <incstd/incstd_console.hpp>
#include <config.hpp>


using namespace std::literals;
inline constexpr std::string appName("incplot"sv);
inline constexpr std::string configFileName("configDB.sqlite"sv);


int main(int argc, char *argv[]) {

    argparse::ArgumentParser ap(std::string(appName), INCPLOT_VERSION_MEDIUM, argparse::default_arguments::all);
    incplot::CL_Args::finishAp(ap);
    auto dpctrs = incplot::CL_Args::get_dpCtorStruct(ap, argc, argv);

    // STDIN IS IN TERMINAL (that is there is no input 'piped in')
    if (incom::standard::console::is_stdin_inTerminal()) {
        std::print("{}\n{}\n{}\n\n{}\n", "The user needs to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }

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


    auto configPath_exp = incstd::filesys::find_configFile(appName, configFileName);

    if (not configPath_exp.has_value()) {
        // Must use default 'in code' config since the sqlite config file is unavailable or somehow corrupted
    }
    else {
        auto configOnDisk              = std::make_shared<sqlpp::sqlite3::connection_config>();
        configOnDisk->path_to_database = configPath_exp->generic_string();
        configOnDisk->flags            = SQLITE_OPEN_READWRITE;

        sqlpp::sqlite3::connection dbOnDisk;
        dbOnDisk.connect_using(configOnDisk); // This can throw an exception.

        if (not incom::terminal_plot::config::validate_configDB(dbOnDisk)) {
            // Must use default 'in code' config since the sqlite configDB is unavailable or somehow corrupted
        }

        else {
            if (auto defScheme = incplot::config::get_lastUsedScheme(dbOnDisk)) {
                if (incplot::config::validate_terminalPaletteSameness(3, defScheme.value().palette)) {
                
                }
                else {}
            }
            else {
            }
        }
    }


    incom::standard::console::set_cocp();
    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    for (auto const &dpctr : dpctrs) { std::cout << incplot::make_plot_collapseUnExp(dpctr, input) << '\n'; }

    return 0;
}