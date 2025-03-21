#pragma once

#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <expected>
#include <format>
#include <functional>
#include <more_concepts/more_concepts.hpp>
#include <nlohmann/json.hpp>
#include <oof.h>
#include <optional>
#include <print>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>


namespace incom {
namespace terminal_plot {

enum class Color_CVTS {
    Default                   = 0,
    Bold_or_Bright            = 1,
    No_bold_or_bright         = 22,
    Underline                 = 4,
    No_underline              = 24,
    Negative                  = 7,
    Positive_No_negative      = 27,
    Foreground_Black          = 30,
    Foreground_Red            = 31,
    Foreground_Green          = 32,
    Foreground_Yellow         = 33,
    Foreground_Blue           = 34,
    Foreground_Magenta        = 35,
    Foreground_Cyan           = 36,
    Foreground_White          = 37,
    Foreground_Extended       = 38,
    Foreground_Default        = 39,
    Background_Black          = 40,
    Background_Red            = 41,
    Background_Green          = 42,
    Background_Yellow         = 43,
    Background_Blue           = 44,
    Background_Magenta        = 45,
    Background_Cyan           = 46,
    Background_White          = 47,
    Background_Extended       = 48,
    Background_Default        = 49,
    Bright_Foreground_Black   = 90,
    Bright_Foreground_Red     = 91,
    Bright_Foreground_Green   = 92,
    Bright_Foreground_Yellow  = 93,
    Bright_Foreground_Blue    = 94,
    Bright_Foreground_Magenta = 95,
    Bright_Foreground_Cyan    = 96,
    Bright_Foreground_White   = 97,
    Bright_Background_Black   = 100,
    Bright_Background_Red     = 101,
    Bright_Background_Green   = 102,
    Bright_Background_Yellow  = 103,
    Bright_Background_Blue    = 104,
    Bright_Background_Magenta = 105,
    Bright_Background_Cyan    = 106,
    Bright_Background_White   = 107
};

class ColorMap {
private:
    static constexpr auto const _m_colorMap = std::array<std::pair<int, std::string_view>, 43>{{
        {0, "\x1b[0m"},     {1, "\x1b[1m"},     {22, "\x1b[22m"},   {4, "\x1b[4m"},     {24, "\x1b[24m"},
        {7, "\x1b[7m"},     {27, "\x1b[27m"},   {30, "\x1b[30m"},   {31, "\x1b[31m"},   {32, "\x1b[32m"},
        {33, "\x1b[33m"},   {34, "\x1b[34m"},   {35, "\x1b[35m"},   {36, "\x1b[36m"},   {37, "\x1b[37m"},
        {38, "\x1b[38m"},   {39, "\x1b[39m"},   {40, "\x1b[40m"},   {41, "\x1b[41m"},   {42, "\x1b[42m"},
        {43, "\x1b[43m"},   {44, "\x1b[44m"},   {45, "\x1b[45m"},   {46, "\x1b[46m"},   {47, "\x1b[47m"},
        {48, "\x1b[48m"},   {49, "\x1b[49m"},   {90, "\x1b[90m"},   {91, "\x1b[91m"},   {92, "\x1b[92m"},
        {93, "\x1b[93m"},   {94, "\x1b[94m"},   {95, "\x1b[95m"},   {96, "\x1b[96m"},   {97, "\x1b[97m"},
        {100, "\x1b[100m"}, {101, "\x1b[101m"}, {102, "\x1b[102m"}, {103, "\x1b[103m"}, {104, "\x1b[104m"},
        {105, "\x1b[105m"}, {106, "\x1b[106m"}, {107, "\x1b[107m"},
    }};

public:
    static consteval auto get_termColSV(Color_CVTS const col) {
        return std::ranges::find_if(_m_colorMap, [&](auto &&pr) { return pr.first == static_cast<int>(col); })->second;
    }
};


// FORWARD DELCARATIONS
struct DataStore;
struct Parser;
class DesiredPlot;

namespace plot_structures {
class Base;
class BarV;
class BarH;
class Line;
class Multiline;
class Scatter;
class Bubble;
} // namespace plot_structures

enum class Unexp_plotSpecs {
    plotType,
    labelCol,
    valCols,
    namesIntoIDs_label,
    namesIntoIDs_vals,
    guessValCols,
    tarWidth,
    tarHeight,
    axisTicks
};
enum class Err_drawer {
    plotStructureInvalid,
    barVplot_tooWide
};

namespace detail {
constexpr inline std::size_t strlen_utf8(const std::string &str) {
    std::size_t length = 0;
    for (char c : str) {
        if ((c & 0xC0) != 0x80) { ++length; }
    }
    return length;
}

/*
Quasi compile time reflection for typenames
*/
template <typename T>
constexpr auto TypeToString() {
    auto EmbeddingSignature = std::string{std::source_location::current().function_name()};
    auto firstPos           = EmbeddingSignature.rfind("::") + 2;
    return EmbeddingSignature.substr(firstPos, EmbeddingSignature.size() - firstPos - 1);
}

template <typename... Ts>
constexpr bool __none_sameLastLevelTypeName_HLPR() {
    std::vector<std::string> vect;
    (vect.push_back(TypeToString<Ts>()), ...);
    std::ranges::sort(vect, std::less());
    auto [beg, end] = std::ranges::unique(vect);
    vect.erase(beg, end);
    return vect.size() == sizeof...(Ts);
}

// Just the 'last level' type name ... not the fully qualified typename
template <typename... Ts>
concept none_sameLastLevelTypeName = __none_sameLastLevelTypeName_HLPR<Ts...>();

constexpr inline std::string middleTrim2Size(std::string const &str, size_t maxSize) {
    if (str.size() > maxSize) {
        size_t cutPoint = maxSize / 2;
        return std::string(str.begin(), str.begin() + cutPoint)
            .append("...")
            .append(str.begin() + cutPoint + 3 + (str.size() - maxSize), str.end());
    }
    else { return std::string(maxSize - str.size(), ' ').append(str); }
}
// TODO: Also make a version where the tick positions are explictily specified in one vector of size_t
constexpr inline std::vector<std::string> create_tickMarkedAxis(std::string filler, std::string tick, size_t steps,
                                                                size_t totalWidth) {
    size_t fillerSize = (totalWidth - steps) / (steps + 1);

    std::vector<std::string> res;
    for (size_t i_step = 0; i_step < steps; ++i_step) {
        for (size_t i_filler = 0; i_filler < fillerSize; ++i_filler) { res.push_back(std::string(filler)); }
        res.push_back(tick);
    }
    for (size_t i_filler = 0; i_filler < fillerSize; ++i_filler) { res.push_back(std::string(filler)); }
    return res;
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

inline constexpr std::array<std::string, 21> const arr{"q", "r", "y", "z", "a", "f", "p", "n", "μ", "m", "",
                                                       "k", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q"};

template <typename T>
requires std::is_arithmetic_v<T>
inline std::pair<double, std::optional<std::string>> rebase_2_SIPreFix(T &&value) {
    int target = value >= 1 ? (std::log10(value) / 3) : INT_MIN;
    if (target == INT_MIN) { return {value, std::nullopt}; }
    else { return {value / std::pow(1000, target), arr.at(target + 10)}; }
}

template <typename T>
requires std::is_arithmetic_v<T>
std::string format_toMax6length(T &&val) {
    auto [rbsed, unit] = rebase_2_SIPreFix(std::forward<decltype(val)>(val));
    return std::format("{:.{}f}{}", rbsed, unit.has_value() ? 1 : 4, unit.value_or(""));
}

} // namespace detail


using NLMjson = nlohmann::json;

// Data storage for the actual data that are to be plotted
struct DataStore {
    // The json this was constucted with/from ... possibly not strictly necessary to keep, but whatever
    std::vector<NLMjson> constructedWith;

    // Data descriptors
    std::vector<std::string>                         colNames;
    std::vector<std::pair<NLMjson::value_t, size_t>> colTypes; // First =  ColType, Second = ID in data vector

    // Actual data storage
    std::vector<std::vector<std::string>> stringCols;
    std::vector<std::vector<long long>>   llCols; // Don't care about signed unsigned, etc. ... all will be long long
    std::vector<std::vector<double>>      doubleCols;


    // CONSTRUCTION
    DataStore(std::vector<NLMjson> &&jsonVec) : constructedWith(std::move(jsonVec)) {

        // Create the requisite data descriptors and the structure
        for (auto const &[key, val] : constructedWith.front().items()) {
            colNames.push_back(key);

            if (val.type() == NLMjson::value_t::string) {
                colTypes.push_back({NLMjson::value_t::string, stringCols.size()});
                stringCols.push_back(std::vector<std::string>());
            }
            else if (val.type() == NLMjson::value_t::number_float) {
                colTypes.push_back({NLMjson::value_t::number_float, doubleCols.size()});
                doubleCols.push_back(std::vector<double>());
            }
            else if (val.type() == NLMjson::value_t::number_integer ||
                     val.type() == NLMjson::value_t::number_unsigned) {
                colTypes.push_back({val.type(), llCols.size()});
                llCols.push_back(std::vector<long long>());
            }
        }
        append_data(constructedWith);
    }

    // APPENDING
    void append_data(std::vector<NLMjson> const &toAppend) {
        for (auto const &oneJson : toAppend) {
            auto oneJsonIT = oneJson.items().begin();
            for (auto const &colTypesPair : colTypes) {
                if (colTypesPair.first == NLMjson::value_t::string) {
                    stringCols[colTypesPair.second].push_back(oneJsonIT.value());
                }
                else if (colTypesPair.first == NLMjson::value_t::number_integer ||
                         colTypesPair.first == NLMjson::value_t::number_unsigned) {
                    llCols[colTypesPair.second].push_back(static_cast<long long>(oneJsonIT.value()));
                }
                else if (colTypesPair.first == NLMjson::value_t::number_float) {
                    doubleCols[colTypesPair.second].push_back(oneJsonIT.value());
                }
                ++oneJsonIT;
            }
        }
    }
    void append_jsonAndData(std::vector<NLMjson> const &toAppend) {
        constructedWith.insert(constructedWith.end(), toAppend.begin(), toAppend.end());
        append_data(toAppend);
    }
};

// Encapsulates parsing of the input into DataStore
// Validates 'hard' errors during parsing
// Validates that input data is not structured 'impossibly' (missing values, different value names per record, etc.)
struct Parser {

    static bool validate_jsonSameness(std::vector<NLMjson> const &jsonVec) {
        // Validate that all the JSON objects parsed above have the same structure
        for (auto const &js : jsonVec) {
            // Different number of items in this line vs the firstline
            if (js.size() != jsonVec.front().size()) { return false; } // throw something here

            auto firstLineIT = jsonVec.front().items().begin();
            for (auto const &[key, val] : js.items()) {

                // Key is not the same as in the first line
                if (key != firstLineIT.key()) { return false; } // throw something here

                // Type is not the same as in the first line
                if (val.type() != firstLineIT.value().type()) { return false; } // throw something here
                ++firstLineIT;
            }
        }
        return true;
    }

    static bool validate_jsonSameness(NLMjson const &json_A, NLMjson const &json_B) {
        if (json_A.size() != json_B.size()) { return false; }

        auto json_A_IT = json_A.items().begin();
        auto json_B_IT = json_B.items().begin();
        for (size_t i = 0; i < json_A.size(); ++i) {
            if (json_A_IT.key() != json_B_IT.key()) { return false; }
            if (json_B_IT.value().type() != json_B_IT.value().type()) { return false; }
            json_A_IT++, json_B_IT++;
        }
        return true;
    }

    template <typename T>
    requires std::is_convertible_v<T, std::string_view>
    static std::vector<NLMjson> parse_NDJSON(T const &stringLike) {

        // Trim input 'string like' of all 'newline' chars at the end
        auto const it =
            std::ranges::find_if_not(stringLike.rbegin(), stringLike.rend(), [](auto &&chr) { return chr == '\n'; });
        std::string_view const trimmed(stringLike.begin(), stringLike.end() - (it - stringLike.rbegin()));

        std::vector<NLMjson> parsed;
        for (auto const &oneLine : std::views::split(trimmed, '\n') |
                                       std::views::transform([](auto const &in) { return std::string_view(in); })) {
            NLMjson oneLineJson;
            try {
                oneLineJson = NLMjson::parse(oneLine);
            }
            catch (const NLMjson::exception &e) {
                // TODO: Finally figure out how to handle exceptions somewhat professionally
                std::print("{}\n", e.what());
            }
            parsed.push_back(std::move(oneLineJson));
        }
        return parsed;
    }

    template <typename T>
    requires std::is_convertible_v<T, std::string_view>
    static void parse_NDJSON_andAddTo(T const &stringLike, DataStore &out_DS_toAppend) {

        auto parsed = parse_NDJSON(stringLike);
        if (not validate_jsonSameness(parsed)) {} // Throw something here
        if (not validate_jsonSameness(parsed.front(), out_DS_toAppend.constructedWith.front())) {
        } // Throw something here

        out_DS_toAppend.append_jsonAndData(parsed);
    }


    template <typename T>
    requires std::is_convertible_v<T, std::string_view>
    static DataStore parse_NDJSON_intoDS(T const &stringLike) {

        auto parsed = parse_NDJSON(stringLike);
        if (not validate_jsonSameness(parsed)) {} // Throw something here

        return DataStore(std::move(parsed));
    }

    template <typename T>
    requires more_concepts::container<T> && std::is_convertible_v<typename T::value_type, std::string_view>
    static DataStore parse_NDJSON(T containerOfStringLike) {

        for (auto &oneStr : containerOfStringLike) {
            while (oneStr.back() == '\n') { oneStr.popback(); }
        }

        std::vector<NLMjson> parsed;
        for (auto const &contItem : containerOfStringLike) {
            for (auto const oneLine : std::views::split(contItem, '\n') |
                                          std::views::transform([](auto const &in) { return std::string_view(in); })) {

                NLMjson oneLineJson;
                try {
                    oneLineJson = NLMjson::parse(oneLine);
                }
                catch (const NLMjson::exception &e) {
                    std::print("{}\n", e.what());
                }
                parsed.push_back(std::move(oneLineJson));
            }
        }
        return DataStore(std::move(parsed));
    }
};

// Encapsulates the 'instructions' information about the kind of plot that is desired by the user
// Big feature is that it includes logic for 'auto guessing' the 'instructions' that were not provided explicitly
// Basically 4 important things: 1) Type of plot, 2) Labels to use (if any), 3) Values to use, 4) Size in'chars'
class DesiredPlot {
private:
    static std::expected<DesiredPlot, Unexp_plotSpecs> transform_namedColsIntoIDs(DesiredPlot    &&dp,
                                                                                  DataStore const &ds) {
        if (dp.label_colName.has_value()) {
            auto it = std::ranges::find(ds.colNames, dp.label_colName.value());
            if (it == ds.colNames.end()) { return std::unexpected(Unexp_plotSpecs::namesIntoIDs_label); }
            else if (not dp.label_colID.has_value()) { dp.label_colID = it - ds.colNames.begin(); }
            else if ((it - ds.colNames.begin()) == dp.label_colID.value()) { dp.label_colName = std::nullopt; }
            else { return std::unexpected(Unexp_plotSpecs::namesIntoIDs_label); }
        }

        for (auto const &v_colName : dp.values_colNames) {
            auto it = std::ranges::find(ds.colNames, v_colName);
            if (it == ds.colNames.end()) { return std::unexpected(Unexp_plotSpecs::namesIntoIDs_label); }

            auto it2 = std::ranges::find(dp.values_colIDs, it - ds.colNames.begin());
            if (it2 == dp.values_colIDs.end()) { dp.values_colIDs.push_back(it2 - dp.values_colIDs.begin()); }

            dp.values_colNames.clear();
        }
        return dp;
    }
    static std::expected<DesiredPlot, Unexp_plotSpecs> guess_plotType(DesiredPlot &&dp, DataStore const &ds) {
        if (dp.plot_type_name.has_value()) { return dp; }

        if (dp.values_colIDs.size() > 3) { return std::unexpected(Unexp_plotSpecs::valCols); }
        else if (dp.values_colIDs.size() < 2) { dp.plot_type_name = detail::TypeToString<plot_structures::BarV>(); }
        else if (dp.values_colIDs.size() == 3) { dp.plot_type_name = detail::TypeToString<plot_structures::Bubble>(); }
        else if (dp.values_colIDs.size() == 2) { dp.plot_type_name = detail::TypeToString<plot_structures::Scatter>(); }
        // This thing never triggers
        else { dp.plot_type_name = detail::TypeToString<plot_structures::Multiline>(); }

        return dp;
    }
    static std::expected<DesiredPlot, Unexp_plotSpecs> guess_labelCol(DesiredPlot &&dp, DataStore const &ds) {
        if (dp.label_colID.has_value()) { return dp; }
        else if (dp.plot_type_name != detail::TypeToString<plot_structures::BarV>()) { return dp; }
        else {
            auto it = std::ranges::find_if(ds.colTypes, [](auto &&a) { return a.first == NLMjson::value_t::string; });
            if (it == ds.colTypes.end()) { return std::unexpected(Unexp_plotSpecs::labelCol); }
            else { dp.label_colID = it->second; }
            return dp;
        }
    }
    static std::expected<DesiredPlot, Unexp_plotSpecs> guess_valueCols(DesiredPlot &&dp, DataStore const &ds) {

        auto addValColsUntil = [&](size_t count) -> std::expected<size_t, Unexp_plotSpecs> {
            auto valColTypes = std::views::filter(ds.colTypes, [](auto &&a) {
                return (a.first == NLMjson::value_t::number_float || a.first == NLMjson::value_t::number_integer ||
                        a.first == NLMjson::value_t::number_unsigned);
            });

            auto getAnotherValColID = [&]() -> std::expected<size_t, Unexp_plotSpecs> {
                for (auto const &vct : valColTypes) {
                    if (std::ranges::find(dp.values_colIDs, vct.second) == dp.values_colIDs.end()) {
                        return vct.second;
                    }
                }
                // Cannot find another one
                return std::unexpected(Unexp_plotSpecs::guessValCols);
            };
            while (dp.values_colIDs.size() < count) {
                auto expID = getAnotherValColID();
                if (expID.has_value()) { dp.values_colIDs.push_back(expID.value()); }
                else { return std::unexpected(expID.error()); }
            }
            return 0uz;
        };

        // BAR PLOTS
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarV>()) {
            if (dp.values_colIDs.size() > 1) { return std::unexpected(Unexp_plotSpecs::valCols); }
            else if (not addValColsUntil(1).has_value()) { return std::unexpected(Unexp_plotSpecs::guessValCols); }
        }
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarH>()) {
            if (dp.values_colIDs.size() > 1) { return std::unexpected(Unexp_plotSpecs::valCols); }
            else if (not addValColsUntil(1).has_value()) { return std::unexpected(Unexp_plotSpecs::guessValCols); }
        }
        // LINE PLOTS
        else if (dp.plot_type_name == detail::TypeToString<plot_structures::Line>()) {
            if (dp.values_colIDs.size() > 1) { return std::unexpected(Unexp_plotSpecs::valCols); }
            else if (not addValColsUntil(1).has_value()) { return std::unexpected(Unexp_plotSpecs::guessValCols); }
        }
        else if (dp.plot_type_name == detail::TypeToString<plot_structures::Multiline>()) {
            if (dp.values_colIDs.size() > 3) { return std::unexpected(Unexp_plotSpecs::valCols); }
            else if (not addValColsUntil(2).has_value()) { return std::unexpected(Unexp_plotSpecs::guessValCols); }
        }

        // SCATTER PLOT
        else if (dp.plot_type_name == detail::TypeToString<plot_structures::Scatter>()) {
            if (dp.values_colIDs.size() > 2) { return std::unexpected(Unexp_plotSpecs::valCols); }
            else if (not addValColsUntil(2).has_value()) { return std::unexpected(Unexp_plotSpecs::guessValCols); }
        }
        // BUBBLE PLOT
        else if (dp.plot_type_name == detail::TypeToString<plot_structures::Bubble>()) {
            if (dp.values_colIDs.size() > 3) { return std::unexpected(Unexp_plotSpecs::valCols); }
            else if (not addValColsUntil(3).has_value()) { return std::unexpected(Unexp_plotSpecs::guessValCols); }
        }
        return dp;
    }
    static std::expected<DesiredPlot, Unexp_plotSpecs> guess_sizes(DesiredPlot &&dp, DataStore const &ds) {
        // Width always need to be provided, otherwise the whole thing doesn't work
        if (not dp.targetWidth.has_value() || dp.targetWidth.value() < 16) {
            return std::unexpected(Unexp_plotSpecs::tarWidth);
        }

        // Height can be inferred
        if (not dp.targetHeight.has_value()) { dp.targetHeight = dp.targetWidth.value() / 2; }

        // Impossible to print with height <3 under all circumstances
        if (dp.targetHeight.value() < 3) { return std::unexpected(Unexp_plotSpecs::tarWidth); }


        return dp;
    }
    static std::expected<DesiredPlot, Unexp_plotSpecs> guess_TFfeatures(DesiredPlot &&dp, DataStore const &ds) {
        if (not dp.valAxesNames_bool.has_value()) { dp.valAxesNames_bool = false; }
        if (not dp.valAxesLabels_bool.has_value()) { dp.valAxesLabels_bool = false; }
        if (not dp.valAutoFormat_bool.has_value()) { dp.valAutoFormat_bool = true; }
        if (not dp.legend_bool.has_value()) { dp.legend_bool = false; }

        return dp;
    }

public:
    std::optional<std::string> plot_type_name;

