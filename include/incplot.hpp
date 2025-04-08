#pragma once

#include <incplot/plot_structures.hpp>

namespace incom {
namespace terminal_plot {

template <typename PS_VAR>
class PlotDrawer {
private:
    PS_VAR m_ps_var;

public:
    constexpr PlotDrawer() {};
    PlotDrawer(auto ps_var, DesiredPlot const &dp, DataStore const &ds) {
        auto ol = [&](auto &&var) {
            m_ps_var = std::move(ps_var).build_self(dp, ds);
            int a    = 0;
        };
        std::visit(ol, m_ps_var);
    }

    void update_newPlotStructure(DesiredPlot const &dp, DataStore const &ds) {
        auto ol = [&](auto &&var) { m_ps_var = decltype(var)().build_self(dp, ds); };
        std::visit(ol, m_ps_var);
    }

    bool validate_self() const {
        auto validate = [&](auto &&var) -> bool { return var.validate_self(); };
        return std::visit(validate, m_ps_var);
    }

    std::expected<std::string, Unexp_plotDrawer> validateAndDrawPlot() const {
        // TODO: Add some validation before drawing
        if (validate_self() == false) { return std::unexpected(Unexp_plotDrawer::plotStructureInvalid); }
        else { return drawPlot(); }
    }

    std::string drawPlot() const {
        auto ol = [&](auto &&var) -> std::string { return var.build_plotAsString(); };
        return std::visit(ol, m_ps_var);
    }
};

// This is a map of default constructed 'plot_structures' inside an std::variant
// Pass the 'plot_structure' template types that should be used by the library
// This is the only place where one 'selects' these template types
inline static const auto mp_names2Types =
    detail::generate_variantTypeMap<plot_structures::Base, plot_structures::BarV, plot_structures::BarH,
                                    plot_structures::Multiline, plot_structures::Scatter>();

inline constexpr auto make_plotDrawer(DesiredPlot const &dp, DataStore const &ds) {
    auto ref          = mp_names2Types.at(dp.plot_type_name.value());
    using varType     = decltype(ref);
    auto overload_set = [&](auto const &variantItem) -> PlotDrawer<varType> {
        return PlotDrawer<varType>(variantItem, dp, ds);
    };
    return std::visit(overload_set, ref);
}


} // namespace terminal_plot
} // namespace incom

namespace incplot = incom::terminal_plot;