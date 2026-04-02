cmake_minimum_required(VERSION 3.21)
project(SkyUIRecentSort VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(binary_io CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)
find_path(FAST_CPP_CSV_PARSER_INCLUDE_DIRS NAMES "csv.h"
    PATH_SUFFIXES "fast-cpp-csv-parser" REQUIRED)

add_library(SkyUIRecentSort SHARED
    src/main.cpp
    src/Hooks.cpp
    src/AcquiredTracker.cpp
    src/Serialization.cpp
    src/PapyrusBindings.cpp
    src/ScaleformExtension.cpp
    src/ConfigInjector.cpp
)

target_include_directories(SkyUIRecentSort PRIVATE
    src
    "${FAST_CPP_CSV_PARSER_INCLUDE_DIRS}"
)

target_precompile_headers(SkyUIRecentSort PRIVATE src/PCH.h)

target_link_libraries(SkyUIRecentSort
    PRIVATE
        CommonLibSSE::CommonLibSSE
)

if (MSVC)
    target_compile_definitions(SkyUIRecentSort PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