    std::optional<size_t>      label_colID; // ID in colTypes
    std::optional<std::string> label_colName;

    // TODO: Make both 'values_' into std::optional as well to keep the logic the same for all here
    std::vector<size_t>      values_colIDs; // IDs in colTypes
    std::vector<std::string> values_colNames;

    std::optional<size_t> targetHeight;
    std::optional<size_t> targetWidth;

    std::optional<bool> valAxesNames_bool;
    std::optional<bool> valAxesLabels_bool;
    std::optional<bool> valAutoFormat_bool;
    std::optional<bool> legend_bool;


    // TODO: Provide some compile time programmatic way to set the default sizes here
    DesiredPlot(std::optional<size_t> tar_width = std::nullopt, std::optional<size_t> tar_height = std::nullopt,
                std::optional<std::string> plot_type_name = std::nullopt, std::optional<size_t> l_colID = std::nullopt,
                std::optional<std::string> l_colName = std::nullopt, std::vector<size_t> v_colIDs = {},
                std::vector<std::string> v_colNames = {})
        : targetWidth(tar_width), targetHeight(tar_height), plot_type_name(std::move(plot_type_name)),
          label_colID(std::move(l_colID)), label_colName(std::move(l_colName)), values_colIDs(std::move(v_colIDs)),
          values_colNames(std::move(v_colNames)) {}

