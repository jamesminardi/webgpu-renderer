#pragma once

#include "terrain.h"

class World {
public:

	// Camera
	// Chunks -> Chunk Meshes
	// Skybox
	// Etc.

	Camera camera;
	std::vector<Chunk> chunks;
	bool dirty;


	World() {
		size = 0;
		wireFrame = false;
		dirty = false;

		camera = Camera();
	};

	void load(Noise::Descriptor noiseDesc, int worldSize = 1) {
//		this->size = worldSize;
//		this->noise = Noise(noiseDesc);
//		camera.center = {worldSize * Chunk::DefaultChunkSize / 2.0f, 0.0f, worldSize * Chunk::DefaultChunkSize / 2.0f};
		chunks.resize(worldSize * worldSize);
//		for (int row = 0; row < size; row++) {
//			for (int col = 0; col < size; col++) {
//				chunks[row * size + col].load(noise, {row, col}, wireFrame);
//			}
//		}


        chunks[0] = Chunk(noise, {0, 0});
	}

	void setWireFrame(bool wire) {
		if (wire == wireFrame) return;
		wireFrame = wire;
		reload();
	}

	[[nodiscard]] bool isWireFrame() const {
		return wireFrame;
	}

	void reload() {
		dirty = true;
//		for (auto& chunk : chunks) {
//			chunk.dirty = true;
//		}
	}

	void update() {
		for (auto& chunk : chunks) {

//			if (chunk.dirty || dirty) {
//				chunk.unload();
//				chunk.load(noise, chunk.worldPos, wireFrame);
//				chunk.dirty = false;
//			}
		}
		dirty = false;
	}

	void unload() {
		for (auto& chunk : chunks) {
//			chunk.unload();
		}
	}



private:
	int size; // Number of chunks per side of the world
	bool wireFrame;
	Noise noise;
};
