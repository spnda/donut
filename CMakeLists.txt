cmake_minimum_required(VERSION 3.28)
project(donut)

find_package(glm REQUIRED)

add_executable(donut main.cpp)
target_link_libraries(donut PRIVATE glm::glm curses)
target_compile_features(donut PRIVATE cxx_std_23)