    // Create a new copy and guess_missingParams on it.
    std::expected<DesiredPlot, Unexp_plotSpecs> build_guessedParamsCPY(this DesiredPlot &self, DataStore const &ds) {
        return DesiredPlot(self).guess_missingParams(ds);
    }

    // Guesses the missing 'desired parameters' and returns a DesiredPlot with those filled in
    // Variation on a 'builder pattern'
    // Normally called 'in place' on 'DesiredPlot' instance constructed as rvalue
    // If impossible to guess or otherwise the user desires something impossible returns Err_plotSpecs.
    std::expected<DesiredPlot, Unexp_plotSpecs> guess_missingParams(this DesiredPlot &&self, DataStore const &ds) {

        // TODO: Could use std::bind for these ... had some trouble with that ... maybe return to it later.
        // Still can't quite figure it out ...  std::bind_back doesn't seem to cooperate with and_then ...

        auto gpt = [&](DesiredPlot &&dp) -> std::expected<DesiredPlot, Unexp_plotSpecs> {
            return DesiredPlot::guess_plotType(std::forward<decltype(dp)>(dp), ds);
        };
        auto glc = [&](DesiredPlot &&dp) -> std::expected<DesiredPlot, Unexp_plotSpecs> {
            return DesiredPlot::guess_labelCol(std::forward<decltype(dp)>(dp), ds);
        };
        auto gvc = [&](DesiredPlot &&dp) -> std::expected<DesiredPlot, Unexp_plotSpecs> {
            return DesiredPlot::guess_valueCols(std::forward<decltype(dp)>(dp), ds);
        };
        auto gsz = [&](DesiredPlot &&dp) -> std::expected<DesiredPlot, Unexp_plotSpecs> {
            return DesiredPlot::guess_sizes(std::forward<decltype(dp)>(dp), ds);
        };
        auto gtff = [&](DesiredPlot &&dp) -> std::expected<DesiredPlot, Unexp_plotSpecs> {
            return DesiredPlot::guess_TFfeatures(std::forward<decltype(dp)>(dp), ds);
        };

        return DesiredPlot::transform_namedColsIntoIDs(std::forward<decltype(self)>(self), ds)
            .and_then(gpt)
            .and_then(glc)
            .and_then(gvc)
            .and_then(gsz)
            .and_then(gtff);
    }
};


namespace plot_structures {
// Classes derived from base represent 'plot structures' of particular types of plots (such as bar vertical, scatter
// etc.)
// Create your own 'plot structure' ie. type of plot by deriving from 'Base' class (or from other classes derived from
// it) and overriding pure virtual functions. The types properly derived from 'Base' can then be used inside
// 'PlotDrawer' inside std::variant<...>. The idea is to be able to easily customize and also possibly 'partially
// customize' as needed You always have to make the 'Base' class a friend ... this enables really nice dynamic
// polymorphism coupled with 'deducing this' feature of C++23
class Base {
protected:
    // Descriptors - First thing to be computed.
    // BEWARE: The sizes are 'as displayed' not the 'size in bytes' ... need to account for UTF8
    size_t areaWidth = 0, areaHeight = 0;
    size_t labels_verLeftWidth = 0, labels_verRightWidth = 0;

