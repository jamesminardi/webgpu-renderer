#pragma once

#include "types.h"
#include "camera.h"
#include "noise/noise.h"
#include "terrain_renderer.h"

class Terrain;
class Chunk;
//class TerrainRenderer;

class World {
public:

	// Camera
	// Chunks -> Chunk Meshes
	// Skybox
	// Etc.


	Camera camera;
	glm::vec3 focalPoint{};
	float ratio{};
	float focalLength{};
	float near{};
	float far{};
	float divider{};
	glm::mat4 S = glm::mat4(1.0);
	glm::mat4 T1 = glm::mat4(1.0);
	glm::mat4 R1 = glm::mat4(1.0);
	glm::mat4 R2 = glm::mat4(1.0);
	glm::mat4 T2 = glm::mat4(1.0);
	float fov{};

//	std::unique_ptr<TerrainRenderer> terrainRenderer;
	Noise::Descriptor noiseDesc;
	std::unique_ptr<Terrain> terrain;
	glm::ivec2 center{};



    // TODO: Update app to use terrain class instead of chunk/world
    // TODO: "Player" class? holds the center to be used for generating chunks. Doesn't necessarily have to have the camera


	World();


	void update();

	void unload();

	void render(wgpu::RenderPassEncoder& renderPass);


};
