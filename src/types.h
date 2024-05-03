//
// Created by james on 4/26/2024.
//

#ifndef WEBGPU_RENDERER_TYPES_H
#define WEBGPU_RENDERER_TYPES_H

#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>



struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
//	glm::vec2 uv;
};

///*
// * A structure that describes the data layout in the vertex buffer,
// * used by loadGeometryFromObj and used it in `sizeof` and `offsetof`
// * when uploading data to the GPU.
// */
//struct VertexAttributes {
//	glm::vec3 position;
//	glm::vec3 normal;
//	glm::vec3 color;
////	glm::vec2 uv;
//};

/*
 * The same structure as in the shader, replicated in C++
 */
struct ShaderUniforms {
	// We add transform matrices
	glm::mat4x4 projectionMatrix;
	glm::mat4x4 viewMatrix;
	glm::mat4x4 modelMatrix;
	std::array<float, 4> color;
};


#endif //WEBGPU_RENDERER_TYPES_H