    size_t axis_verLeftSteps = 1, axis_varRightSteps = 1, axis_horTopSteps = 1, axis_horBottomSteps = 1;

    size_t pad_left = 2, pad_right = 0, pad_top = 0, pad_bottom = 0;

    bool labels_horTop_bool = false, labels_horBottom_bool = false;
    bool axisName_horTop_bool = false, axisName_horBottom_bool = false;


    // Actual structure
    std::vector<std::string> labels_verLeft;
    std::vector<std::string> axis_verLeft;

    std::vector<std::string> axis_verRight;
    std::vector<std::string> labels_verRight;

    std::string              axisName_horTop;
    std::string              label_horTop;
    std::vector<std::string> axis_horTop;

    std::vector<std::string> axis_horBottom;
    std::string              label_horBottom;
    std::string              axisName_horBottom;

    std::vector<std::string> corner_topLeft;
    std::vector<std::string> corner_bottomLeft;
    std::vector<std::string> corner_bottomRight;
    std::vector<std::string> corner_topRight;

    std::vector<std::string> plotArea;

    std::optional<std::reference_wrapper<const DataStore>> ds_ref;


public:
    // This needs to get called after default construction
    auto build_self(this auto &&self, DesiredPlot const &dp, DataStore const &ds) {

        self.ds_ref = std::ref(ds);

        self.compute_descriptors(dp);

        if (not self.validate_descriptors()) { return false; }

        self.compute_labels_vl(dp, ds);
        self.compute_labels_vr(dp, ds);

        self.compute_axis_vl(dp, ds);
        self.compute_axis_vr(dp, ds);

        self.compute_corner_tl(dp, ds);
        self.compute_corner_bl(dp, ds);
        self.compute_corner_br(dp, ds);
        self.compute_corner_tr(dp, ds);

        self.compute_axis_ht(dp, ds);
        self.compute_axisName_ht(dp, ds);
        self.compute_axisLabels_ht(dp, ds);

        self.compute_axis_hb(dp, ds);
        self.compute_axisName_hb(dp, ds);
        self.compute_axisLabels_hb(dp, ds);

        self.compute_plot_area(dp, ds);

        self.ds_ref.reset();

        return true;
    }

