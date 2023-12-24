cmake_minimum_required(VERSION 3.20)
project(app)


include(utils.cmake)


add_subdirectory(external/glfw)
add_subdirectory(external/webgpu)
#add_subdirectory(glfw3webgpu)


add_executable(app
        main.cpp)


target_link_libraries(app PRIVATE glfw webgpu
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