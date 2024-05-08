cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME QuadroPlugin )
project(${PROJECT_NAME})

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

set(SOURCE_FILES
    ./src/quadro_plugin.cpp
    
../src/ttk/vtkPVExtractComponent.cxx
../src/ttk/vtkPVExtractComponent.h
../src/ttk/httplib.h
)

set(LIBRARIES
    OpenMaya Foundation
)
find_package(Eigen3 REQUIRED)

find_package(TTKBase REQUIRED)

include_directories(
    F:/DUT/QuadMeshGen/3rdparty/ttk-1.2.0/install/include/ttk/base
)
link_libraries(
    ttk::base::baseAll
    )
if (CMAKE_GENERATOR MATCHES "Visual Studio")
add_compile_options(/openmp:llvm)
endif()

# target_include_directories(QuadroPlugin PUBLIC
#   $ENV{TTK_DIR}/include
#   ../../3rdparty/cpp-httplib
#   ../../3rdparty/eigen-3.4.0
# )

# target_link_libraries(QuadroPlugin
#   PUBLIC
#     ttk::base::baseAll
#     )

build_plugin()