    bool validate_self() const { return true; }

    size_t compute_lengthOfSelf() const {

        size_t lngth = pad_top + pad_bottom;

        // Horizontal top axis name and axis labels lines
        lngth += axisName_horTop_bool ? (axisName_horTop.size() + corner_topLeft.front().size() +
                                         corner_topRight.front().size() + pad_left + pad_right)
                                      : 0;
        lngth += labels_horTop_bool ? (label_horTop.size() + corner_topLeft.front().size() +
                                       corner_topRight.front().size() + pad_left + pad_right)
                                    : 0;

        // First and last vertical labels
        lngth += labels_verLeft.front().size() + labels_verRight.front().size() + labels_verLeft.back().size() +
                 labels_verRight.back().size();

        // The 'corners'
        lngth += sizeof("┌") + sizeof("┐") + sizeof("└") + sizeof("┘") - 4;

        // All top and bottom axes
        for (int i = 0; i < areaWidth; i++) { lngth += (axis_horTop.at(i).size() + axis_horBottom.at(i).size()); }

        // Main plot area
        for (int i = 0; i < areaHeight; ++i) {
            lngth += labels_verLeft.at(i + 1).size() + labels_verRight.at(i + 1).size();
            lngth += axis_verLeft.at(i).size();
            lngth += axis_verRight.at(i).size();
            lngth += plotArea.at(i).size();
        }

        // Horizontal bottom axis name and axis labels lines
        lngth += labels_horBottom_bool ? (label_horBottom.size() + corner_bottomLeft.front().size() +
                                          corner_bottomRight.front().size() + pad_left + pad_right)
                                       : 0;
        lngth += axisName_horBottom_bool ? (axisName_horBottom.size() + corner_bottomLeft.front().size() +
                                            corner_bottomRight.front().size() + pad_left + pad_right)
                                         : 0;
        return lngth;
    }

