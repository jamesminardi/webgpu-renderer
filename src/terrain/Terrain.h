#pragma once

#include "../globals.h"
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image_write.h>
#include "../noise/noise.h"
#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include "../types.h"
#include "Mesh.h"
#include "HeightMap.h"
#include "Grid.h"
#include "Chunk.h"
#include "RoiManager.h"
#include "Observer.h"

enum TerrainUpdate {
	UPDATED_CHUNKS_LOADED
};

/**
* Manages the chunks in the world.
*/
class Terrain : public IObservable<Terrain, RoiManager> {

public:

	RoiManager loadManager;
	bool regenerate = false;

	// Ordered map perhaps better suited for lots of adding and removing
	std::map<glm::ivec2, Chunk, decltype(posCmp)> chunks;


	glm::ivec2 currentChunkPos{};

	//	Chunk chunk;
private:


	Noise terrainNoise;
	int chunkSize{};
	int numVisibleChunks{};



public:


	Terrain() = default;


	explicit Terrain(Noise::Descriptor noiseDesc);

	void init(glm::ivec2 startChunk);

	// Updates the visible chunks list based on center position
	void update(glm::ivec2 centerChunkPos);

//	void setNoise(Noise::Descriptor noiseDesc);
//	void setWireFrame(bool wire);

//	bool isWireFrame();

//	void createRenderPipelines();
//	void terminateRenderPipeline();

//	void render(wgpu::RenderPassEncoder &renderPass);


//	void initChunkBuffers(Chunk& chunk);

//	void initUniforms();
//	void writeUniforms();
//	void initBindGroup();
	//	void initChunkUniforms(Chunk& chunk);

	//	void initChunkBindGroup(Chunk& chunk);



};
