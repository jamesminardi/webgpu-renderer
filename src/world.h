#pragma once

#include "terrain.h"

class World {
public:

	// Camera
	// Terrain -> Chunks -> Chunk Meshes
	// Skybox
	// Etc.



	World() {
		size = 0;
		wireFrame = false;
	};

	void load(Noise::Descriptor noiseDesc, int worldSize = 1) {
		this->size = worldSize;
		this->noise = Noise(noiseDesc);
		chunks.resize(worldSize * worldSize);
		for (int row = 0; row < size; row++) {
			for (int col = 0; col < size; col++) {
				chunks[row * size + col].load(noise, {row, col});
			}
		}
	}

	void unload() {
		for (auto& chunk : chunks) {
			chunk.unload();
		}
	}

	void setAmplitude(float amplitude) {
		for (auto& chunk : chunks) {
			chunk.amplitude = amplitude;
		}
	}

	int size; // Number of chunks per side of the world
	bool wireFrame;
	std::vector<Chunk> chunks;
	Noise noise;
};