    std::string build_plotAsString() const {
        std::string result;
        result.reserve(compute_lengthOfSelf());

        // Add padding on top
        for (int i = 0; i < pad_top; ++i) { result.push_back('\n'); }

        // Build the heading lines of the plot
        if (axisName_horTop_bool) {
            result.append(std::string(pad_left, ' '));
            result.append(corner_topLeft.at(0));
            result.append(label_horTop);
            result.append(corner_topRight.at(0));
            result.append(std::string(pad_right, ' '));
            result.push_back('\n');
        }
        if (labels_horTop_bool) {
            result.append(std::string(pad_left, ' '));
            result.append(corner_topLeft.at(1));
            result.append(label_horTop);
            result.append(corner_topRight.at(1));
            result.append(std::string(pad_right, ' '));
            result.push_back('\n');
        }

        // Build horizontal top axis line
        result.append(std::string(pad_left, ' '));
        result.append(labels_verLeft.front());
        result.append("┌");
        for (auto const &toAppend : axis_horTop) { result.append(toAppend); }
        result.append("┐");
        result.append(labels_verRight.front());
        result.append(std::string(pad_right, ' '));
        result.push_back('\n');

        // Add plot area lines
        for (size_t i = 0; i < areaHeight; ++i) {
            result.append(std::string(pad_left, ' '));
            result.append(labels_verLeft.at(i + 1));
            result.append(axis_verLeft.at(i));
            result.append(plotArea.at(i));
            result.append(axis_verRight.at(i));
            result.append(labels_verRight.at(i + 1));
            result.append(std::string(pad_right, ' '));
            result.push_back('\n');
        }

        // Add horizontal bottom axis line
        result.append(std::string(pad_left, ' '));
        result.append(labels_verLeft.back());
        result.append("└");
        for (auto const &toAppend : axis_horBottom) { result.append(toAppend); }
        result.append("┘");
        result.append(labels_verRight.back());
        result.append(std::string(pad_right, ' '));
        result.push_back('\n');

        // Add the bottom lines of the plot
        if (labels_horBottom_bool) {
            result.append(std::string(pad_left, ' '));
            result.append(corner_bottomLeft.at(0));
            result.append(label_horBottom);
            result.append(corner_bottomRight.at(0));
            result.append(std::string(pad_right, ' '));
            result.push_back('\n');
        }
        if (axisName_horBottom_bool) {
            result.append(std::string(pad_left, ' '));
            result.append(corner_bottomLeft.at(1));
            result.append(axisName_horBottom);
            result.append(corner_bottomRight.at(1));
            result.append(std::string(pad_right, ' '));
            result.push_back('\n');
        }

        // Add padding on bottom
        for (int i = 0; i < pad_bottom; ++i) { result.push_back('\n'); }
        return result;
    }

private:
    // TODO: Implement validate_descriptors for 'plot_structures'
    bool validate_descriptors() { return true; }

    // One needs to define all of these in a derived class.
    void compute_descriptors(DesiredPlot const &dp) = delete;

    void compute_labels_vl(DesiredPlot const &dp, DataStore const &ds) = delete;
    void compute_labels_vr(DesiredPlot const &dp, DataStore const &ds) = delete;

    void compute_axis_vl(DesiredPlot const &dp, DataStore const &ds) = delete;
    void compute_axis_vr(DesiredPlot const &dp, DataStore const &ds) = delete;

    void compute_corner_tl(DesiredPlot const &dp, DataStore const &ds) = delete;
    void compute_corner_bl(DesiredPlot const &dp, DataStore const &ds) = delete;
    void compute_corner_br(DesiredPlot const &dp, DataStore const &ds) = delete;
    void compute_corner_tr(DesiredPlot const &dp, DataStore const &ds) = delete;

