cmake_minimum_required(VERSION 3.29.20240507-gf2b76d7)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

project(advent-of-code.c++.2024 CXX)

add_subdirectory(src)
