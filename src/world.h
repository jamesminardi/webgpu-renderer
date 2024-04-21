#pragma once

#include "terrain.h"

class World {
public:

	// Camera
	// Chunks -> Chunk Meshes
	// Skybox
	// Etc.

	Camera camera;
	Chunk chunk;
	bool dirty;


	World() {
		size = 0;
		wireFrame = false;
		dirty = false;

		camera = Camera();
	};

	void load(Noise::Descriptor noiseDesc, int worldSize = 1) {
		this->size = worldSize;
		this->noise = Noise(noiseDesc);
		camera.center = {worldSize * Chunk::DefaultChunkSize / 2.0f, 0.0f, worldSize * Chunk::DefaultChunkSize / 2.0f};
//		chunks.resize(worldSize * worldSize);
//		for (int row = 0; row < size; row++) {
//			for (int col = 0; col < size; col++) {
//				chunks[row * size + col].load(noise, {row, col}, wireFrame);
//			}
//		}


        chunk = Chunk(noise, {0, 0});
	}

    void setNoise(Noise::Descriptor noiseDesc) {
        noise = Noise(noiseDesc);
        dirty = true;
    }

	void setWireFrame(bool wire) {
		if (wire == wireFrame) return;
		wireFrame = wire;
		dirty = true;
	}

	[[nodiscard]] bool isWireFrame() const {
		return wireFrame;
	}


	void update() {

        if (dirty) {
            chunk = Chunk(noise, {0, 0});
            dirty = false;
        }

		dirty = false;
	}

	void unload() {

	}



private:
	int size; // Number of chunks per side of the world
	bool wireFrame;
	Noise noise;
};