    void compute_axis_ht(DesiredPlot const &dp, DataStore const &ds)       = delete;
    void compute_axisName_ht(DesiredPlot const &dp, DataStore const &ds)   = delete;
    void compute_axisLabels_ht(DesiredPlot const &dp, DataStore const &ds) = delete;

    void compute_axis_hb(DesiredPlot const &dp, DataStore const &ds)       = delete;
    void compute_axisName_hb(DesiredPlot const &dp, DataStore const &ds)   = delete;
    void compute_axisLabels_hb(DesiredPlot const &dp, DataStore const &ds) = delete;

    void compute_plot_area(DesiredPlot const &dp, DataStore const &ds) = delete;
};

class BarV : public Base {
    friend class Base;

    void compute_descriptors(DesiredPlot const &dp) {
        auto &ds = ds_ref.value().get();
        // Vertical left labels
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarV>()) {
            auto const &labelColRef = ds.stringCols.at(ds.colTypes.at(dp.label_colID.value()).second);
            auto const  labelSizes =
                std::views::transform(labelColRef, [](auto const &a) { return detail::strlen_utf8(a); });

            // TODO: Convert the 'hard limit' into some sort of constexpr config thing
            labels_verLeftWidth = std::min(
                30uz, std::min(std::ranges::max(labelSizes), (dp.targetWidth.value() - pad_left - pad_right) / 4));
        }
        // TODO: Computation for numeric labels
        else {}

        // TODO: Vertical right labels ... probably nothing so keeping 0 size
        // ...

        // Plot area width (-2 is for the 2 vertical axes positions)
        areaWidth = dp.targetWidth.value() - pad_left - labels_verLeftWidth - 2 - labels_verRightWidth - pad_right;

        // Labels and axis name bottom
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarH>() ||
            (dp.plot_type_name == detail::TypeToString<plot_structures::Line>() ||
             dp.plot_type_name == detail::TypeToString<plot_structures::Multiline>()) &&
                true) {} // TODO: Proper assessment for Line and ML
        else {
            labels_horBottom_bool = true;
            axisName_horBottom    = true;
        }

        // Labels and axis name top ... probably nothing so keeping 0 size
        // ...

        // Plot area height (-2 is for the 2 horizontal axes positions)
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarV>()) {
            areaHeight = ds.stringCols.at(ds.colTypes.at(dp.label_colID.value()).second).size();
        }
        else {
            areaHeight = dp.targetHeight.value() - pad_top - axisName_horTop_bool - labels_horTop_bool - 2 -
                         labels_horBottom_bool - axisName_horBottom_bool - pad_bottom;
        }

        // Axes steps
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarV>()) { axis_verLeftSteps = areaHeight; }
        else {} // TODO: Computation for numeric labels


        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarH>()) { axis_horBottomSteps = areaWidth; }
        else {} // TODO: Computation for numeric labels

        // Top and Right axes steps keeping as-is
    }

    void compute_labels_vl(DesiredPlot const &dp, DataStore const &ds) {
        auto const &labelsRef = ds.stringCols.at(ds.colTypes.at(dp.label_colID.value()).second);
        labels_verLeft.push_back(std::string(labels_verLeftWidth, ' '));
        for (auto const &rawLabel : labelsRef) {
            labels_verLeft.push_back(detail::middleTrim2Size(rawLabel, labels_verLeftWidth));
        }
        labels_verLeft.push_back(std::string(labels_verLeftWidth, ' '));
    }
    void compute_labels_vr(DesiredPlot const &dp, DataStore const &ds) {

        labels_verRight.push_back("");
        for (auto const &_ : ds.stringCols.at(ds.colTypes.at(dp.label_colID.value()).second)) {
            // TODO: Logic for vr labels
            labels_verRight.push_back("");
        }
        labels_verRight.push_back("");
    }

    void compute_axis_vl(DesiredPlot const &dp, DataStore const &ds) {
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarV>()) {
            axis_verLeft = detail::create_tickMarkedAxis("│", "┤", areaHeight, areaHeight);
        }
        // All else should have vl axis ticks according to numeric values
        else {}
    }
    void compute_axis_vr(DesiredPlot const &dp, DataStore const &ds) {
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarV>()) {
            axis_verRight = std::vector(areaHeight, std::string(" "));
        }
    }

    // All corners are simply empty as default ... but can possibly be used for something later if overrided in derived
    void compute_corner_tl(DesiredPlot const &dp, DataStore const &ds) {
        if (axisName_horTop_bool) { corner_topLeft.push_back(std::string(labels_verLeftWidth, ' ')); }
        if (labels_horTop_bool) { corner_topLeft.push_back(std::string(labels_verLeftWidth, ' ')); }
    }
    void compute_corner_bl(DesiredPlot const &dp, DataStore const &ds) {
        if (axisName_horBottom_bool) { corner_bottomLeft.push_back(std::string(labels_verLeftWidth, ' ')); }
        if (labels_horBottom_bool) { corner_bottomLeft.push_back(std::string(labels_verLeftWidth, ' ')); }
    }
    void compute_corner_br(DesiredPlot const &dp, DataStore const &ds) {
        if (axisName_horTop_bool) { corner_topRight.push_back(std::string(labels_verRightWidth, ' ')); }
        if (labels_horTop_bool) { corner_topRight.push_back(std::string(labels_verRightWidth, ' ')); }
    }
    void compute_corner_tr(DesiredPlot const &dp, DataStore const &ds) {
        if (axisName_horBottom_bool) { corner_bottomRight.push_back(std::string(labels_verRightWidth, ' ')); }
        if (labels_horBottom_bool) { corner_bottomRight.push_back(std::string(labels_verRightWidth, ' ')); }
    }

    void compute_axis_ht(DesiredPlot const &dp, DataStore const &ds) {
        axis_horTop = std::vector(areaWidth, std::string(" "));
    }
    void compute_axisName_ht(DesiredPlot const &dp, DataStore const &ds) {}
    void compute_axisLabels_ht(DesiredPlot const &dp, DataStore const &ds) {}


    void compute_axis_hb(DesiredPlot const &dp, DataStore const &ds) {
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarH>()) {
            axis_horBottom = detail::create_tickMarkedAxis("─", "┬", areaWidth, areaWidth);
        }
        // All else should be values according to value col #1
        // TODO: Fix this so there are ticks by values ... must make specialized method for that
        else { axis_horBottom = std::vector(areaWidth, std::string(" ")); }
    }

    void compute_axisName_hb(DesiredPlot const &dp, DataStore const &ds) {
        if (dp.plot_type_name == detail::TypeToString<plot_structures::BarH>()) {
            // TODO: What to do with BarHs axisName bottom
        }
        else { axisName_horBottom = ds.colNames.at(dp.values_colIDs.front()); }
    }
    void compute_axisLabels_hb(DesiredPlot const &dp, DataStore const &ds) {}

    void compute_plot_area(DesiredPlot const &dp, DataStore const &ds) {
        auto const &valColTypeRef = ds.colTypes.at(dp.values_colIDs.front());
        if (valColTypeRef.first == nlohmann::detail::value_t::number_float) {
            auto const &valColRef   = ds.doubleCols.at(valColTypeRef.second);
            auto const [minV, maxV] = std::ranges::minmax(valColRef);
            double stepSize         = (maxV - minV) / areaWidth;
            for (auto const &val : valColRef) {
                plotArea.push_back(std::string());
                size_t rpt = static_cast<size_t>((val - minV) / stepSize);
                for (size_t i = rpt; i > 0; --i) { plotArea.back().append("■"); }
                for (size_t i = rpt; i < areaWidth; ++i) { plotArea.back().push_back(' '); }
            }
        }
        else {
            auto const &valColRef   = ds.llCols.at(valColTypeRef.second);
            auto const [minV, maxV] = std::ranges::minmax(valColRef);
            long long scalingFactor = LONG_LONG_MAX / (std::max(std::abs(maxV), std::abs(minV)));
            long long maxV_adj      = maxV * scalingFactor;
            long long minV_adj      = minV * scalingFactor;
            long long stepSize      = (maxV_adj - minV_adj) / areaWidth;

            for (auto const &val : valColRef) {
                plotArea.push_back(std::string());
                long long rpt = (val * scalingFactor - minV_adj) / stepSize;
                for (long long i = rpt; i > 0; --i) { plotArea.back().append("■"); }
                for (long long i = rpt; i < areaWidth; ++i) { plotArea.back().push_back(' '); }
            }
        }
    }
};

