# Test gtest unit test
cmake_minimum_required (VERSION 3.8)

set(PROJECT_NAME "unittest")
set(TestArgs "")

include(GoogleTest)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "include/*.h")

include_directories(include ${OpenCV_INCLUDE_DIRS} "${gtest_SOURCE_DIR}/include")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(${PROJECT_NAME} ${SOURCES})

GTEST_ADD_TESTS(${PROJECT_NAME} "${TestArgs}" ${SOURCES})

target_link_libraries(${PROJECT_NAME} gtest)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} dlib::dlib)
target_link_libraries(${PROJECT_NAME} libzmq-static)
target_link_libraries(${PROJECT_NAME} MessagesLib)
target_link_libraries(${PROJECT_NAME} AiDetectLib)
if(NOT WIN32)
	target_link_libraries(${PROJECT_NAME} stdc++fs)
	#target_link_libraries(${PROJECT_NAME} GPIOLib)
endif()

