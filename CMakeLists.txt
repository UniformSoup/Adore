cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(Adore
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "The Adore Engine"
)

if (POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND
    NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif ()

# Options
option(ADORE_BUILD_SHARED "Build the shared library" ON)
option(ADORE_BUILD_TESTS "Build test programs" OFF)
option(ADORE_BUILD_EXAMPLES "Build examples" OFF)
option(ADORE_BUILD_DOCS "Build documentation" ON)

# Generate Version Header:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/Adore/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/Adore/version.hpp @ONLY)

# Determine LTO ON/OFF
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORT OUTPUT LTO_ERROR)
set(ADORE_LTO_ENABLED FALSE CACHE BOOL "Is LTO Enabled?")

if(LTO_SUPPORT)
    if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        message(STATUS "LTO is supported and will be enabled by default.")
        set(ADORE_LTO_ENABLED TRUE)
    endif()
else()
    message(WARNING "LTO is not supported: ${LTO_ERROR}")
endif()

find_package(Vulkan REQUIRED)
add_subdirectory(lib)
add_subdirectory(src)

# Tests and Examples
if (ADORE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (ADORE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (ADORE_BUILD_DOCS)
    add_subdirectory(docs)
endif()