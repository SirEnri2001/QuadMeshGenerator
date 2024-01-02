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
  ./src/qmorph/frontedge_operator.cpp
  ./src/qmorph/frontedge_operator.h
  ./src/qmorph/qmorph_operator.cpp
  ./src/qmorph/qmorph_operator.h
  ./src/qmorph/sidedefine_operator.cpp
  ./src/qmorph/sidedefine_operator.h
  ./src/qmorph/component_operator.h
  ./src/qmorph/component_operator.cpp
  ./src/qmorph/util.h
  ./src/qmorph/util.cpp
  ./src/qmorph/smoother.h
  ./src/qmorph/smoother.cpp
  ./src/qmorph/frontedge.h
)

target_include_directories(qmorph PUBLIC
    ./3rdparty/eigen-3.2.8
    ./3rdparty/glm
)

add_library(mesh STATIC
  ./src/mesh/meshcomponents.cpp
  ./src/mesh/meshcomponents.h
  ./src/mesh/mesh.h
  ./src/mesh/meshdisplay.cpp
  ./src/mesh/meshdisplay.h
  ./src/mesh/meshio.cpp
  ./src/mesh/meshio.h
  ./src/mesh/meshoperator.h
  ./src/mesh/meshoperator.cpp
  ./src/mesh/misc/parser.h
  ./src/mesh/misc/strutil.h
)

target_include_directories(mesh PUBLIC
    ./3rdparty/eigen-3.2.8
    ./3rdparty/glm
    ./3rdparty/tinyobj
    ./3rdparty/boost_1_82_0
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
add_subdirectory(./3rdparty/imgui-1.90)

# Add tinyobj
add_library(tinyobj STATIC
    ./3rdparty/tinyobj/tiny_obj_loader.h
    ./3rdparty/tinyobj/tiny_obj_loader.cc
)

# Add OpenFBX
add_subdirectory(./3rdparty/OpenFBX)

add_executable(GLMeshViewer
    ./src/glmeshviewer_main.cpp
    ./src/glmeshviewer.cpp
    ./src/glmeshviewer.h
    ./src/camera.h
    ./src/camera.cpp
    ./src/shader.h
    ./src/shader.cpp
    ./src/drawable.h
    ./src/drawable.cpp
    ./src/objmodel.cpp
    ./src/objmodel.h
    ./src/test_operator.cpp
    ./src/test_operator.h
)

target_include_directories(GLMeshViewer PUBLIC 
    ./3rdparty/imgui-1.90
    ./3rdparty/glm
    ./3rdparty/tinyobj
)

target_link_libraries(GLMeshViewer PUBLIC qmorph mesh glad glfw imgui tinyobj OpenFBX opengl32)
