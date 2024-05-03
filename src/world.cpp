#include "world.h"
#include "globals.h"
#include "terrain.h"
#include "terrain_renderer.h"
#include "window.h"
#include "noise/noise.h"


World::World() :
		noiseDesc(Noise::Descriptor()),
		camera(Camera()) {


//	chunk = new Chunk(Noise(noiseDesc), {0, 0}, false);



//	terrainRenderer = std::make_unique<TerrainRenderer>(TerrainRenderer(this));

	center = Terrain::DefaultCenter;

	ratio = static_cast<float>(Globals::window->getWidth()) / static_cast<float>(Globals::window->getHeight());
	focalLength = 2.0f;
	near = 0.01f;
	far = 1000.0f;
	divider = 1 / (focalLength * (far - near));


	m_uniforms.modelMatrix = T1 * R1 * S;

	camera.center = {1 * Chunk::DefaultChunkSize / 2.0f, 0.0f, 1 * Chunk::DefaultChunkSize / 2.0f};

	m_uniforms.viewMatrix = camera.updateViewMatrix();

	// Projection
	fov = 2 * glm::atan(1 / focalLength);
	m_uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);

	m_uniforms.color = {0.5f, 0.6f, 1.0f, 1.0f};



	terrain = std::make_unique<Terrain>(Terrain(this, noiseDesc));


};



void World::update() {

	// Update Center

	// Update center in terrain
	terrain->update(center);
	// Terrain will update chunks in state manager

	// Update terrain renderer (will account for new chunks in state manager)
//	terrainRenderer->update(*this);

	// Update Camera


}

void World::unload() {

}

void World::render(wgpu::RenderPassEncoder& renderPass) {

//	terrainRenderer->render(*this, renderPass);
	terrain->render(renderPass);

}