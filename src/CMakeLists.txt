# Collect source files
set(SOURCES
    
)

if (${PROJECT_NAME}_BUILD_STATIC_LIBS)
    add_library(Static STATIC ${SOURCES})
    target_link_libraries(Static PRIVATE Vulkan::Vulkan)
    target_include_directories(Static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(Static PROPERTIES OUTPUT_NAME PROJECT_NAME)
endif()

if (${PROJECT_NAME}_BUILD_SHARED_LIBS)
    add_library(Shared SHARED ${SOURCES})
    target_link_libraries(Shared PRIVATE Vulkan::Vulkan)
    target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(Shared PROPERTIES OUTPUT_NAME PROJECT_NAME)
endif()
