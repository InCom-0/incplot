#pragma once

#include <cmath>
#include <codecvt>
#include <format>
#include <locale>

#include <incplot/config.hpp>
#include <incplot/detail/concepts.hpp>
#include <incplot/detail/misc.hpp>


namespace incom {
namespace terminal_plot {
namespace detail {
constexpr inline std::string convert_u32u8(std::u32string const &str) {
    static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(str);
}

constexpr inline std::u32string convert_u32u8(std::string const &str) {
    static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(str);
}

// Compute 'on display' size of a string (correctly taking into account UTF8 glyphs)
constexpr inline std::size_t strlen_utf8(const std::string &str) {
    std::size_t length = 0;
    for (char c : str) {
        if ((c & 0xC0) != 0x80) { ++length; }
    }
    return length;
}

constexpr std::string trim2Size_leading(std::string const &str, size_t maxSize) {
    // TODO: Need to somehow handle unicode in labels in this function
    if (str.size() > maxSize) {
        size_t cutPoint = maxSize / 2;
        return std::string(str.begin(), str.begin() + cutPoint)
            .append("...")
            .append(str.begin() + cutPoint + 3 + (str.size() - maxSize), str.end());
    }
    else { return std::string(maxSize - strlen_utf8(str), Config::space).append(str); }
}
constexpr std::string trim2Size_leading(std::string const &&str, size_t maxSize) {
    return trim2Size_leading(str, maxSize);
}
constexpr std::string trim2Size_leadingEnding(std::string const &str, size_t maxSize) {
    // TODO: Need to somehow handle unicode in labels in this function
    if (str.size() > maxSize) {
        size_t cutPoint = maxSize / 2;
        return std::string(str.begin(), str.begin() + cutPoint)
            .append("...")
            .append(str.begin() + cutPoint + 3 + (str.size() - maxSize), str.end());
    }
    else {
        return std::string((maxSize - strlen_utf8(str)) / 2, Config::space)
            .append(str)
            .append(
                std::string(((maxSize - strlen_utf8(str)) / 2) + ((maxSize - strlen_utf8(str)) % 2), Config::space));
    }
}
constexpr std::string trim2Size_leadingEnding(std::string const &&str, size_t maxSize) {
    return trim2Size_leadingEnding(str, maxSize);
}

constexpr size_t get_axisFillerSize(size_t axisLength, size_t axisStepCount) {
    return (axisLength - axisStepCount) / (axisStepCount + 1);
}

// TODO: Also make a version where the tick positions are explictily specified in one vector of size_t
constexpr inline std::vector<std::string> create_tickMarkedAxis(std::string filler, std::string tick, size_t steps,
                                                                size_t totalLength) {
    size_t fillerSize = get_axisFillerSize(totalLength, steps);

    std::vector<std::string> res;
    for (size_t i_step = 0; i_step < steps; ++i_step) {
        for (size_t i_filler = 0; i_filler < fillerSize; ++i_filler) {
            res.push_back(TermColors::get_coloured(filler, Config::color_Axes_enum));
        }
        res.push_back(TermColors::get_coloured(tick, Config::color_Axes_enum));
    }
    size_t sizeOfRest = totalLength - (steps) - (steps * fillerSize);
    for (size_t i_filler = 0; i_filler < sizeOfRest; ++i_filler) {
        res.push_back(TermColors::get_coloured(filler, Config::color_Axes_enum));
    }
    return res;
}
constexpr inline size_t guess_stepsOnHorAxis(size_t width, size_t maxLabelSize = Config::max_valLabelSize) {
    // Substract the beginning and the end label sizes and -2 for spacing
    width += (-2 * maxLabelSize + 2) - 2;
    return (width / (maxLabelSize + 4));
}
constexpr inline size_t guess_stepsOnVerAxis(size_t height, size_t verticalStepSize = Config::axis_stepSize_vl) {
    // Substract the beginning and the end label sizes and -2 for spacing
    height += -(verticalStepSize - 1);
    return (height / verticalStepSize);
}


// Ring vector for future usage in 'scrolling plot' scenarios
template <typename T>
class RingVector {
private:
    std::vector<T> _m_buf;
    size_t         head        = 0;
    size_t         nextRead_ID = 0;

public:
    RingVector(std::vector<T> &&t) : _m_buf(t) {};
    RingVector(std::vector<T> &t) : _m_buf(t) {};

    // TODO: Might not need to create a copy here or below once std::views::concatenate from C++26 exists
    std::vector<T> create_copy() {
        std::vector<T> res(_m_buf.begin() + head, _m_buf.end());
        for (int i = 0; i < head; ++i) { res.push_back(_m_buf[i]); }
        return res;
    }

    std::vector<T> create_copy_reversed() {
        std::vector<T> res(_m_buf.rbegin() + (_m_buf.size() - head), _m_buf.rend());
        for (int i = (_m_buf.size() - 1); i >= head; --i) { res.push_back(_m_buf[i]); }
        return res;
    }