class BarH : public BarV {
    friend class Base;

    void compute_axis_ht(DesiredPlot const &dp, DataStore const &ds) { axis_horBottomSteps = areaWidth; }
};

class Line : public BarV {
    friend class Base;
};

class Multiline : public Line {
    friend class Base;
};

class Scatter : public BarV {
    friend class Base;
};

class Bubble : public Scatter {
    friend class Base;
};

} // namespace plot_structures


template <typename PS_VAR>
class PlotDrawer {
private:
    PS_VAR m_ps_var;

public:
    constexpr PlotDrawer() {};
    PlotDrawer(auto ps_var, DesiredPlot const &dp, DataStore const &ds) : m_ps_var(ps_var) {
        auto ol = [&](auto &&var) { var.build_self(dp, ds); };
        std::visit(ol, m_ps_var);
    }

    void update_newPlotStructure(DesiredPlot const &dp, DataStore const &ds) {
        auto ol = [&](auto &&var) {
            m_ps_var = decltype(var)();
            var.build_self(dp, ds);
        };
        std::visit(ol, m_ps_var);
    }

    bool validate_self() const {
        auto validate = [&](auto &&var) -> bool { return var.validate_self(); };
        return std::visit(validate, m_ps_var);
    }

    std::expected<std::string, Err_drawer> validateAndDrawPlot() const {
        // TODO: Add some validation before drawing
        if (validate_self() == false) { return std::unexpected(Err_drawer::plotStructureInvalid); }
        else { return drawPlot(); }
    }

    std::string drawPlot() const {
        auto ol = [&](auto &&var) -> std::string { return var.build_plotAsString(); };
        return std::visit(ol, m_ps_var);
    }
};


template <typename... Ts>
requires(std::is_base_of_v<plot_structures::Base, Ts>, ...) && detail::none_sameLastLevelTypeName<Ts...>
constexpr auto generate_PD_PS_variantTypeMap() {
    std::unordered_map<std::string, std::variant<Ts...>> res;
    (res.insert({detail::TypeToString<Ts>(), std::variant<Ts...>(Ts())}), ...);
    return res;
}

// This is a map of default constructed 'plot_structures' inside an std::variant
// Pass the 'plot_structure' template types that should be used by the library
// This is the only where one 'selects' these template types
static const auto mp_names2Types =
    generate_PD_PS_variantTypeMap<plot_structures::BarV, plot_structures::BarH, plot_structures::Line,
                                  plot_structures::Multiline, plot_structures::Scatter, plot_structures::Bubble>();

inline auto make_plotDrawer(DesiredPlot const &dp, DataStore const &ds) {
    auto ref          = mp_names2Types.at(dp.plot_type_name.value());
    using varType     = decltype(ref);
    auto overload_set = [&]<typename T>(T const &variantItem) -> PlotDrawer<varType> {
        return PlotDrawer<varType>(variantItem, dp, ds);
    };
    return std::visit(overload_set, ref);
}


} // namespace terminal_plot
} // namespace incom

namespace incplot = incom::terminal_plot;