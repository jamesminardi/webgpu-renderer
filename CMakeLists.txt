
cmake_minimum_required(VERSION 3.21)
project(cross-platform-webgpu)

include(utils.cmake)

if (NOT EMSCRIPTEN)
    # Do not include this with emscripten, it provides its own version.
    add_subdirectory(external/glfw)
endif()
add_subdirectory(external/webgpu)
add_subdirectory(external/glfw3webgpu)
add_subdirectory(src)

target_link_libraries(app PRIVATE glfw webgpu glfw3webgpu )

# Disable GLFW build examples, tests, docs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)