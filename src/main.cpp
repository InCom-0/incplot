#include <cstring>
#include <print>

#include <auto/versiondef.h>

#include <args.hpp>
#include <config.hpp>
#include <incplot.hpp>
#include <incstd/core/filesys.hpp>
#include <incstd/incstd_console.hpp>

#include <incfontdisc/incfontdisc.hpp>
#include <string_view>


using namespace std::literals;


int main(int argc, char *argv[]) {

    // Create and populate ArgumentParser
    argparse::ArgumentParser ap(std::string(incplot::config::appName), INCPLOT_VERSION_MEDIUM,
                                argparse::default_arguments::all);
    argparse::ArgumentParser subap_setup("setup", INCPLOT_VERSION_MEDIUM, argparse::default_arguments::help);
    incplot::cl_args::finishAp(ap, subap_setup);
    incplot::cl_args::populateAp(ap, argc, argv);

    // auto fnt       = incfontdisc::match_fonts(incfontdisc::FontQuery{.family = "JetBrainsMono Nerd Font"});
    // auto fnt_bytes = incfontdisc::load_font_data(fnt->font.id);

    // Set the right character page of the terminal
    incom::standard::console::set_cocp();

    // Get connection to configDB
    auto dbCon = incplot::config::db::get_configConnection(incplot::config::appName, incplot::config::configFileName);

    // auto ios_sv = incom::standard::filesys::get_file_bytes("Iosevka.tar.xz"sv);

    // auto ria = incom::terminal_plot::config::extract_fromArchive(
    //     std::span<const std::byte>(reinterpret_cast<const std::byte *>(ios_sv.value().data()), ios_sv.value().size()),
    //     [](archive_entry *item) {
    //         if (archive_entry_pathname(item) == "IosevkaNerdFont-Regular.ttf"sv) { return true; }
    //         else { return false; }
    //     });

    // {
    //     struct archive       *a;
    //     struct archive_entry *entry;
    //     int                   r;

    //     a = archive_read_new();
    //     archive_read_support_filter_all(a);
    //     archive_read_support_format_all(a);
    //     // archive_read_open_memory(a, downloaded.data(), downloaded.size());
    //     r = archive_read_open_filename(a, "Iosevka.tar.xz", 10240);
    //     if (r != ARCHIVE_OK) { exit(1); }


    //     while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
    //         printf("%s\n", archive_entry_pathname(entry));
    //         if ("IosevkaNerdFont-Regular.ttf"sv == std::string_view(archive_entry_pathname(entry))) {
    //             auto const size64 = archive_entry_size(entry);
    //             if (size64 < 0) { throw std::runtime_error("Unknown entry size"); }

    //             std::vector<std::byte> out(static_cast<std::size_t>(size64));
    //             std::size_t            offset = 0;

    //             while (offset < out.size()) {
    //                 auto const n = archive_read_data(a, out.data() + offset, out.size() - offset);
    //                 if (n == 0) { break; }
    //                 if (n < 0) { throw std::runtime_error(archive_error_string(a)); }
    //                 offset += static_cast<std::size_t>(n);
    //             }

    //             out.resize(offset);

    //             break;                 // Break on success
    //         }
    //         archive_read_data_skip(a); // Note 2
    //     }
    //     r = archive_read_free(a);      // Note 3
    //     if (r != ARCHIVE_OK) { exit(1); }
    // }


    // auto r = incplot::config::db::set_default_font(dbCon.value(), fnt_bytes.value());

    // cpr::Response r =
    //     cpr::Get(cpr::Url{"https://github.com/ryanoasis/nerd-fonts/releases/latest/download/Iosevka.tar.xz"});

    // auto downloaded = incplot::config::download_fileRaw(incplot::config::html_fallbackFont_URLsource);

    // If the user wants just to display the available schemes we do that and exit
    if (ap.get<bool>("-s")) {
        std::cout << incplot::config::get_showSchemes(dbCon);
        return 0;
    }

    auto dpctrs = incplot::cl_args::get_dpCtorStruct(ap);


    // STDIN IS IN TERMINAL (that is there is no input 'piped in')
    if (incom::standard::console::is_stdin_inTerminal()) {
        std::print("{}\n{}\n{}\n\n{}\n", "The user needs to 'pipe in' data on standard input\n",
                   "This is usually done using the '|' operator", "Pass '-h' to obtain more detailed help",
                   "... exiting");
        std::exit(1);
    }
    // Get the input data and store it in std::string
    std::string const input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());


    // If not the above then we create the dpCtors (ie. create the instructions from what was parsed by ArgumentParser)


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