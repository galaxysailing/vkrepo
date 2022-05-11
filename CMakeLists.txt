cmake_minimum_required(VERSION 3.0.0)
project(vkrepo VERSION 0.1.0)

# c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

include_directories(external/include/glfw-3.3.7)
include_directories(external/include/glm-0.9.9.8)
include_directories(external/include/)


if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

# message(${CMAKE_SOURCE_DIR})
IF (NOT Vulkan_FOUND)
	find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/external/lib/vulkan)
	IF (Vulkan_LIBRARY)
		set(Vulkan_FOUND ON)
	    MESSAGE("Using bundled Vulkan library version")
    ENDIF()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ENDIF()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

IF (NOT Glfw3_FOUND)
	find_library(Glfw3_LIBRARY NAMES libglfw3.a glfw3 PATHS ${CMAKE_SOURCE_DIR}/external/lib/glfw3)
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

set(VULKAN_SOURCE render/vulkan_context.cpp
	render/vulkan_util.cpp)
set(VULKAN_HEADER render/vulkan_context.h
	render/vulkan_util.h)

set(VKREPO_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VK_RENDER_DIR "${VKREPO_DIR}/render")


add_executable(hello_rect example/hello_rect.cpp)
target_link_libraries(hello_rect ${Vulkan_LIBRARY} ${Glfw3_LIBRARY})

# add_executable(vkrepo ${VULKAN_HEADER} ${VULKAN_SOURCE} main.cpp hello_rect.h)
# target_link_libraries(vkrepo )