    T get_cur() const { return _m_buf[nextRead_ID]; }
    T get_cur_and_next() {
        T res = get_cur();
        advanceByOne();
        return res;
    }
    T get_cur_and_advanceBy(size_t by = 1) {
        T res = get_cur();
        advanceBy(by);
        return res;
    }

    void inline advanceByOne() { nextRead_ID = (nextRead_ID + 1) % _m_buf.size(); }
    void inline advanceBy(int by = 1) { nextRead_ID = (nextRead_ID + by) % _m_buf.size(); }

    // On insertion resets nextRead_ID to head as well
    void insertAtHead(T &&item) { insertAtHead(item); }
    void insertAtHead(T const &item) {
        _m_buf[head] = item;
        head         = (head + 1) % _m_buf.size();
        nextRead_ID  = head;
    }
};

template <typename T>
requires std::is_arithmetic_v<std::decay_t<T>>
constexpr inline std::pair<double, std::optional<std::string>> rebase_2_SIPrefix(T &&value) {
    if (value == 0) { return {0, ""}; }
    else {
        int target = value >= 1 ? (std::log10(value) / 3) : (std::log10(value) / 3) - 1;
        return {value / std::pow(1000, target), Config::si_prefixes.at(target + 10)};
    }
}

template <typename T>
requires std::is_arithmetic_v<std::decay_t<T>>
constexpr inline std::string format_toMax5length(T &&val) {
    auto [rbsed, unit] = rebase_2_SIPrefix(std::forward<decltype(val)>(val));
    return std::format("{:.{}f}{}", rbsed, rbsed >= 10 ? 0 : 1, unit.value_or(""));
}

class BrailleDrawer {
private:
    std::vector<std::vector<std::u32string>> m_canvasColors;
    std::vector<std::vector<char32_t>>       m_canvasBraille;
    std::vector<std::u32string>              m_colorPallete = std::vector<std::u32string>{
        detail::convert_u32u8(Config::color_Vals1), detail::convert_u32u8(Config::color_Vals2),
        detail::convert_u32u8(Config::color_Vals3)};
    std::u32string s_terminalDefault = detail::convert_u32u8(Config::term_setDefault);


    BrailleDrawer() {};
    BrailleDrawer(size_t canvas_width, size_t canvas_height)
        : m_canvasColors(std::vector(canvas_height, std::vector<std::u32string>(canvas_width, U""))),
          m_canvasBraille(std::vector(canvas_height, std::vector<char32_t>(canvas_width, Config::braille_blank))) {};

public:
    template <typename Y, typename X>
    requires std::is_arithmetic_v<typename X::value_type> && std::is_arithmetic_v<typename Y::value_type>
    static constexpr std::vector<std::string> drawPoints(size_t canvas_width, size_t canvas_height, Y const &&y_values,
                                                         X const &&x_values) {
        return drawPoints(canvas_width, canvas_height, y_values, x_values);
    }

    template <typename Y, typename X>
    requires std::is_arithmetic_v<typename X::value_type> && std::is_arithmetic_v<typename Y::value_type>
    static constexpr std::vector<std::string> drawPoints(size_t canvas_width, size_t canvas_height, Y const &y_values,
                                                         X const &x_values) {
        BrailleDrawer bd(canvas_width, canvas_height);

        auto [yMin, yMax] = std::ranges::minmax(y_values);
        auto [xMin, xMax] = std::ranges::minmax(x_values);

        auto yStepSize = (yMax - yMin) / ((static_cast<double>(canvas_height) * 4) - 1);
        auto xStepSize = (xMax - xMin) / ((static_cast<double>(canvas_width) * 2) - 1);

        auto placePointOnCanvas = [&](auto const &yVal, auto const &xVal) {
            auto y       = static_cast<size_t>(((yVal - yMin) / yStepSize)) / 4;
            auto yChrPos = static_cast<size_t>(((yVal - yMin) / yStepSize)) % 4;

            auto x       = static_cast<size_t>(((xVal - xMin) / xStepSize)) / 2;
            auto xChrPos = static_cast<size_t>(((xVal - xMin) / xStepSize)) % 2;

            bd.m_canvasBraille[y][x] |= Config::braille_map[yChrPos][xChrPos];
            bd.m_canvasColors[y][x]   = bd.m_colorPallete.front();
        };

        for (size_t i = 0; i < x_values.size(); ++i) { placePointOnCanvas(y_values[i], x_values[i]); }

        std::vector<std::string> res;
        for (int rowID = bd.m_canvasBraille.size() - 1; rowID > -1; --rowID) {
            std::u32string oneLine;
            for (size_t colID = 0; colID < bd.m_canvasBraille.front().size(); ++colID) {
                if (bd.m_canvasColors[rowID][colID].empty()) { oneLine.push_back(bd.m_canvasBraille[rowID][colID]); }
                else {
                    oneLine.append(bd.m_canvasColors[rowID][colID]);
                    oneLine.push_back(bd.m_canvasBraille[rowID][colID]);
                    oneLine.append(bd.s_terminalDefault);
                }
            }
            res.push_back(detail::convert_u32u8(oneLine));
        }

        return res;
    }
};

} // namespace detail
} // namespace terminal_plot
} // namespace incom