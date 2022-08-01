cmake_minimum_required(VERSION 3.0.0)
project(vkrepo VERSION 0.1.0)

# c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

set(VKREPO_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(engine/external/include/glfw-3.3.7)
include_directories(engine/external/include/glm-0.9.9.8)
include_directories(engine/external/include/vulkan-1.2.189.2)
include_directories(common/)


# if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
# 	message(STATUS "Using module to find Vulkan")
# 	find_package(Vulkan)
# endif()

IF (NOT Vulkan_FOUND)
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${VKREPO_ROOT_DIR}/engine/external/lib/vulkan)
	find_library(Shaderc_LIBRARY NAMES shaderc_shared shaderc PATHS ${VKREPO_ROOT_DIR}/engine/external/lib/vulkan)
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
	    MESSAGE("Using bundled Vulkan library version")
    ENDIF()
	IF (Shaderc_LIBRARY)
		set(Shaderc_FOUND ON)
	    MESSAGE("Using bundled Shaderc library version")
    ENDIF()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

IF (NOT Shaderc_FOUND)
	message(FATAL_ERROR "Could not find shaderc library!")
ELSE()
	message(STATUS ${Shaderc_LIBRARY})
ENDIF()

IF (NOT Glfw3_FOUND)
	find_library(Glfw3_LIBRARY NAMES libglfw3.a glfw3 PATHS ${VKREPO_ROOT_DIR}/engine/external/lib/glfw3)
	IF (Glfw3_LIBRARY)
		set(Glfw3_FOUND ON)
	    MESSAGE("Using bundled glfw3 library version")
    ENDIF()
ENDIF()

IF (NOT Glfw3_FOUND)
	message(FATAL_ERROR "Could not find glfw3 library!")
ELSE()
	message(STATUS ${Glfw3_LIBRARY})
ENDIF()

file(GLOB_RECURSE VULKAN_SOURCE "${VKREPO_ROOT_DIR}/engine/render/*.cpp")
file(GLOB_RECURSE VULKAN_HEADER "${VKREPO_ROOT_DIR}/engine/render/*.h")

set(VKREPO_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VK_RENDER_DIR "${VKREPO_DIR}/render")


add_executable(vk_render tests/vk_render_test.cpp ${VULKAN_SOURCE} ${VULKAN_HEADER})
target_link_libraries(vk_render ${Vulkan_LIBRARY} ${Glfw3_LIBRARY} ${Shaderc_LIBRARY})

add_executable(split_sum_approx tests/split_sum_approx.cpp ${VULKAN_SOURCE} ${VULKAN_HEADER})
target_link_libraries(split_sum_approx ${Vulkan_LIBRARY} ${Glfw3_LIBRARY} ${Shaderc_LIBRARY})

target_include_directories(
  split_sum_approx
  PUBLIC $<BUILD_INTERFACE:${VKREPO_ROOT_DIR}/engine>
)
