#include <optional>
#include <typeindex>

#include <args.hpp>
#include <config.hpp>
#include <incplot/plot_structures.hpp>
#include <incstd/core/typegen.hpp>


namespace incom {
namespace terminal_plot {
std::vector<DesiredPlot::DP_CtorStruct> CL_Args::get_dpCtorStruct(argparse::ArgumentParser &inout_ap, int argc,
                                                                  const char *const *argv) {
    try {
        inout_ap.parse_args(argc, argv);
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << inout_ap;
        std::exit(1);
    }
    DesiredPlot::DP_CtorStruct nonDifferentiated;

    if (auto wdt = inout_ap.present<int>("-w")) { nonDifferentiated.tar_width = wdt.value(); }
    if (auto hgt = inout_ap.present<int>("-t")) { nonDifferentiated.tar_height = hgt.value(); }
    if (auto stddev = inout_ap.present<int>("-e")) {
        if (stddev != 0) { nonDifferentiated.filter_outsideStdDev = stddev.value(); }
        else { nonDifferentiated.filter_outsideStdDev = std::nullopt; }
    }
    else { nonDifferentiated.filter_outsideStdDev = Config::filter_withinStdDevMultiple_default; }


    if (auto optVal = inout_ap.present<int>("-x")) { nonDifferentiated.lts_colID = optVal.value(); }
    if (auto optVal = inout_ap.present<std::vector<int>>("-y")) {
        nonDifferentiated.v_colIDs = std::vector<size_t>(optVal.value().begin(), optVal.value().end());
    }
    if (auto optVal = inout_ap.present<int>("-c")) { nonDifferentiated.c_colID = optVal.value(); }

    if (inout_ap.get<bool>("-r")) { nonDifferentiated.forceRGB_bool = true; }
    if (inout_ap.get<bool>("-d")) {
        nonDifferentiated.colScheme     = color_schemes::defaultScheme16;
        nonDifferentiated.forceRGB_bool = true;
    }
    else if (inout_ap.get<bool>("-m")) {
        nonDifferentiated.colScheme     = color_schemes::other_sources::monochrome;
        nonDifferentiated.forceRGB_bool = true;
    }
    else {
        auto dbConn = config::maybeGet_configConnection(config::appName, config::configFileName);
        if (dbConn.has_value()) {
            if (inout_ap.is_used("-l")) { auto schm_name = inout_ap.get<std::string>("-l"); }
            else {
                // Pure default path
            }
        }
        else {
            auto schm_opt               = config::maybeGet_schemeFromTerminal();
            nonDifferentiated.colScheme = schm_opt.value_or(config::get_defaultColScheme16());
            if (schm_opt) {
                nonDifferentiated.forceRGB_bool = true;
                // Config DB conn not available, falling back to using ANSI color scheme currently set in the
                // terminal
            }
            else {
                // Config DB conn not available and couldn't obtain ANSI color scheme from the terminal ... using
                // default integrated color scheme
            }
        }
    }

    std::vector<DesiredPlot::DP_CtorStruct> res;
    auto                                    addOne = [&](std::optional<std::type_index> const &&sv_opt) {
        res.push_back(nonDifferentiated);
        res.back().plot_type_name = sv_opt;
    };
    using namespace incom::standard::typegen;
    if (inout_ap.get<bool>("-B")) { addOne(get_typeIndex<plot_structures::BarV>()); }
    if (inout_ap.get<bool>("-S")) { addOne(get_typeIndex<plot_structures::Scatter>()); }
    if (inout_ap.get<bool>("-L")) { addOne(get_typeIndex<plot_structures::Multiline>()); }
    if (inout_ap.get<bool>("-V")) { addOne(get_typeIndex<plot_structures::BarVM>()); }
    if (inout_ap.get<bool>("-H")) { addOne(get_typeIndex<plot_structures::BarHM>()); }
    if (inout_ap.get<bool>("-T")) { addOne(get_typeIndex<plot_structures::BarHS>()); }
    if (res.empty()) { addOne(std::nullopt); }

    return res;
}

std::vector<DesiredPlot::DP_CtorStruct> CL_Args::get_dpCtorStruct() {
    return std::vector<DesiredPlot::DP_CtorStruct>{DesiredPlot::DP_CtorStruct{.plot_type_name = std::nullopt}};
}

void CL_Args::finishAp(argparse::ArgumentParser &out_ap) {
    out_ap.add_description(
        "Draw coloured plots using unicode symbols inside terminal.\n\nAutomatically infers what to display and "
        "how based on the shape of the data piped in.\nPipe in data in JSON, JSON Lines, NDJSON, CSV or TSV formats. "
        "All "
        "arguments "
        "are optional");

    out_ap.add_group("Plot type options");
    out_ap.add_argument("-B", "--barV").help("Draw vertical [B]ar plot [flag]").flag().nargs(0);
    out_ap.add_argument("-S", "--scatter").help("Draw [S]catter plot [flag]").flag().nargs(0);
    out_ap.add_argument("-L", "--line").help("Draw (multi)[L]ine plot [flag]").flag().nargs(0);
    out_ap.add_argument("-V", "--barVM").help("Draw [V]ertical multibar plot [flag]").flag().nargs(0);
    out_ap.add_argument("-H", "--barHM").help("Draw [H]orizontal multibar plot [flag]").flag().nargs(0);
    out_ap.add_argument("-T", "--barHS").help("Draw horizontal s[T]acked bar plot [flag]").flag().nargs(0);


    out_ap.add_group("Values options");
    out_ap.add_argument("-x", "--main").help("Specify primary axis by column ID").nargs(1).scan<'d', int>();
    out_ap.add_argument("-y", "--values")
        .help("Specify secondary axis values by column IDs")
        .nargs(1, 6)
        .scan<'d', int>();
    out_ap.add_argument("-c", "--category").help("Specify the column used to group the data").nargs(1).scan<'d', int>();
    out_ap.add_argument("-e", "--filter-extremes")
        .help(std::format("Specify a multiple of standard deviation above and below which data is filtered('0' means "
                          "no filtering) [default = {}]",
                          Config::filter_withinStdDevMultiple_default))
        .nargs(1)
        .scan<'d', int>();


    out_ap.add_group("Size options");
    out_ap.add_argument("-w", "--width").help("Requested width (in characters)").nargs(1).scan<'d', int>();
    out_ap.add_argument("-t", "--height").help("Requested height (in characters)").nargs(1).scan<'d', int>();


    auto &mex_grp = out_ap.add_mutually_exclusive_group();
    out_ap.add_group("Other options:");

    mex_grp.add_argument("-l", "--color-scheme")
        .help("Specify a color scheme by name (the scheme must exist in the config database)")
        .default_value<std::string>("")
        .nargs(1);
    mex_grp.add_argument("-d", "--default-colors")
        .help("Draw with [d]efault colors (Windows Terminal Campbell theme)")
        .flag()
        .nargs(0);
    mex_grp.add_argument("-m", "--monochrome").help("Draw in [m]onochromatic colors").flag().nargs(0);
    out_ap.add_argument("-r", "--force-rgb")
        .help("Always use the RGB SGR way to output colors (virtually never necessary, exists for hypothetical "
              "compatibility reasons only)")
        .flag()
        .nargs(0);
}
} // namespace terminal_plot
} // namespace incom