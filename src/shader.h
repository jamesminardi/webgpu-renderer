#pragma once

#include "webgpu/webgpu.hpp"
#include <string>
#include <fstream>

class Shader {
public:
	static wgpu::ShaderModule loadShaderModule(wgpu::Device device, const std::string& path);

};
