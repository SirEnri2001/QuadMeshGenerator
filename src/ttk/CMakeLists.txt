cmake_minimum_required(VERSION 3.21)

# name of the project
project(ttkExample-c++)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE "Release")
find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  IOGeometry
  InteractionStyle
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  IOLegacy
  IOXML
)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "ReadOBJ: Unable to find the VTK build folder.")
endif()
find_package(TTKBase REQUIRED)
find_package(TTKVtk REQUIRED)
find_package(Eigen3 REQUIRED)


if (CMAKE_GENERATOR MATCHES "Visual Studio")
add_compile_options(/openmp:llvm)
endif()

add_executable(ttkExample-c++ 
main.cpp
vtkPVExtractComponent.cxx
vtkPVExtractComponent.h
httplib.h
)

target_include_directories(ttkExample-c++ PUBLIC
  $ENV{TTK_DIR}/include
  ../../3rdparty/cpp-httplib
  ../../3rdparty/eigen-3.4.0
)

target_link_libraries(ttkExample-c++
  PUBLIC
    ttk::base::baseAll
    ttk::vtk::ttkMorseSmaleQuadrangulation
    ttk::vtk::ttkMorseSmaleComplex
    ttk::vtk::ttkScalarFieldNormalizer
    ttk::vtk::ttkPersistenceDiagram
    ttk::vtk::ttkTopologicalSimplification
    ttk::vtk::ttkQuadrangulationSubdivision
    ttk::vtk::ttkEigenField
    ${VTK_LIBRARIES}
    )

vtk_module_autoinit(
  TARGETS ttkExample-c++
  MODULES ${VTK_LIBRARIES}
)