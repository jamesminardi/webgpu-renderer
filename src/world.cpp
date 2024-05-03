#include "world.h"
#include "globals.h"
#include "terrain.h"
#include "terrain_renderer.h"
#include "window.h"
#include "noise/noise.h"


World::World() :
		noiseDesc(Noise::Descriptor()),
		camera(Camera()) {


	chunk = new Chunk(Noise(noiseDesc), {0, 0}, false);


	terrain = std::make_unique<Terrain>(Terrain(noiseDesc));

//	terrainRenderer = std::make_unique<TerrainRenderer>(TerrainRenderer(this));

	center = Terrain::DefaultCenter;

	ratio = static_cast<float>(Globals::window->getWidth()) / static_cast<float>(Globals::window->getHeight());
	focalLength = 2.0f;
	near = 0.01f;
	far = 1000.0f;
	divider = 1 / (focalLength * (far - near));


	m_uniforms.modelMatrix = T1 * R1 * S;

	m_uniforms.viewMatrix = camera.updateViewMatrix();

	// Projection
	fov = 2 * glm::atan(1 / focalLength);
	m_uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);

	m_uniforms.color = {0.5f, 0.6f, 1.0f, 1.0f};

};

void World::load(Noise::Descriptor noiseDescriptor, int worldSize) {


	camera.center = {worldSize * Chunk::DefaultChunkSize / 2.0f, 0.0f, worldSize * Chunk::DefaultChunkSize / 2.0f};
//		chunks.resize(worldSize * worldSize);
//		for (int row = 0; row < size; row++) {
//			for (int col = 0; col < size; col++) {
//				chunks[row * size + col].load(noise, {row, col}, wireFrame);
//			}
//		}


	terrain->createRenderPipelines();
	terrain->initChunkBuffers(*chunk);
	terrain->initChunkUniforms(*chunk);
	terrain->initChunkBindGroup(*chunk);


}



void World::update() {

	// Update Center

	// Update center in terrain
//	terrain->update(center);
	// Terrain will update chunks in state manager

	// Update terrain renderer (will account for new chunks in state manager)
//	terrainRenderer->update(*this);

	// Update Camera


}

void World::unload() {

}

void World::render(wgpu::RenderPassEncoder& renderPass) {

//	terrainRenderer->render(*this, renderPass);
	terrain->render(*this, renderPass);

}