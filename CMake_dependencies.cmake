include(FetchContent)

FetchContent_Declare(
    incplot-lib
    GIT_REPOSITORY https://github.com/InCom-0/incplot-lib
    GIT_TAG main
)
FetchContent_MakeAvailable(incplot-lib)