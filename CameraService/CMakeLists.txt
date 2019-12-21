# CMakeList.txt : CMake project for RPiCamera, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(RPICAMERA_NAME "CameraService")

file(GLOB_RECURSE EXE_SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp src/*.hpp include/*.hpp)

include_directories(include ${ZeroMQ_INCLUDE_DIR})

# Silence C++ 17 deprication warning
add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Add source to this project's executable.
add_executable (${RPICAMERA_NAME} ${EXE_SOURCE_FILES})

target_link_libraries(${RPICAMERA_NAME} ${OpenCV_LIBS})
target_link_libraries(${RPICAMERA_NAME} libzmq-static)
target_link_libraries(${RPICAMERA_NAME} MessagesLib)
target_link_libraries(${RPICAMERA_NAME} AiDetectLib)

if(NOT WIN32)
	target_link_libraries(${RPICAMERA_NAME} stdc++fs)
	#target_link_libraries(${RPICAMERA_NAME} GPIOLib)
endif()

target_include_directories(${RPICAMERA_NAME} PUBLIC include PRIVATE src)

# Copy config file
file(COPY "${PROJECT_SOURCE_DIR}/CameraService/assets/message_developer.json" DESTINATION "${CMAKE_BINARY_DIR}/bin/")

# Copy Face detection files to /assets folder
file(GLOB AI_ASSETS "${PROJECT_SOURCE_DIR}/Imports/AiDetectLib/assets/*")
file(COPY ${AI_ASSETS} DESTINATION "${CMAKE_BINARY_DIR}/bin/assets/")

install(FILES "${CMAKE_BINARY_DIR}/bin/${RPICAMERA_NAME}" "${PROJECT_SOURCE_DIR}/CameraService/assets/message_developer.json" 
		DESTINATION /opt/${RPICAMERA_NAME}
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ)
