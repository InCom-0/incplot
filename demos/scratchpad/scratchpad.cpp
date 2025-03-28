#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <windows.h>



#include <incplot.hpp>


int main() {
    using json = nlohmann::json;

    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);
    DWORD  type = GetFileType(hIn);
    switch (type) {
        case FILE_TYPE_PIPE: break;
        default:
            std::print("{}", "STD INPUT is not 'pipe' ... exiting");
            return 1;
            break;
    }

    std::string input((std::istreambuf_iterator(std::cin)), std::istreambuf_iterator<char>());

    /*
        std::string testInput(R"({"name":"BioWare","size":750}
    {"name":"CD Projekt Red","size":980}
    {"name":"Cline","size":6500}
    {"name":"DSP_Star_Sector_Resources_61571387-64.csv","size":7558}
    {"name":"Dyson Sphere Program","size":4096}
    {"name":"Filmy","size":4500}
    {"name":"Hudba","size":8500}
    {"name":"My Games","size":1500}
    {"name":"Obrázky","size":1800}
    {"name":"OpenIV","size":2600}
    {"name":"PowerToys","size":1000}
    {"name":"Project CARS 2","size":4096}
    {"name":"Respawn","size":300}
    {"name":"Rockstar Games","size":50}
    {"name":"The Riftbreaker","size":4096}
    {"name":"VTune","size":0})");


        std::string testInput_petal(R"({"sepal_length":5.1,"petal_length":1.4}
    {"sepal_length":4.9,"petal_length":1.4}
    {"sepal_length":4.7,"petal_length":1.3}
    {"sepal_length":4.6,"petal_length":1.5}
    {"sepal_length":5,"petal_length":1.4}
    {"sepal_length":5.4,"petal_length":1.7}
    {"sepal_length":4.6,"petal_length":1.4}
    {"sepal_length":5,"petal_length":1.5}
    {"sepal_length":4.4,"petal_length":1.4}
    {"sepal_length":4.9,"petal_length":1.5}
    {"sepal_length":5.4,"petal_length":1.5}
    {"sepal_length":4.8,"petal_length":1.6}
    {"sepal_length":4.8,"petal_length":1.4}
    {"sepal_length":4.3,"petal_length":1.1}
    {"sepal_length":5.8,"petal_length":1.2}
    {"sepal_length":5.7,"petal_length":1.5}
    {"sepal_length":5.4,"petal_length":1.3}
    {"sepal_length":5.1,"petal_length":1.4}
    {"sepal_length":5.7,"petal_length":1.7}
    {"sepal_length":5.1,"petal_length":1.5}
    {"sepal_length":5.4,"petal_length":1.7}
    {"sepal_length":5.1,"petal_length":1.5}
    {"sepal_length":4.6,"petal_length":1}
    {"sepal_length":5.1,"petal_length":1.7}
    {"sepal_length":4.8,"petal_length":1.9}
    {"sepal_length":5,"petal_length":1.6}
    {"sepal_length":5,"petal_length":1.6}
    {"sepal_length":5.2,"petal_length":1.5}
    {"sepal_length":5.2,"petal_length":1.4}
    {"sepal_length":4.7,"petal_length":1.6}
    {"sepal_length":4.8,"petal_length":1.6}
    {"sepal_length":5.4,"petal_length":1.5}
    {"sepal_length":5.2,"petal_length":1.5}
    {"sepal_length":5.5,"petal_length":1.4}
    {"sepal_length":4.9,"petal_length":1.5}
    {"sepal_length":5,"petal_length":1.2}
    {"sepal_length":5.5,"petal_length":1.3}
    {"sepal_length":4.9,"petal_length":1.5}
    {"sepal_length":4.4,"petal_length":1.3}
    {"sepal_length":5.1,"petal_length":1.5}
    {"sepal_length":5,"petal_length":1.3}
    {"sepal_length":4.5,"petal_length":1.3}
    {"sepal_length":4.4,"petal_length":1.3}
    {"sepal_length":5,"petal_length":1.6}
    {"sepal_length":5.1,"petal_length":1.9}
    {"sepal_length":4.8,"petal_length":1.4}
    {"sepal_length":5.1,"petal_length":1.6}
    {"sepal_length":4.6,"petal_length":1.4}
    {"sepal_length":5.3,"petal_length":1.5}
    {"sepal_length":5,"petal_length":1.4}
    {"sepal_length":7,"petal_length":4.7}
    {"sepal_length":6.4,"petal_length":4.5}
    {"sepal_length":6.9,"petal_length":4.9}
    {"sepal_length":5.5,"petal_length":4}
    {"sepal_length":6.5,"petal_length":4.6}
    {"sepal_length":5.7,"petal_length":4.5}
    {"sepal_length":6.3,"petal_length":4.7}
    {"sepal_length":4.9,"petal_length":3.3}
    {"sepal_length":6.6,"petal_length":4.6}
    {"sepal_length":5.2,"petal_length":3.9}
    {"sepal_length":5,"petal_length":3.5}
    {"sepal_length":5.9,"petal_length":4.2}
    {"sepal_length":6,"petal_length":4}
    {"sepal_length":6.1,"petal_length":4.7}
    {"sepal_length":5.6,"petal_length":3.6}
    {"sepal_length":6.7,"petal_length":4.4}
    {"sepal_length":5.6,"petal_length":4.5}
    {"sepal_length":5.8,"petal_length":4.1}
    {"sepal_length":6.2,"petal_length":4.5}
    {"sepal_length":5.6,"petal_length":3.9}
    {"sepal_length":5.9,"petal_length":4.8}
    {"sepal_length":6.1,"petal_length":4}
    {"sepal_length":6.3,"petal_length":4.9}
    {"sepal_length":6.1,"petal_length":4.7}
    {"sepal_length":6.4,"petal_length":4.3}
    {"sepal_length":6.6,"petal_length":4.4}
    {"sepal_length":6.8,"petal_length":4.8}
    {"sepal_length":6.7,"petal_length":5}
    {"sepal_length":6,"petal_length":4.5}
    {"sepal_length":5.7,"petal_length":3.5}
    {"sepal_length":5.5,"petal_length":3.8}
    {"sepal_length":5.5,"petal_length":3.7}
    {"sepal_length":5.8,"petal_length":3.9}
    {"sepal_length":6,"petal_length":5.1}
    {"sepal_length":5.4,"petal_length":4.5}
    {"sepal_length":6,"petal_length":4.5}
    {"sepal_length":6.7,"petal_length":4.7}
    {"sepal_length":6.3,"petal_length":4.4}
    {"sepal_length":5.6,"petal_length":4.1}
    {"sepal_length":5.5,"petal_length":4}
    {"sepal_length":5.5,"petal_length":4.4}
    {"sepal_length":6.1,"petal_length":4.6}
    {"sepal_length":5.8,"petal_length":4}
    {"sepal_length":5,"petal_length":3.3}
    {"sepal_length":5.6,"petal_length":4.2}
    {"sepal_length":5.7,"petal_length":4.2}
    {"sepal_length":5.7,"petal_length":4.2}
    {"sepal_length":6.2,"petal_length":4.3}
    {"sepal_length":5.1,"petal_length":3}
    {"sepal_length":5.7,"petal_length":4.1}
    {"sepal_length":6.3,"petal_length":6}
    {"sepal_length":5.8,"petal_length":5.1}
    {"sepal_length":7.1,"petal_length":5.9}
    {"sepal_length":6.3,"petal_length":5.6}
    {"sepal_length":6.5,"petal_length":5.8}
    {"sepal_length":7.6,"petal_length":6.6}
    {"sepal_length":4.9,"petal_length":4.5}
    {"sepal_length":7.3,"petal_length":6.3}
    {"sepal_length":6.7,"petal_length":5.8}
    {"sepal_length":7.2,"petal_length":6.1}
    {"sepal_length":6.5,"petal_length":5.1}
    {"sepal_length":6.4,"petal_length":5.3}
    {"sepal_length":6.8,"petal_length":5.5}
    {"sepal_length":5.7,"petal_length":5}
    {"sepal_length":5.8,"petal_length":5.1}
    {"sepal_length":6.4,"petal_length":5.3}
    {"sepal_length":6.5,"petal_length":5.5}
    {"sepal_length":7.7,"petal_length":6.7}
    {"sepal_length":7.7,"petal_length":6.9}
    {"sepal_length":6,"petal_length":5}
    {"sepal_length":6.9,"petal_length":5.7}
    {"sepal_length":5.6,"petal_length":4.9}
    {"sepal_length":7.7,"petal_length":6.7}
    {"sepal_length":6.3,"petal_length":4.9}
    {"sepal_length":6.7,"petal_length":5.7}
    {"sepal_length":7.2,"petal_length":6}
    {"sepal_length":6.2,"petal_length":4.8}
    {"sepal_length":6.1,"petal_length":4.9}
    {"sepal_length":6.4,"petal_length":5.6}
    {"sepal_length":7.2,"petal_length":5.8}
    {"sepal_length":7.4,"petal_length":6.1}
    {"sepal_length":7.9,"petal_length":6.4}
    {"sepal_length":6.4,"petal_length":5.6}
    {"sepal_length":6.3,"petal_length":5.1}
    {"sepal_length":6.1,"petal_length":5.6}
    {"sepal_length":7.7,"petal_length":6.1}
    {"sepal_length":6.3,"petal_length":5.6}
    {"sepal_length":6.4,"petal_length":5.5}
    {"sepal_length":6,"petal_length":4.8}
    {"sepal_length":6.9,"petal_length":5.4}
    {"sepal_length":6.7,"petal_length":5.6}
    {"sepal_length":6.9,"petal_length":5.1}
    {"sepal_length":5.8,"petal_length":5.1}
    {"sepal_length":6.8,"petal_length":5.9}
    {"sepal_length":6.7,"petal_length":5.7}
    {"sepal_length":6.7,"petal_length":5.2}
    {"sepal_length":6.3,"petal_length":5}
    {"sepal_length":6.5,"petal_length":5.2}
    {"sepal_length":6.2,"petal_length":5.4}
    {"sepal_length":5.9,"petal_length":5.1})"); */

    auto ds = incplot::Parser::parse_NDJSON_intoDS(input);


    auto dp_autoGuessed = incplot::DesiredPlot(32).guess_missingParams(ds);

    if (not dp_autoGuessed.has_value()) {
        std::print("{0}{1}", "Autoguessing of 'DesiresPlot' parameters failed \n", "Exiting ...");
        return 1;
    }

    auto plotDrawer2 = incplot::make_plotDrawer(dp_autoGuessed.value(), ds);

    auto outExp = plotDrawer2.validateAndDrawPlot();

    if (not outExp.has_value()) {
        std::print("{0}{1}", "Invalid plot structure", "Exiting ...");
        return 1;
    }

    std::print("{}\n", outExp.value());
    

    return 0;
}