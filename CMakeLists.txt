cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(Adore
    VERSION 0.1
    LANGUAGES CXX
    DESCRIPTION "The Adore Engine"
)

# Options
option(${PROJECT_NAME}_BUILD_STATIC_LIBS "Build the static library" ON)
option(${PROJECT_NAME}_BUILD_SHARED_LIBS "Build the shared library" ON)
option(${PROJECT_NAME}_BUILD_TESTS "Build test programs" OFF)
option(${PROJECT_NAME}_BUILD_EXAMPLES "Build examples" OFF)
option(${PROJECT_NAME}_BUILD_DOCS "Build documentation" ON)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Generate the header for exports
generate_export_header(PROJECT_NAME)
include("${PROJECT_NAME}_exports.h")

# Determine LTO ON/OFF
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_SUPPORT OUTPUT LTO_ERROR)

if(LTO_SUPPORT)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "LTO is supported and will be enabled by default.")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
else()
    message(WARNING "LTO is not supported: ${LTO_ERROR}")
endif()

# Add dependencies
add_subdirectory(lib)

# Add source code
add_subdirectory(src)

# Tests and Examples
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    # Set paths for Doxygen
    set(DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs")
    set(DOXYGEN_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

    # Generate default Doxygen configuration file if not present
    if(NOT EXISTS ${DOXYGEN_CONFIG_FILE})
        message(STATUS "Generating default Doxygen configuration file...")
        execute_process(
            COMMAND ${DOXYGEN_EXECUTABLE} -g ${DOXYGEN_CONFIG_FILE}
        )
    endif()

    # Configure custom Doxyfile
    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME}")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DOXYGEN_OUTPUT_DIRECTORY "${DOXYGEN_OUTPUT_DIR}")
    set(DOXYGEN_INPUT "${DOXYGEN_INPUT_DIR}")
    set(DOXYGEN_HTML_OUTPUT "${DOXYGEN_OUTPUT_DIR}/html")
    set(DOXYGEN_GENERATE_LATEX YES)
    configure_file(${DOXYGEN_CONFIG_FILE} ${DOXYGEN_CONFIG_FILE} @ONLY)

    # Add custom target for docs
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen..."
        VERBATIM
    )
endif()