//
// Created by james on 4/23/2024.
//

#ifndef WEBGPU_RENDERER_GLOBALS_H
#define WEBGPU_RENDERER_GLOBALS_H

#include <memory>
#include "window.h"

class Window;

class Globals {
public:
	static std::unique_ptr<Window> window;

//    static *app;
//	static std::unique_ptr<wgpu::Device> device;
//	static std::unique_ptr<wgpu::Queue> queue;
};


#endif //WEBGPU_RENDERER_GLOBALS_H
