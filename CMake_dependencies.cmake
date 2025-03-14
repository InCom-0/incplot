include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.16.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    more_concepts
    GIT_REPOSITORY https://github.com/MiSo1289/more_concepts.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(more_concepts)

FetchContent_Declare(
    OOF
    GIT_REPOSITORY https://github.com/InCom-0/oof
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(OOF)

find_package(nlohmann_json REQUIRED)
find_package(DataFrame CONFIG REQUIRED)