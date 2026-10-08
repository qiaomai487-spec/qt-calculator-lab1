cmake_minimum_required(VERSION 3.16)

project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Calculator
    main.cpp
    calculator.h
    calculator.cpp
    calculator.ui
    resources.qrc
)

target_link_libraries(Calculator PRIVATE Qt6::Widgets)

set_target_properties(Calculator PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
