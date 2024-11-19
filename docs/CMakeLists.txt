find_package(Doxygen REQUIRED)

# Set paths for Doxygen
set(DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../docs")
set(DOXYGEN_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")

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