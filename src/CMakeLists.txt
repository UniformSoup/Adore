include(GenerateExportHeader)

# Collect source files
set(SOURCES
    Log.cpp
    Context.cpp
    Window.cpp
    Shader.cpp
    Renderer.cpp
    Internal/Log.cpp
    Internal/Vulkan/Context.cpp
    Internal/Vulkan/Window.cpp
    Internal/Vulkan/Shader.cpp
    Internal/Vulkan/Renderer.cpp
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
generate_export_header(Adore EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../include/Adore/Export.hpp BASE_NAME ADORE)
target_link_libraries(Adore PRIVATE Vulkan::Vulkan glfw)
target_include_directories(Adore SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(Adore PROPERTIES
                      OUTPUT_NAME ${PROJECT_NAME}
                      INTERPROCEDURAL_OPTIMIZATION ${ADORE_LTO_ENABLED})