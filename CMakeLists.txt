cmake_minimum_required(VERSION 3.12.1)

project(dandy-vr-remap)

# Configuration Options
# set(OPENVR_ROOT_DIR "G:/workspace/c++/openvr")
set(OPENVR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/openvr)
set(CMG_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../CMGEngine)


find_library(OPENVR_LIBRARIES
  NAMES
    openvr_api
  PATHS
    ${OPENVR_ROOT_DIR}/bin
    ${OPENVR_ROOT_DIR}/lib
  PATH_SUFFIXES
    osx32
    linux64
    win32
	Win32
	x86
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)
set(OPENVR_INCLUDE_DIR ${OPENVR_ROOT_DIR}/headers)

function(link_cmg target_name lib_name)
	set(lib_var "lib_${lib_name}_path")
	find_library(${lib_var} NAMES ${lib_name}
		PATHS ${CMG_ROOT_DIR}/INSTALL/lib/Debug
		NO_DEFAULT_PATH
		NO_CMAKE_FIND_ROOT_PATH
		)
	message(STATUS "Linking ${target_name} to ${${lib_var}}")
	target_link_libraries(${target_name} PRIVATE ${${lib_var}})
endfunction()

set(CMG_INCLUDE_DIR ${CMG_ROOT_DIR}/INSTALL/include)

# Find OpenGL package
find_package(OpenGL)
message(STATUS "OPENGL_INCLUDE_DIRS = ${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL_LIBRARIES = ${OPENGL_LIBRARIES}")

message(STATUS "CMG_ROOT_DIR = ${CMG_ROOT_DIR}")
message(STATUS "CMG_INCLUDE_DIR = ${CMG_INCLUDE_DIR}")

message(STATUS "OPENVR_ROOT_DIR = ${OPENVR_ROOT_DIR}")
message(STATUS "OPENVR_INCLUDE_DIR = ${OPENVR_INCLUDE_DIR}")
message(STATUS "OPENVR_LIBRARIES = ${OPENVR_LIBRARIES}")

# Create target
set(TARGET_NAME "dandy-vr-remap")
add_executable(${TARGET_NAME}
	src/main.cpp
	src/app.hpp
	src/app.cpp
	src/vr/actions.hpp
	src/vr/actions.cpp
	src/vr/device.hpp
	src/vr/device.cpp
	src/outputs/outputs.hpp
	src/outputs/outputs.cpp
	src/inputs/inputs.hpp
	src/inputs/inputs.cpp
	src/mappings/bindings.hpp
	src/mappings/bindings.cpp
	src/mappings/sphere_aim_controller.hpp
	src/mappings/sphere_aim_controller.cpp
	src/mappings/logic_parser.hpp
	src/mappings/logic_parser.cpp
	src/mappings/bind_config.hpp
	src/mappings/bind_config.cpp
)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${TARGET_NAME} PRIVATE ${OPENVR_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE ${OPENVR_LIBRARIES})
# link_directories(${OPENVR_ROOT_DIR}/bin)

target_include_directories(${TARGET_NAME} PRIVATE ${CMG_INCLUDE_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${OPENGL_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE ${OPENGL_LIBRARIES})
link_cmg(${TARGET_NAME} cmgCore)
link_cmg(${TARGET_NAME} cmgMath)
link_cmg(${TARGET_NAME} cmgInput)
link_cmg(${TARGET_NAME} cmgGraphics)
link_cmg(${TARGET_NAME} cmgApplication)

link_cmg(${TARGET_NAME} SOIL)
link_cmg(${TARGET_NAME} zlib)
link_cmg(${TARGET_NAME} dinput8)
link_cmg(${TARGET_NAME} FreeType)
