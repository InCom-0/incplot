#include <print>
#include <string>
#include <windows.h>


#include <incplot.hpp>


int main() {

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

    std::string testInput_petal_OLD(
        R"({"sepal_length":5.1,"sepal_width":3.5,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.9,"sepal_width":3,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.7,"sepal_width":3.2,"petal_length":1.3,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.6,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.6,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.4,"sepal_width":3.9,"petal_length":1.7,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":4.6,"sepal_width":3.4,"petal_length":1.4,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.4,"petal_length":1.5,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.4,"sepal_width":2.9,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.9,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.1,"species":"Iris-setosa"}
{"sepal_length":5.4,"sepal_width":3.7,"petal_length":1.5,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.8,"sepal_width":3.4,"petal_length":1.6,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.8,"sepal_width":3,"petal_length":1.4,"petal_width":0.1,"species":"Iris-setosa"}
{"sepal_length":4.3,"sepal_width":3,"petal_length":1.1,"petal_width":0.1,"species":"Iris-setosa"}
{"sepal_length":5.8,"sepal_width":4,"petal_length":1.2,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.7,"sepal_width":4.4,"petal_length":1.5,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":5.4,"sepal_width":3.9,"petal_length":1.3,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.5,"petal_length":1.4,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":5.7,"sepal_width":3.8,"petal_length":1.7,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.8,"petal_length":1.5,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":5.4,"sepal_width":3.4,"petal_length":1.7,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.7,"petal_length":1.5,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":4.6,"sepal_width":3.6,"petal_length":1,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.3,"petal_length":1.7,"petal_width":0.5,"species":"Iris-setosa"}
{"sepal_length":4.8,"sepal_width":3.4,"petal_length":1.9,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3,"petal_length":1.6,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.4,"petal_length":1.6,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":5.2,"sepal_width":3.5,"petal_length":1.5,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.2,"sepal_width":3.4,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.7,"sepal_width":3.2,"petal_length":1.6,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.8,"sepal_width":3.1,"petal_length":1.6,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.4,"sepal_width":3.4,"petal_length":1.5,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":5.2,"sepal_width":4.1,"petal_length":1.5,"petal_width":0.1,"species":"Iris-setosa"}
{"sepal_length":5.5,"sepal_width":4.2,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.9,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.1,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.2,"petal_length":1.2,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.5,"sepal_width":3.5,"petal_length":1.3,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.9,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.1,"species":"Iris-setosa"}
{"sepal_length":4.4,"sepal_width":3,"petal_length":1.3,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.4,"petal_length":1.5,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.5,"petal_length":1.3,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":4.5,"sepal_width":2.3,"petal_length":1.3,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":4.4,"sepal_width":3.2,"petal_length":1.3,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.5,"petal_length":1.6,"petal_width":0.6,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.8,"petal_length":1.9,"petal_width":0.4,"species":"Iris-setosa"}
{"sepal_length":4.8,"sepal_width":3,"petal_length":1.4,"petal_width":0.3,"species":"Iris-setosa"}
{"sepal_length":5.1,"sepal_width":3.8,"petal_length":1.6,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":4.6,"sepal_width":3.2,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5.3,"sepal_width":3.7,"petal_length":1.5,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":5,"sepal_width":3.3,"petal_length":1.4,"petal_width":0.2,"species":"Iris-setosa"}
{"sepal_length":7,"sepal_width":3.2,"petal_length":4.7,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":6.4,"sepal_width":3.2,"petal_length":4.5,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":6.9,"sepal_width":3.1,"petal_length":4.9,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":5.5,"sepal_width":2.3,"petal_length":4,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.5,"sepal_width":2.8,"petal_length":4.6,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":5.7,"sepal_width":2.8,"petal_length":4.5,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.3,"sepal_width":3.3,"petal_length":4.7,"petal_width":1.6,"species":"Iris-versicolor"}
{"sepal_length":4.9,"sepal_width":2.4,"petal_length":3.3,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":6.6,"sepal_width":2.9,"petal_length":4.6,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":5.2,"sepal_width":2.7,"petal_length":3.9,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":5,"sepal_width":2,"petal_length":3.5,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":5.9,"sepal_width":3,"petal_length":4.2,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":6,"sepal_width":2.2,"petal_length":4,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":6.1,"sepal_width":2.9,"petal_length":4.7,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":5.6,"sepal_width":2.9,"petal_length":3.6,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.7,"sepal_width":3.1,"petal_length":4.4,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":5.6,"sepal_width":3,"petal_length":4.5,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":4.1,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":6.2,"sepal_width":2.2,"petal_length":4.5,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":5.6,"sepal_width":2.5,"petal_length":3.9,"petal_width":1.1,"species":"Iris-versicolor"}
{"sepal_length":5.9,"sepal_width":3.2,"petal_length":4.8,"petal_width":1.8,"species":"Iris-versicolor"}
{"sepal_length":6.1,"sepal_width":2.8,"petal_length":4,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.3,"sepal_width":2.5,"petal_length":4.9,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":6.1,"sepal_width":2.8,"petal_length":4.7,"petal_width":1.2,"species":"Iris-versicolor"}
{"sepal_length":6.4,"sepal_width":2.9,"petal_length":4.3,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.6,"sepal_width":3,"petal_length":4.4,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":6.8,"sepal_width":2.8,"petal_length":4.8,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":6.7,"sepal_width":3,"petal_length":5,"petal_width":1.7,"species":"Iris-versicolor"}
{"sepal_length":6,"sepal_width":2.9,"petal_length":4.5,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":5.7,"sepal_width":2.6,"petal_length":3.5,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":5.5,"sepal_width":2.4,"petal_length":3.8,"petal_width":1.1,"species":"Iris-versicolor"}
{"sepal_length":5.5,"sepal_width":2.4,"petal_length":3.7,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":3.9,"petal_width":1.2,"species":"Iris-versicolor"}
{"sepal_length":6,"sepal_width":2.7,"petal_length":5.1,"petal_width":1.6,"species":"Iris-versicolor"}
{"sepal_length":5.4,"sepal_width":3,"petal_length":4.5,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":6,"sepal_width":3.4,"petal_length":4.5,"petal_width":1.6,"species":"Iris-versicolor"}
{"sepal_length":6.7,"sepal_width":3.1,"petal_length":4.7,"petal_width":1.5,"species":"Iris-versicolor"}
{"sepal_length":6.3,"sepal_width":2.3,"petal_length":4.4,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":5.6,"sepal_width":3,"petal_length":4.1,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":5.5,"sepal_width":2.5,"petal_length":4,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":5.5,"sepal_width":2.6,"petal_length":4.4,"petal_width":1.2,"species":"Iris-versicolor"}
{"sepal_length":6.1,"sepal_width":3,"petal_length":4.6,"petal_width":1.4,"species":"Iris-versicolor"}
{"sepal_length":5.8,"sepal_width":2.6,"petal_length":4,"petal_width":1.2,"species":"Iris-versicolor"}
{"sepal_length":5,"sepal_width":2.3,"petal_length":3.3,"petal_width":1,"species":"Iris-versicolor"}
{"sepal_length":5.6,"sepal_width":2.7,"petal_length":4.2,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":5.7,"sepal_width":3,"petal_length":4.2,"petal_width":1.2,"species":"Iris-versicolor"}
{"sepal_length":5.7,"sepal_width":2.9,"petal_length":4.2,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.2,"sepal_width":2.9,"petal_length":4.3,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":5.1,"sepal_width":2.5,"petal_length":3,"petal_width":1.1,"species":"Iris-versicolor"}
{"sepal_length":5.7,"sepal_width":2.8,"petal_length":4.1,"petal_width":1.3,"species":"Iris-versicolor"}
{"sepal_length":6.3,"sepal_width":3.3,"petal_length":6,"petal_width":2.5,"species":"Iris-virginica"}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":5.1,"petal_width":1.9,"species":"Iris-virginica"}
{"sepal_length":7.1,"sepal_width":3,"petal_length":5.9,"petal_width":2.1,"species":"Iris-virginica"}
{"sepal_length":6.3,"sepal_width":2.9,"petal_length":5.6,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.5,"sepal_width":3,"petal_length":5.8,"petal_width":2.2,"species":"Iris-virginica"}
{"sepal_length":7.6,"sepal_width":3,"petal_length":6.6,"petal_width":2.1,"species":"Iris-virginica"}
{"sepal_length":4.9,"sepal_width":2.5,"petal_length":4.5,"petal_width":1.7,"species":"Iris-virginica"}
{"sepal_length":7.3,"sepal_width":2.9,"petal_length":6.3,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.7,"sepal_width":2.5,"petal_length":5.8,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":7.2,"sepal_width":3.6,"petal_length":6.1,"petal_width":2.5,"species":"Iris-virginica"}
{"sepal_length":6.5,"sepal_width":3.2,"petal_length":5.1,"petal_width":2,"species":"Iris-virginica"}
{"sepal_length":6.4,"sepal_width":2.7,"petal_length":5.3,"petal_width":1.9,"species":"Iris-virginica"}
{"sepal_length":6.8,"sepal_width":3,"petal_length":5.5,"petal_width":2.1,"species":"Iris-virginica"}
{"sepal_length":5.7,"sepal_width":2.5,"petal_length":5,"petal_width":2,"species":"Iris-virginica"}
{"sepal_length":5.8,"sepal_width":2.8,"petal_length":5.1,"petal_width":2.4,"species":"Iris-virginica"}
{"sepal_length":6.4,"sepal_width":3.2,"petal_length":5.3,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":6.5,"sepal_width":3,"petal_length":5.5,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":7.7,"sepal_width":3.8,"petal_length":6.7,"petal_width":2.2,"species":"Iris-virginica"}
{"sepal_length":7.7,"sepal_width":2.6,"petal_length":6.9,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":6,"sepal_width":2.2,"petal_length":5,"petal_width":1.5,"species":"Iris-virginica"}
{"sepal_length":6.9,"sepal_width":3.2,"petal_length":5.7,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":5.6,"sepal_width":2.8,"petal_length":4.9,"petal_width":2,"species":"Iris-virginica"}
{"sepal_length":7.7,"sepal_width":2.8,"petal_length":6.7,"petal_width":2,"species":"Iris-virginica"}
{"sepal_length":6.3,"sepal_width":2.7,"petal_length":4.9,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.7,"sepal_width":3.3,"petal_length":5.7,"petal_width":2.1,"species":"Iris-virginica"}
{"sepal_length":7.2,"sepal_width":3.2,"petal_length":6,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.2,"sepal_width":2.8,"petal_length":4.8,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.1,"sepal_width":3,"petal_length":4.9,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.4,"sepal_width":2.8,"petal_length":5.6,"petal_width":2.1,"species":"Iris-virginica"}
{"sepal_length":7.2,"sepal_width":3,"petal_length":5.8,"petal_width":1.6,"species":"Iris-virginica"}
{"sepal_length":7.4,"sepal_width":2.8,"petal_length":6.1,"petal_width":1.9,"species":"Iris-virginica"}
{"sepal_length":7.9,"sepal_width":3.8,"petal_length":6.4,"petal_width":2,"species":"Iris-virginica"}
{"sepal_length":6.4,"sepal_width":2.8,"petal_length":5.6,"petal_width":2.2,"species":"Iris-virginica"}
{"sepal_length":6.3,"sepal_width":2.8,"petal_length":5.1,"petal_width":1.5,"species":"Iris-virginica"}
{"sepal_length":6.1,"sepal_width":2.6,"petal_length":5.6,"petal_width":1.4,"species":"Iris-virginica"}
{"sepal_length":7.7,"sepal_width":3,"petal_length":6.1,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":6.3,"sepal_width":3.4,"petal_length":5.6,"petal_width":2.4,"species":"Iris-virginica"}
{"sepal_length":6.4,"sepal_width":3.1,"petal_length":5.5,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6,"sepal_width":3,"petal_length":4.8,"petal_width":1.8,"species":"Iris-virginica"}
{"sepal_length":6.9,"sepal_width":3.1,"petal_length":5.4,"petal_width":2.1,"species":"Iris-virginica"}
{"sepal_length":6.7,"sepal_width":3.1,"petal_length":5.6,"petal_width":2.4,"species":"Iris-virginica"}
{"sepal_length":6.9,"sepal_width":3.1,"petal_length":5.1,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":5.1,"petal_width":1.9,"species":"Iris-virginica"}
{"sepal_length":6.8,"sepal_width":3.2,"petal_length":5.9,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":6.7,"sepal_width":3.3,"petal_length":5.7,"petal_width":2.5,"species":"Iris-virginica"}
{"sepal_length":6.7,"sepal_width":3,"petal_length":5.2,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":6.3,"sepal_width":2.5,"petal_length":5,"petal_width":1.9,"species":"Iris-virginica"}
{"sepal_length":6.5,"sepal_width":3,"petal_length":5.2,"petal_width":2,"species":"Iris-virginica"}
{"sepal_length":6.2,"sepal_width":3.4,"petal_length":5.4,"petal_width":2.3,"species":"Iris-virginica"}
{"sepal_length":5.9,"sepal_width":3,"petal_length":5.1,"petal_width":1.8,"species":"Iris-virginica"})");


    std::string testInput_petal(
        R"({"sepal_length":5.1,"sepal_width":3.5,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":4.9,"sepal_width":3,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":4.7,"sepal_width":3.2,"petal_length":1.3,"petal_width":0.2}
{"sepal_length":4.6,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.2}
{"sepal_length":5,"sepal_width":3.6,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":5.4,"sepal_width":3.9,"petal_length":1.7,"petal_width":0.4}
{"sepal_length":4.6,"sepal_width":3.4,"petal_length":1.4,"petal_width":0.3}
{"sepal_length":5,"sepal_width":3.4,"petal_length":1.5,"petal_width":0.2}
{"sepal_length":4.4,"sepal_width":2.9,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":4.9,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.1}
{"sepal_length":5.4,"sepal_width":3.7,"petal_length":1.5,"petal_width":0.2}
{"sepal_length":4.8,"sepal_width":3.4,"petal_length":1.6,"petal_width":0.2}
{"sepal_length":4.8,"sepal_width":3,"petal_length":1.4,"petal_width":0.1}
{"sepal_length":4.3,"sepal_width":3,"petal_length":1.1,"petal_width":0.1}
{"sepal_length":5.8,"sepal_width":4,"petal_length":1.2,"petal_width":0.2}
{"sepal_length":5.7,"sepal_width":4.4,"petal_length":1.5,"petal_width":0.4}
{"sepal_length":5.4,"sepal_width":3.9,"petal_length":1.3,"petal_width":0.4}
{"sepal_length":5.1,"sepal_width":3.5,"petal_length":1.4,"petal_width":0.3}
{"sepal_length":5.7,"sepal_width":3.8,"petal_length":1.7,"petal_width":0.3}
{"sepal_length":5.1,"sepal_width":3.8,"petal_length":1.5,"petal_width":0.3}
{"sepal_length":5.4,"sepal_width":3.4,"petal_length":1.7,"petal_width":0.2}
{"sepal_length":5.1,"sepal_width":3.7,"petal_length":1.5,"petal_width":0.4}
{"sepal_length":4.6,"sepal_width":3.6,"petal_length":1,"petal_width":0.2}
{"sepal_length":5.1,"sepal_width":3.3,"petal_length":1.7,"petal_width":0.5}
{"sepal_length":4.8,"sepal_width":3.4,"petal_length":1.9,"petal_width":0.2}
{"sepal_length":5,"sepal_width":3,"petal_length":1.6,"petal_width":0.2}
{"sepal_length":5,"sepal_width":3.4,"petal_length":1.6,"petal_width":0.4}
{"sepal_length":5.2,"sepal_width":3.5,"petal_length":1.5,"petal_width":0.2}
{"sepal_length":5.2,"sepal_width":3.4,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":4.7,"sepal_width":3.2,"petal_length":1.6,"petal_width":0.2}
{"sepal_length":4.8,"sepal_width":3.1,"petal_length":1.6,"petal_width":0.2}
{"sepal_length":5.4,"sepal_width":3.4,"petal_length":1.5,"petal_width":0.4}
{"sepal_length":5.2,"sepal_width":4.1,"petal_length":1.5,"petal_width":0.1}
{"sepal_length":5.5,"sepal_width":4.2,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":4.9,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.1}
{"sepal_length":5,"sepal_width":3.2,"petal_length":1.2,"petal_width":0.2}
{"sepal_length":5.5,"sepal_width":3.5,"petal_length":1.3,"petal_width":0.2}
{"sepal_length":4.9,"sepal_width":3.1,"petal_length":1.5,"petal_width":0.1}
{"sepal_length":4.4,"sepal_width":3,"petal_length":1.3,"petal_width":0.2}
{"sepal_length":5.1,"sepal_width":3.4,"petal_length":1.5,"petal_width":0.2}
{"sepal_length":5,"sepal_width":3.5,"petal_length":1.3,"petal_width":0.3}
{"sepal_length":4.5,"sepal_width":2.3,"petal_length":1.3,"petal_width":0.3}
{"sepal_length":4.4,"sepal_width":3.2,"petal_length":1.3,"petal_width":0.2}
{"sepal_length":5,"sepal_width":3.5,"petal_length":1.6,"petal_width":0.6}
{"sepal_length":5.1,"sepal_width":3.8,"petal_length":1.9,"petal_width":0.4}
{"sepal_length":4.8,"sepal_width":3,"petal_length":1.4,"petal_width":0.3}
{"sepal_length":5.1,"sepal_width":3.8,"petal_length":1.6,"petal_width":0.2}
{"sepal_length":4.6,"sepal_width":3.2,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":5.3,"sepal_width":3.7,"petal_length":1.5,"petal_width":0.2}
{"sepal_length":5,"sepal_width":3.3,"petal_length":1.4,"petal_width":0.2}
{"sepal_length":7,"sepal_width":3.2,"petal_length":4.7,"petal_width":1.4}
{"sepal_length":6.4,"sepal_width":3.2,"petal_length":4.5,"petal_width":1.5}
{"sepal_length":6.9,"sepal_width":3.1,"petal_length":4.9,"petal_width":1.5}
{"sepal_length":5.5,"sepal_width":2.3,"petal_length":4,"petal_width":1.3}
{"sepal_length":6.5,"sepal_width":2.8,"petal_length":4.6,"petal_width":1.5}
{"sepal_length":5.7,"sepal_width":2.8,"petal_length":4.5,"petal_width":1.3}
{"sepal_length":6.3,"sepal_width":3.3,"petal_length":4.7,"petal_width":1.6}
{"sepal_length":4.9,"sepal_width":2.4,"petal_length":3.3,"petal_width":1}
{"sepal_length":6.6,"sepal_width":2.9,"petal_length":4.6,"petal_width":1.3}
{"sepal_length":5.2,"sepal_width":2.7,"petal_length":3.9,"petal_width":1.4}
{"sepal_length":5,"sepal_width":2,"petal_length":3.5,"petal_width":1}
{"sepal_length":5.9,"sepal_width":3,"petal_length":4.2,"petal_width":1.5}
{"sepal_length":6,"sepal_width":2.2,"petal_length":4,"petal_width":1}
{"sepal_length":6.1,"sepal_width":2.9,"petal_length":4.7,"petal_width":1.4}
{"sepal_length":5.6,"sepal_width":2.9,"petal_length":3.6,"petal_width":1.3}
{"sepal_length":6.7,"sepal_width":3.1,"petal_length":4.4,"petal_width":1.4}
{"sepal_length":5.6,"sepal_width":3,"petal_length":4.5,"petal_width":1.5}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":4.1,"petal_width":1}
{"sepal_length":6.2,"sepal_width":2.2,"petal_length":4.5,"petal_width":1.5}
{"sepal_length":5.6,"sepal_width":2.5,"petal_length":3.9,"petal_width":1.1}
{"sepal_length":5.9,"sepal_width":3.2,"petal_length":4.8,"petal_width":1.8}
{"sepal_length":6.1,"sepal_width":2.8,"petal_length":4,"petal_width":1.3}
{"sepal_length":6.3,"sepal_width":2.5,"petal_length":4.9,"petal_width":1.5}
{"sepal_length":6.1,"sepal_width":2.8,"petal_length":4.7,"petal_width":1.2}
{"sepal_length":6.4,"sepal_width":2.9,"petal_length":4.3,"petal_width":1.3}
{"sepal_length":6.6,"sepal_width":3,"petal_length":4.4,"petal_width":1.4}
{"sepal_length":6.8,"sepal_width":2.8,"petal_length":4.8,"petal_width":1.4}
{"sepal_length":6.7,"sepal_width":3,"petal_length":5,"petal_width":1.7}
{"sepal_length":6,"sepal_width":2.9,"petal_length":4.5,"petal_width":1.5}
{"sepal_length":5.7,"sepal_width":2.6,"petal_length":3.5,"petal_width":1}
{"sepal_length":5.5,"sepal_width":2.4,"petal_length":3.8,"petal_width":1.1}
{"sepal_length":5.5,"sepal_width":2.4,"petal_length":3.7,"petal_width":1}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":3.9,"petal_width":1.2}
{"sepal_length":6,"sepal_width":2.7,"petal_length":5.1,"petal_width":1.6}
{"sepal_length":5.4,"sepal_width":3,"petal_length":4.5,"petal_width":1.5}
{"sepal_length":6,"sepal_width":3.4,"petal_length":4.5,"petal_width":1.6}
{"sepal_length":6.7,"sepal_width":3.1,"petal_length":4.7,"petal_width":1.5}
{"sepal_length":6.3,"sepal_width":2.3,"petal_length":4.4,"petal_width":1.3}
{"sepal_length":5.6,"sepal_width":3,"petal_length":4.1,"petal_width":1.3}
{"sepal_length":5.5,"sepal_width":2.5,"petal_length":4,"petal_width":1.3}
{"sepal_length":5.5,"sepal_width":2.6,"petal_length":4.4,"petal_width":1.2}
{"sepal_length":6.1,"sepal_width":3,"petal_length":4.6,"petal_width":1.4}
{"sepal_length":5.8,"sepal_width":2.6,"petal_length":4,"petal_width":1.2}
{"sepal_length":5,"sepal_width":2.3,"petal_length":3.3,"petal_width":1}
{"sepal_length":5.6,"sepal_width":2.7,"petal_length":4.2,"petal_width":1.3}
{"sepal_length":5.7,"sepal_width":3,"petal_length":4.2,"petal_width":1.2}
{"sepal_length":5.7,"sepal_width":2.9,"petal_length":4.2,"petal_width":1.3}
{"sepal_length":6.2,"sepal_width":2.9,"petal_length":4.3,"petal_width":1.3}
{"sepal_length":5.1,"sepal_width":2.5,"petal_length":3,"petal_width":1.1}
{"sepal_length":5.7,"sepal_width":2.8,"petal_length":4.1,"petal_width":1.3}
{"sepal_length":6.3,"sepal_width":3.3,"petal_length":6,"petal_width":2.5}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":5.1,"petal_width":1.9}
{"sepal_length":7.1,"sepal_width":3,"petal_length":5.9,"petal_width":2.1}
{"sepal_length":6.3,"sepal_width":2.9,"petal_length":5.6,"petal_width":1.8}
{"sepal_length":6.5,"sepal_width":3,"petal_length":5.8,"petal_width":2.2}
{"sepal_length":7.6,"sepal_width":3,"petal_length":6.6,"petal_width":2.1}
{"sepal_length":4.9,"sepal_width":2.5,"petal_length":4.5,"petal_width":1.7}
{"sepal_length":7.3,"sepal_width":2.9,"petal_length":6.3,"petal_width":1.8}
{"sepal_length":6.7,"sepal_width":2.5,"petal_length":5.8,"petal_width":1.8}
{"sepal_length":7.2,"sepal_width":3.6,"petal_length":6.1,"petal_width":2.5}
{"sepal_length":6.5,"sepal_width":3.2,"petal_length":5.1,"petal_width":2}
{"sepal_length":6.4,"sepal_width":2.7,"petal_length":5.3,"petal_width":1.9}
{"sepal_length":6.8,"sepal_width":3,"petal_length":5.5,"petal_width":2.1}
{"sepal_length":5.7,"sepal_width":2.5,"petal_length":5,"petal_width":2}
{"sepal_length":5.8,"sepal_width":2.8,"petal_length":5.1,"petal_width":2.4}
{"sepal_length":6.4,"sepal_width":3.2,"petal_length":5.3,"petal_width":2.3}
{"sepal_length":6.5,"sepal_width":3,"petal_length":5.5,"petal_width":1.8}
{"sepal_length":7.7,"sepal_width":3.8,"petal_length":6.7,"petal_width":2.2}
{"sepal_length":7.7,"sepal_width":2.6,"petal_length":6.9,"petal_width":2.3}
{"sepal_length":6,"sepal_width":2.2,"petal_length":5,"petal_width":1.5}
{"sepal_length":6.9,"sepal_width":3.2,"petal_length":5.7,"petal_width":2.3}
{"sepal_length":5.6,"sepal_width":2.8,"petal_length":4.9,"petal_width":2}
{"sepal_length":7.7,"sepal_width":2.8,"petal_length":6.7,"petal_width":2}
{"sepal_length":6.3,"sepal_width":2.7,"petal_length":4.9,"petal_width":1.8}
{"sepal_length":6.7,"sepal_width":3.3,"petal_length":5.7,"petal_width":2.1}
{"sepal_length":7.2,"sepal_width":3.2,"petal_length":6,"petal_width":1.8}
{"sepal_length":6.2,"sepal_width":2.8,"petal_length":4.8,"petal_width":1.8}
{"sepal_length":6.1,"sepal_width":3,"petal_length":4.9,"petal_width":1.8}
{"sepal_length":6.4,"sepal_width":2.8,"petal_length":5.6,"petal_width":2.1}
{"sepal_length":7.2,"sepal_width":3,"petal_length":5.8,"petal_width":1.6}
{"sepal_length":7.4,"sepal_width":2.8,"petal_length":6.1,"petal_width":1.9}
{"sepal_length":7.9,"sepal_width":3.8,"petal_length":6.4,"petal_width":2}
{"sepal_length":6.4,"sepal_width":2.8,"petal_length":5.6,"petal_width":2.2}
{"sepal_length":6.3,"sepal_width":2.8,"petal_length":5.1,"petal_width":1.5}
{"sepal_length":6.1,"sepal_width":2.6,"petal_length":5.6,"petal_width":1.4}
{"sepal_length":7.7,"sepal_width":3,"petal_length":6.1,"petal_width":2.3}
{"sepal_length":6.3,"sepal_width":3.4,"petal_length":5.6,"petal_width":2.4}
{"sepal_length":6.4,"sepal_width":3.1,"petal_length":5.5,"petal_width":1.8}
{"sepal_length":6,"sepal_width":3,"petal_length":4.8,"petal_width":1.8}
{"sepal_length":6.9,"sepal_width":3.1,"petal_length":5.4,"petal_width":2.1}
{"sepal_length":6.7,"sepal_width":3.1,"petal_length":5.6,"petal_width":2.4}
{"sepal_length":6.9,"sepal_width":3.1,"petal_length":5.1,"petal_width":2.3}
{"sepal_length":5.8,"sepal_width":2.7,"petal_length":5.1,"petal_width":1.9}
{"sepal_length":6.8,"sepal_width":3.2,"petal_length":5.9,"petal_width":2.3}
{"sepal_length":6.7,"sepal_width":3.3,"petal_length":5.7,"petal_width":2.5}
{"sepal_length":6.7,"sepal_width":3,"petal_length":5.2,"petal_width":2.3}
{"sepal_length":6.3,"sepal_width":2.5,"petal_length":5,"petal_width":1.9}
{"sepal_length":6.5,"sepal_width":3,"petal_length":5.2,"petal_width":2}
{"sepal_length":6.2,"sepal_width":3.4,"petal_length":5.4,"petal_width":2.3}
{"sepal_length":5.9,"sepal_width":3,"petal_length":5.1,"petal_width":1.8})");

    std::string testInput_flights(
        R"({"period":1,"value":112}
{"period":2,"value":118}
{"period":3,"value":132}
{"period":4,"value":129}
{"period":5,"value":121}
{"period":6,"value":135}
{"period":7,"value":148}
{"period":8,"value":148}
{"period":9,"value":136}
{"period":10,"value":119}
{"period":11,"value":104}
{"period":12,"value":118}
{"period":13,"value":115}
{"period":14,"value":126}
{"period":15,"value":141}
{"period":16,"value":135}
{"period":17,"value":125}
{"period":18,"value":149}
{"period":19,"value":170}
{"period":20,"value":170}
{"period":21,"value":158}
{"period":22,"value":133}
{"period":23,"value":114}
{"period":24,"value":140}
{"period":25,"value":145}
{"period":26,"value":150}
{"period":27,"value":178}
{"period":28,"value":163}
{"period":29,"value":172}
{"period":30,"value":178}
{"period":31,"value":199}
{"period":32,"value":199}
{"period":33,"value":184}
{"period":34,"value":162}
{"period":35,"value":146}
{"period":36,"value":166}
{"period":37,"value":171}
{"period":38,"value":180}
{"period":39,"value":193}
{"period":40,"value":181}
{"period":41,"value":183}
{"period":42,"value":218}
{"period":43,"value":230}
{"period":44,"value":242}
{"period":45,"value":209}
{"period":46,"value":191}
{"period":47,"value":172}
{"period":48,"value":194}
{"period":49,"value":196}
{"period":50,"value":196}
{"period":51,"value":236}
{"period":52,"value":235}
{"period":53,"value":229}
{"period":54,"value":243}
{"period":55,"value":264}
{"period":56,"value":272}
{"period":57,"value":237}
{"period":58,"value":211}
{"period":59,"value":180}
{"period":60,"value":201}
{"period":61,"value":204}
{"period":62,"value":188}
{"period":63,"value":235}
{"period":64,"value":227}
{"period":65,"value":234}
{"period":66,"value":264}
{"period":67,"value":302}
{"period":68,"value":293}
{"period":69,"value":259}
{"period":70,"value":229}
{"period":71,"value":203}
{"period":72,"value":229}
{"period":73,"value":242}
{"period":74,"value":233}
{"period":75,"value":267}
{"period":76,"value":269}
{"period":77,"value":270}
{"period":78,"value":315}
{"period":79,"value":364}
{"period":80,"value":347}
{"period":81,"value":312}
{"period":82,"value":274}
{"period":83,"value":237}
{"period":84,"value":278}
{"period":85,"value":284}
{"period":86,"value":277}
{"period":87,"value":317}
{"period":88,"value":313}
{"period":89,"value":318}
{"period":90,"value":374}
{"period":91,"value":413}
{"period":92,"value":405}
{"period":93,"value":355}
{"period":94,"value":306}
{"period":95,"value":271}
{"period":96,"value":306}
{"period":97,"value":315}
{"period":98,"value":301}
{"period":99,"value":356}
{"period":100,"value":348}
{"period":101,"value":355}
{"period":102,"value":422}
{"period":103,"value":465}
{"period":104,"value":467}
{"period":105,"value":404}
{"period":106,"value":347}
{"period":107,"value":305}
{"period":108,"value":336}
{"period":109,"value":340}
{"period":110,"value":318}
{"period":111,"value":362}
{"period":112,"value":348}
{"period":113,"value":363}
{"period":114,"value":435}
{"period":115,"value":491}
{"period":116,"value":505}
{"period":117,"value":404}
{"period":118,"value":359}
{"period":119,"value":310}
{"period":120,"value":337}
{"period":121,"value":360}
{"period":122,"value":342}
{"period":123,"value":406}
{"period":124,"value":396}
{"period":125,"value":420}
{"period":126,"value":472}
{"period":127,"value":548}
{"period":128,"value":559}
{"period":129,"value":463}
{"period":130,"value":407}
{"period":131,"value":362}
{"period":132,"value":405}
{"period":133,"value":417}
{"period":134,"value":391}
{"period":135,"value":419}
{"period":136,"value":461}
{"period":137,"value":472}
{"period":138,"value":535}
{"period":139,"value":622}
{"period":140,"value":606}
{"period":141,"value":508}
{"period":142,"value":461}
{"period":143,"value":390}
{"period":144,"value":432})");


/*     auto ds = incplot::Parser::parse_NDJSON_intoDS(testInput);


    auto dp_autoGuessed =
        incplot::DesiredPlot(incplot::DesiredPlot::DP_CtorStruct{.tar_width = 96}).guess_missingParams(ds);

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


    std::print("{}Viridis {}Reset \n", incplot::TermColors::get_basicColor(incplot::Color_CVTS::Foreground_Green),
               incplot::TermColors::get_basicColor(incplot::Color_CVTS::Default));
    std::print("{}Rhodo {}Reset \n", incplot::TermColors::get_basicColor(incplot::Color_CVTS::Foreground_Red),
               incplot::TermColors::get_basicColor(incplot::Color_CVTS::Default));
    std::print("{}Tanza {}Reset \n", incplot::TermColors::get_basicColor(incplot::Color_CVTS::Foreground_Blue),
               incplot::TermColors::get_basicColor(incplot::Color_CVTS::Default));


    // std::print("{0}{1}\n", "\x1b[38;2;125;125;0m", "TESTEST");


    auto ds2 = incplot::Parser::parse_NDJSON_intoDS(testInput_petal);

    auto dp2_autoGuessed =
        incplot::DesiredPlot(incplot::DesiredPlot::DP_CtorStruct{.tar_width = 48, .plot_type_name = "Scatter"})
            .guess_missingParams(ds2);


    if (not dp2_autoGuessed.has_value()) {
        std::print("{0}{1}", "Autoguessing of 'DesiresPlot_2' parameters failed \n", "Exiting ...");
        return 1;
    }

    auto plotDrawer3 = incplot::make_plotDrawer(dp2_autoGuessed.value(), ds2);

    auto outExp2 = plotDrawer3.validateAndDrawPlot();

    if (not outExp2.has_value()) {
        std::print("{0}{1}", "Invalid plot structure 2", "Exiting ...");
        return 1;
    }

    std::print("{}\n", outExp2.value()); */




    auto ds3 = incplot::Parser::parse_NDJSON_intoDS(testInput_flights);

    auto dp3_autoGuessed =
        incplot::DesiredPlot(incplot::DesiredPlot::DP_CtorStruct{.tar_width = 48, .plot_type_name = "Multiline"})
            .guess_missingParams(ds3);


    if (not dp3_autoGuessed.has_value()) {
        std::print("{0}{1}", "Autoguessing of 'DesiresPlot_3' parameters failed \n", "Exiting ...");
        return 1;
    }

    auto plotDrawer4 = incplot::make_plotDrawer(dp3_autoGuessed.value(), ds3);

    auto outExp3 = plotDrawer4.validateAndDrawPlot();

    if (not outExp3.has_value()) {
        std::print("{0}{1}", "Invalid plot structure 2", "Exiting ...");
        return 1;
    }

    std::print("{}\n", outExp3.value());

    return 0;
}