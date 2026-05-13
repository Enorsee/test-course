cmake_minimum_required(VERSION 3.16)

project(cfg_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CFGCORE_LIB "${CMAKE_SOURCE_DIR}/libcfgcore.a")

if(NOT EXISTS "${CFGCORE_LIB}")
    message(FATAL_ERROR "libcfgcore.a not found at ${CFGCORE_LIB}")
endif()

add_library(cfgcore STATIC IMPORTED)

set_target_properties(cfgcore PROPERTIES
    IMPORTED_LOCATION "${CFGCORE_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include"
)

add_executable(cfg_lab
        src/main.cpp
        src/analysis.cpp
)
target_link_libraries(cfg_lab PRIVATE cfgcore)
target_compile_options(cfg_lab PRIVATE -Wall -Wextra -Wpedantic)

add_executable(prime_path_demo 
	src/analysis.cpp
	samples/prime_path_demo.cpp
)
target_link_libraries(prime_path_demo PRIVATE cfgcore)
target_compile_options(prime_path_demo PRIVATE -Wall -Wextra -Wpedantic)
