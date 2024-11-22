include(GenerateExportHeader)

# Collect source files
set(SOURCES
    Logger.cpp
)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Set output directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})  # For libraries

# Build library
if (ADORE_BUILD_SHARED)
    add_library(Adore SHARED ${SOURCES})
else()
    add_library(Adore STATIC ${SOURCES})
    target_compile_definitions(Adore PUBLIC ADORE_STATIC_DEFINE)
endif()

# Configure Exports
generate_export_header(Adore EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/export.h BASE_NAME ADORE)
target_link_libraries(Adore PRIVATE Vulkan::Vulkan)
target_include_directories(Adore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(Adore PROPERTIES
                      OUTPUT_NAME ${PROJECT_NAME}
                      INTERPROCEDURAL_OPTIMIZATION ${ADORE_LTO_ENABLED})