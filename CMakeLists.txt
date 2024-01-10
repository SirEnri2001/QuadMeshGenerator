cmake_minimum_required(VERSION 3.10)

# set the project name
project(QuadMeshGenerator)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# Set up libraries

add_library(thread_support STATIC
    ./src/thread_support/thread_support.cpp
    ./src/thread_support/thread_support.h
)

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
  ./src/qmorph/qmorph_display.cpp
  ./src/qmorph/qmorph_display.h
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
  ./src/mesh/meshassert.h
  ./src/mesh/meshassert.cpp
  ./src/mesh/meshattribute.h
  ./src/mesh/meshattribute.cpp
  ./src/mesh/meshnormal.h
  ./src/mesh/meshnormal.cpp
  ./src/mesh/meshuv.h
  ./src/mesh/meshuv.cpp
  ./src/mesh/meshrecorder.cpp
  ./src/mesh/meshrecorder.h
  ./src/mesh/meshcontroller.cpp
  ./src/mesh/meshcontroller.h
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

target_link_libraries(GLMeshViewer PUBLIC thread_support qmorph mesh glad glfw imgui tinyobj OpenFBX opengl32)

add_executable(test_thread
    ./src/tests/test_thread.cpp
    ./src/tests/test_thread.h
)
target_link_libraries(test_thread PUBLIC thread_support)

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
    set(QMORPH_OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/qmorph.dir)
    message("-- Object files will be output to: ${QMORPH_OBJECT_DIR}")
    add_custom_target(gcov
        COMMAND mkdir coverage
        COMMAND ${CMAKE_MAKE_PROGRAM} test
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_custom_command(TARGET gcov
        COMMAND echo "=================== GCOV ===================="
        COMMAND gcov -b ${CMAKE_SOURCE_DIR}/src/qmorph/*.cpp -o ${QMORPH_OBJECT_DIR}/src/qmorph
        COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/coverage"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
    )
    add_dependencies(gcov GLMeshViewer)
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)
    add_custom_target(scrub
        COMMAND ${CMAKE_MAKE_PROGRAM} clean
        COMMAND rm -f ${QMORPH_OBJECT_DIR}/*.gcno
        COMMAND rm -f ${QMORPH_OBJECT_DIR}/*.gcda
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    enable_testing()
    add_test(output_test ${CMAKE_CURRENT_BINARY_DIR}/GLMeshViewer)
    set_tests_properties(output_test PROPERTIES PASS_REGULAR_EXPRESSION "0;5;10")
endif ()