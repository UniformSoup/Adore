# Add dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW build docs.")
set(GLFW_INSTALL OFF CACHE BOOL "Disable GLFW install.")
set(GLFW_LIBRARY_TYPE "STATIC" CACHE STRING "Build GLFW as STATIC lib.")
set(GLFW_USE_WAYLAND ON CACHE BOOL "Use wayland if available.")

add_subdirectory(glfw)
add_subdirectory(glm)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(glm PRIVATE GLM_FORCE_RADIANS
                                       GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
                                       GLM_FORCE_INTRINSICS
                                       GLM_FORCE_SWIZZLE)

add_library(stb STATIC stb_impl.cpp ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.h)
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)