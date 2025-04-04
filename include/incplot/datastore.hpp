#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace incom {
namespace terminal_plot {
// Data storage for the actual data that are to be plotted
struct DataStore {
    using NLMjson = nlohmann::ordered_json;
    // The json this was constucted with/from ... possibly not strictly necessary to keep, but whatever
    std::vector<NLMjson> constructedWith;

    // TODO: Maybe provide my own 'ColType' enum ... consider
    // Data descriptors
    std::vector<std::string>                         colNames;
    std::vector<std::pair<NLMjson::value_t, size_t>> colTypes; // First =  ColType, Second = ID in data vector

    // Actual data storage
    std::vector<std::vector<std::string>> stringCols;
    std::vector<std::vector<long long>>   llCols; // Don't care about signed unsigned, etc. ... all will be long long
    std::vector<std::vector<double>>      doubleCols;

    // DataStore can be accessed using dynamic polymorphism with this vector of varints reference to each collumn in the
    // data storage
    std::vector<
        std::variant<std::reference_wrapper<std::vector<std::string>>, std::reference_wrapper<std::vector<long long>>,
                     std::reference_wrapper<std::vector<double>>>>
        vec_colVariants;


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
        build_vecOfColVariants();
    }

    // APPENDING
    void append_data(std::vector<NLMjson> const &toAppend) {
        // For each json line ...
        for (auto const &oneJson : toAppend) {
            auto oneJsonIT = oneJson.items().begin();
            // ... go across the colTypes and add data to the right vector
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

    void build_vecOfColVariants() {
        for (auto const &colTypesPair : colTypes) {
            if (colTypesPair.first == NLMjson::value_t::string) {
                vec_colVariants.push_back(stringCols[colTypesPair.second]);
            }
            else if (colTypesPair.first == NLMjson::value_t::number_integer ||
                     colTypesPair.first == NLMjson::value_t::number_unsigned) {
                vec_colVariants.push_back(llCols[colTypesPair.second]);
            }
            else if (colTypesPair.first == NLMjson::value_t::number_float) {
                vec_colVariants.push_back(doubleCols[colTypesPair.second]);
            }
        }
    }
};

} // namespace terminal_plot
} // namespace incom