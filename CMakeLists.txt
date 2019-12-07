# CMakeList.txt : Top-level CMake project file, do global configuration
cmake_minimum_required (VERSION 3.8)

project ("RPiCameraService")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT WIN32)
	add_definitions(-std=c++17 -Wno-psabi)
else()
	add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif()

# control where the static and shared libraries are built so that on windows we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY, "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

find_package(OpenCV REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(dlib REQUIRED)

# Include sub-projects.
add_subdirectory("Imports/JsoncppLib")
add_subdirectory("Imports/MessagesLib")
add_subdirectory("Imports/AiDetectLib")
add_subdirectory ("CameraService")




