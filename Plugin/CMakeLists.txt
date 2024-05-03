cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME QuadroPlugin )
project(${PROJECT_NAME})

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(SOURCE_FILES
    ./src/quadro_plugin.cpp
)

set(LIBRARIES
    OpenMaya Foundation
)

build_plugin()