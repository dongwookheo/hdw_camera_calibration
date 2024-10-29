cmake_minimum_required(VERSION 3.0.2...3.28)
project(Camera_Calibration
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(${PROJECT_NAME}_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(${PROJECT_NAME}_SRC ${${PROJECT_NAME}_PATH}/src)

add_subdirectory(modules)

find_package(OpenCV REQUIRED HINTS ${CMAKE_SOURCE_DIR}/thirdparty/OpenCV/install/lib/cmake/opencv4)
find_package(yaml-cpp REQUIRED)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC}/main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
  CameraCalibration
)
