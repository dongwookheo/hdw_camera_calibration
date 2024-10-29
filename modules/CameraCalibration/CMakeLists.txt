cmake_minimum_required(VERSION 3.0.2...3.28)
project(CameraCalibration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(${PROJECT_NAME}_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(HEADER_FILES
        ${${PROJECT_NAME}_PATH}/include/CalibrationConfig.hpp
        ${${PROJECT_NAME}_PATH}/include/CameraCalibration.hpp
)
set(SOURCE_FILES
        ${${PROJECT_NAME}_PATH}/src/CalibrationConfig.cpp
        ${${PROJECT_NAME}_PATH}/src/CameraCalibration.cpp
)

add_library(CameraCalibration
        ${HEADER_FILES}
        ${SOURCE_FILES}
)

find_package(yaml-cpp REQUIRED)
find_package(OpenCV REQUIRED HINTS ${CMAKE_SOURCE_DIR}/thirdparty/OpenCV/install/lib/cmake/opencv4)

if(OpenCV_FOUND)
    message(STATUS "OpenCV Found! - ${OpenCV_DIR}")
endif()

target_include_directories(CameraCalibration PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(CameraCalibration PUBLIC
        ${OpenCV_INCLUDE_DIRS}
        ${YAML_CPP_INCLUDE_DIR}
)

target_link_libraries(CameraCalibration PUBLIC
        ${OpenCV_LIBS}
        ${YAML_CPP_LIBRARIES}
)