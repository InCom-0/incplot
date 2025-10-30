#include <format>
#include <optional>
#include <typeindex>

#include <args.hpp>
#include <config.hpp>
#include <incplot/plot_structures.hpp>
#include <incstd/core/typegen.hpp>


namespace incom {
namespace terminal_plot {
namespace cl_args {
std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct(argparse::ArgumentParser const &ap) {

    DesiredPlot::DP_CtorStruct nonDifferentiated;

    auto schemeGetter = [&]() -> void {
        auto setSchemeFromTermOrDefault = [&]() -> bool {
            auto schm_opt = config::get_schemeFromTerminal();
            if (schm_opt) {
                // Use the scheme obtained from the terminal
                nonDifferentiated.colScheme     = schm_opt.value();
                nonDifferentiated.forceRGB_bool = false;
            }
            else {
                // Can't get scheme out of terminal using ANSI control sequences
                // Must revert to default
                nonDifferentiated.colScheme     = config::default_scheme16;
                nonDifferentiated.forceRGB_bool = true;
            }
            return not(nonDifferentiated.forceRGB_bool.value());
        };
        auto dbConn = config::db::get_configConnection(config::appName, config::configFileName);
        if (dbConn.has_value() && config::db::validate_configDB(dbConn.value())) {
            if (ap.is_used("-l")) {
                // Path of explicitly specified theme
                auto schm_name = ap.get<std::string>("-l");
                auto schmExp   = config::db::get_scheme16(dbConn.value(), schm_name);

                if (not schmExp.has_value()) {
                    if (schmExp.error() == config::dbErr::notFound) {
                        nonDifferentiated.additionalInfo.push_back(
                            std::format("The specified scheme: '{}' was not found in the config database.", schm_name));
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(
                            std::string("Internal error of incplot config databse, which is probably unfixable by the "
                                        "user. Incplot will continue to work with limited functionality."));
                    }
                    // Set the default scheme
                    nonDifferentiated.colScheme     = config::default_scheme16;
                    nonDifferentiated.forceRGB_bool = true;
                }
                else {
                    nonDifferentiated.colScheme     = schmExp.value();
                    nonDifferentiated.forceRGB_bool = true;
                }
            }
            else {
                // Path of 'pure default'
                auto lus_exp = config::db::get_defaultScheme16(dbConn.value());
                if (lus_exp) {
                    auto validated = config::validate_terminalPaletteSameness(3, lus_exp.value().palette);
                    if (not validated.has_value()) {
                        // Can't validate beacuse can't get color information out of terminal
                        // Must use the 'unvalidated' scheme from the config
                        nonDifferentiated.colScheme     = lus_exp.value();
                        nonDifferentiated.forceRGB_bool = true;
                        nonDifferentiated.additionalInfo.push_back(
                            std::string("Error encountered while querying terminal for colors.\nUsing default color "
                                        "scheme from config."));
                    }
                    else {
                        if (validated.value()) {
                            // HAPPY PATH
                            nonDifferentiated.colScheme     = lus_exp.value();
                            nonDifferentiated.forceRGB_bool = false;
                        }
                        else {
                            // Validation result == false
                            if (setSchemeFromTermOrDefault()) {
                                nonDifferentiated.colScheme.name = config::fromTerminalSchemeName;
                                auto upsertedSchm =
                                    config::db::upsert_scheme16(dbConn.value(), nonDifferentiated.colScheme);
                            }
                            else {
                                nonDifferentiated.additionalInfo.push_back(std::string(
                                    "Error encountered while querying terminal for colors.\nUsing fall default color "
                                    "scheme."));
                            }
                        }
                    }
                }
                else if (lus_exp.error() == config::dbErr::notFound) {
                    if (setSchemeFromTermOrDefault()) {
                        nonDifferentiated.colScheme.name = "__fromTerminalScheme";
                        auto upsertedSchm = config::db::upsert_scheme16(dbConn.value(), nonDifferentiated.colScheme);

                        if (upsertedSchm) {
                            auto res = config::db::update_default(dbConn.value(), upsertedSchm.value());
                        }
                    }
                    else {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Error encountered while querying terminal for colors.\nUsing fall default color "
                            "scheme."));
                    }
                }
                else {
                    // Can't reach configDB or it is otherwise somehow corrupted
                    nonDifferentiated.additionalInfo.push_back(
                        std::string("Internal error of incplot config databse, which is probably unfixable by the "
                                    "user. Incplot will continue to work with limited functionality."));
                    if (not setSchemeFromTermOrDefault()) {
                        nonDifferentiated.additionalInfo.push_back(std::string(
                            "Error encountered while querying terminal for colors.\nUsing fallback default color "
                            "scheme."));
                    }
                }
            }
        }
        else {
            // Path of 'noDB' available or corrupted configDB
            nonDifferentiated.additionalInfo.push_back(
                std::string("Incplot config database does not seem to exist on your system or its internal structure "
                            "is somehow corrupted. Incplot will continue to work with limited functionality."));
            if (not setSchemeFromTermOrDefault()) {
                nonDifferentiated.additionalInfo.push_back(
                    std::string("Error encountered while querying terminal for colors.\nUsing fallback default color "
                                "scheme."));
            }
        }
    };

