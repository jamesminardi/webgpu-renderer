cmake_minimum_required(VERSION 3.20)
project(app)


include(utils.cmake)

if (NOT EMSCRIPTEN)
    # Do not include this with emscripten, it provides its own version.
    add_subdirectory(external/glfw)
endif()
add_subdirectory(external/webgpu)
add_subdirectory(external/glfw3webgpu)
#add_subdirectory(glfw3webgpu)


add_executable(app main.cpp)


target_link_libraries(app PRIVATE glfw webgpu glfw3webgpu
        )
#        webgpu glfw3webgpu)


# Disable GLFEW build examples, tests, docs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)


set_target_properties(app PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR ON
        VS_DEBUGGER_ENVIRONMENT "DAWN_DEBUG_BREAK_ON_ERROR=1"
)
if(XCODE)
    set_target_properties(app PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()


target_treat_all_warnings_as_errors(app)


# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary. Not required for DAWN.
target_copy_webgpu_binaries(app)


if (EMSCRIPTEN)

    # Generate a full web page rather than a simple WebAssembly module
    set_target_properties(app PROPERTIES SUFFIX ".html")

    # Add Emscripten-specific link options
    target_link_options(app PRIVATE
            -sUSE_GLFW=3 # Use Emscripten-provided GLFW
            -sUSE_WEBGPU # Handle WebGPU symbols
            -sASYNCIFY # Required by WebGPU-C++
    )
endif()