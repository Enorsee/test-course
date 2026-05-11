cmake_minimum_required(VERSION 3.16)

project(cfg_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cfgcore
    src/util.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/cfg.cpp
    src/analysis.cpp
)

target_include_directories(cfgcore PUBLIC include)

target_compile_options(cfgcore PRIVATE -Wall -Wextra -Wpedantic)

add_executable(cfg_lab src/main.cpp)
target_link_libraries(cfg_lab PRIVATE cfgcore)

add_executable(prime_path_demo samples/prime_path_demo.cpp)
target_link_libraries(prime_path_demo PRIVATE cfgcore)