    if (auto wdt = ap.present<int>("-w")) { nonDifferentiated.tar_width = wdt.value(); }
    if (auto hgt = ap.present<int>("-t")) { nonDifferentiated.tar_height = hgt.value(); }
    if (auto stddev = ap.present<int>("-e")) {
        if (stddev != 0) { nonDifferentiated.filter_outsideStdDev = stddev.value(); }
        else { nonDifferentiated.filter_outsideStdDev = std::nullopt; }
    }
    else { nonDifferentiated.filter_outsideStdDev = Config::filter_withinStdDevMultiple_default; }


    if (auto optVal = ap.present<int>("-x")) { nonDifferentiated.lts_colID = optVal.value(); }
    if (auto optVal = ap.present<std::vector<int>>("-y")) {
        nonDifferentiated.v_colIDs = std::vector<size_t>(optVal.value().begin(), optVal.value().end());
    }
    if (auto optVal = ap.present<int>("-c")) { nonDifferentiated.c_colID = optVal.value(); }


    if (ap.get<bool>("-d")) {
        nonDifferentiated.colScheme     = color_schemes::defaultScheme16;
        nonDifferentiated.forceRGB_bool = true;
    }
    else if (ap.get<bool>("-m")) {
        nonDifferentiated.colScheme     = color_schemes::other_sources::monochrome;
        nonDifferentiated.forceRGB_bool = true;
    }
    else { schemeGetter(); }
    if (ap.get<bool>("-r")) { nonDifferentiated.forceRGB_bool = true; }

    std::vector<DesiredPlot::DP_CtorStruct> res;
    auto                                    addOne = [&](std::optional<std::type_index> const &&sv_opt) {
        res.push_back(nonDifferentiated);
        res.back().plot_type_name = sv_opt;
    };
    using namespace incom::standard::typegen;
    if (ap.get<bool>("-B")) { addOne(get_typeIndex<plot_structures::BarV>()); }
    if (ap.get<bool>("-S")) { addOne(get_typeIndex<plot_structures::Scatter>()); }
    if (ap.get<bool>("-L")) { addOne(get_typeIndex<plot_structures::Multiline>()); }
    if (ap.get<bool>("-V")) { addOne(get_typeIndex<plot_structures::BarVM>()); }
    if (ap.get<bool>("-H")) { addOne(get_typeIndex<plot_structures::BarHM>()); }
    if (ap.get<bool>("-T")) { addOne(get_typeIndex<plot_structures::BarHS>()); }
    if (res.empty()) { addOne(std::nullopt); }

    return res;
}

std::vector<DesiredPlot::DP_CtorStruct> get_dpCtorStruct() {
    return std::vector<DesiredPlot::DP_CtorStruct>{DesiredPlot::DP_CtorStruct{.plot_type_name = std::nullopt}};
}

void finishAp(argparse::ArgumentParser &out_ap) {
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
    mex_grp.add_argument("-d", "--default-colors").help("Draw with [d]efault colors (dimidium theme)").flag().nargs(0);
    mex_grp.add_argument("-m", "--monochrome").help("Draw in [m]onochromatic colors").flag().nargs(0);
    out_ap.add_argument("-s", "--show-schemes").help("Show all available color [s]chemes and exit").flag().nargs(0);
    out_ap.add_argument("-r", "--force-rgb")
        .help("Always use the RGB SGR way to output colors (virtually never necessary, exists for hypothetical "
              "compatibility reasons only)")
        .flag()
        .nargs(0);
}

void populateAp(argparse::ArgumentParser &out_ap, int argc, const char *const *argv) {
    try {
        out_ap.parse_args(argc, argv);
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << out_ap;
        std::exit(1);
    }
}
} // namespace cl_args
} // namespace terminal_plot
} // namespace incom