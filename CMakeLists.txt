cmake_minimum_required(VERSION 3.10)

# set the project name
project(QuadMeshGenerator)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# Set up libraries

add_library(qmorph STATIC
  ./src/qmorph_main.cpp
  ./src/qmorph.h
)

target_include_directories(qmorph PUBLIC
    ./3rdparty/eigen-3.2.8
)

add_library(mesh STATIC
  ./src/meshcomponents.cpp
  ./src/meshcomponents.h
)

target_include_directories(mesh PUBLIC
    ./3rdparty/eigen-3.2.8
)

# Set up executables/viewers
# Find OpenGL
find_package(OpenGL REQUIRED)

# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(./3rdparty/glfw-3.3.2)

# Add GLAD
add_subdirectory(./3rdparty/glad)

# Add ImGUI
add_subdirectory(./3rdparty/imgui-1.77)

# Add tinyobj
add_library(tinyobj STATIC
    ./3rdparty/tinyobj/tiny_obj_loader.h
    ./3rdparty/tinyobj/tiny_obj_loader.cc
)

# Add OpenFBX
add_subdirectory(./3rdparty/OpenFBX)

add_executable(GLMeshViewer
    ./src/glmeshviewer_main.cpp
)

target_include_directories(GLMeshViewer PUBLIC 
    ./3rdparty/imgui-1.77
    ./3rdparty/glm
    ./3rdparty/tinyobj
)

target_link_libraries(GLMeshViewer PUBLIC qmorph mesh glad glfw imgui tinyobj OpenFBX)
