include(CMakePrintHelpers)

add_executable(app main.cpp
        window.cpp
        application.cpp
        implementations.cpp
        shader.cpp
        camera.cpp
        noise/noise.cpp
        terrain_renderer.cpp
        terrain_renderer.h
        globals.h
        types.h
        terrain.cpp
        world.cpp
        globals.cpp)



set_target_properties(app PROPERTIES
        CXX_STANDARD 20
        CXX_EXTENSIONS OFF
        VS_DEBUGGER_ENVIRONMENT "DAWN_DEBUG_BREAK_ON_ERROR=1"
)
if(XCODE)
    set_target_properties(app PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()
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

target_treat_all_warnings_as_errors(app)

target_copy_webgpu_binaries(app)
