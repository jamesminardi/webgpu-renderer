#include "world.h"
#include "globals.h"
#include "terrain/Terrain.h"
#include "terrain/TerrainRenderer.h"
#include "window.h"
#include "noise/noise.h"


World::World() :
		noiseDesc(Noise::Descriptor()),
		camera(Camera()) {

	terrain = std::make_unique<Terrain>(noiseDesc); // Don't do Terrain(noiseDesc) for make_unique since it copies and destructs... TODO RAII
	terrainRenderer = std::make_unique<TerrainRenderer>(*terrain);
	terrain->addObserver(*terrainRenderer);


	center = {0, 0};
	terrain->init(center);

//	chunk = new Chunk(Noise(noiseDesc), {0, 0}, false);



//	terrainRenderer = std::make_unique<TerrainRenderer>(TerrainRenderer(this));


	ratio = static_cast<float>(Globals::window->getWidth()) / static_cast<float>(Globals::window->getHeight());
	focalLength = 2.0f;
	near = 0.01f;
	far = 1000.0f;
	divider = 1 / (focalLength * (far - near));

	ShaderUniforms uniforms{};
	uniforms = terrainRenderer->getUniforms();

	uniforms.modelMatrix = T1 * R1 * S;

	//	camera.center = {1 * Chunk::DefaultChunkSize / 2.0f, 0.0f, 1 * Chunk::DefaultChunkSize / 2.0f};
	camera.center = {0.0f, 0.0f, 0.0f};

	uniforms.viewMatrix = camera.updateViewMatrix();

	// Projection
	fov = 2 * glm::atan(1 / focalLength);
	uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);

	uniforms.color = {0.5f, 0.6f, 1.0f, 1.0f};

	terrainRenderer->setUniforms(uniforms);




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
//	terrain->render(renderPass);
	terrainRenderer->render(renderPass);

}