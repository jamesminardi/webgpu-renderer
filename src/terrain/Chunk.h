#pragma once

#include "../globals.h"
#include "glm/glm.hpp"
#include "../noise/noise.h"
#include "Mesh.h"

class Chunk {
public:

	Chunk() = default;

	Chunk(Noise& noise, glm::ivec2 worldPosition) :
			worldPos(worldPosition)
	{
		chunkSeed = noise.desc.seed * worldPos.x + worldPos.y;
		constexpr int borderedSize = DefaultChunkWidth + 1 + (DefaultBorderWidth * 2); // +1 for verts/side, +2 for 1 border on each side
		HeightMap<borderedSize> heightMap = HeightMap<borderedSize>::generate(noise, {worldPos.x * DefaultChunkWidth, worldPos.y * DefaultChunkWidth}, 0.0f, 1.0f);

		mesh = Mesh::generate<borderedSize, DefaultBorderWidth>(heightMap, {worldPos.x * DefaultChunkWidth, worldPos.y * DefaultChunkWidth});

	}

	static constexpr int DefaultChunkWidth = 4;
	static constexpr int DefaultBorderWidth = 1;
	int chunkSeed = 0;
	glm::ivec2 worldPos{};

	Mesh mesh;

};