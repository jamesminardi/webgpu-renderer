
cmake_minimum_required(VERSION 3.21)
project(webgpu-renderer)

include(cmake/utils.cmake)
include(cmake/imgui.cmake)

# We add an option to enable different settings when developping the app than
# when distributing it.
option(DEV_MODE "Set up development helper settings" ON)


if (NOT EMSCRIPTEN)
    # Do not include this with emscripten, it provides its own version.
    add_subdirectory(external/glfw) # Must be before webgpu since DAWN provides its own glfw
endif()

add_subdirectory(external/webgpu)
add_subdirectory(external/glfw3webgpu)
add_subdirectory(external/glm)
add_subdirectory(src)

if(DEV_MODE)
    # Load resources from the source tree to allow for
    # dynamic editing of resources (like shaders)
    target_compile_definitions(app PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
else()
    # Load resources relatively to wherever the
    # executable is launched from so the binary is portable
    target_compile_definitions(app PRIVATE
            RESOURCE_DIR="./resources"
    )

    target_copy_resources(app resources)

endif()


target_link_libraries(app PRIVATE glfw webgpu glfw3webgpu glm imgui)

target_include_directories(app
        PRIVATE external/stb
)


# Disable GLFW build examples, tests, docs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)