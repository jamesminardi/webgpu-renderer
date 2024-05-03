#include "terrain.h"



 Terrain::Terrain(Noise::Descriptor noiseDesc, glm::ivec2 centerChunkPos, int numVisibleChunks, int chunkSize, bool wireFrame)
		: center(centerChunkPos), numVisibleChunks(numVisibleChunks), chunkSize(chunkSize), wireFrame(wireFrame) {

	noise = Noise(noiseDesc);


	loadManager.addPointOfInterest(PointOfInterest{center, numVisibleChunks});

//	// Generate chunk positions of the chunks surrounding the center chunk
//	for (int row = -numVisibleChunks; row <= numVisibleChunks; row++) {
//		for (int col = -numVisibleChunks; col <= numVisibleChunks; col++) {
//			glm::ivec2 worldPos = center + glm::ivec2{col, row};
//			chunks.insert(std::make_pair(key(worldPos), Chunk(noise, worldPos, chunkSize, wireFrame)));
//
//		}
//	}

	 loadManager.updateChunkLists();

	 for (auto& pos : loadManager.chunksToLoad) {
		 chunks.insert({pos, Chunk(noise, pos, chunkSize, wireFrame)});
		 // Cant add to chunks to render until renderer creates buffers
	 }


}

// Updates the visible chunks list based on center position
void Terrain::update(glm::ivec2 centerChunkPos) {

	if (centerChunkPos != this->center) {

		loadManager.removePointOfInterest(PointOfInterest(this->center, numVisibleChunks));
		loadManager.addPointOfInterest(PointOfInterest(centerChunkPos, numVisibleChunks));

		this->center = centerChunkPos;

		loadManager.updateChunkLists();

		// Unload old chunks
		for (auto& pos : loadManager.chunksToUnload) {
			chunks.erase(pos);
			loadManager.chunksToUnload.erase(pos);
		}

		// Load new chunks
		for (auto& pos : loadManager.chunksToLoad) {
			chunks.insert({pos, Chunk(noise, pos, chunkSize, wireFrame)});
			// Cant add to chunks to render until renderer creates buffers
		}

	}

	if (regenerate) {
		for (auto& [key, chunk] : chunks) {
			chunk = Chunk(noise, chunk.worldPos, chunkSize, wireFrame);
		}
		// Move all chunks in chunksToRender to chunksToLoad
		loadManager.chunksToLoad.insert(loadManager.chunksToRender.begin(), loadManager.chunksToRender.end());
		regenerate = false;
	}



//		// Unload chunks not in new view
//		for (auto it = chunks.begin(); it != chunks.end();) {
//			if (std::abs(it->second.worldPos.x - center.x) > numVisibleChunks ||
//				std::abs(it->second.worldPos.y - center.y) > numVisibleChunks) {
//				it = chunks.erase(it);
//			} else {
//				it++;
//			}
//		}
//
//		for (int row = -numVisibleChunks; row <= numVisibleChunks; row++) {
//			for (int col = -numVisibleChunks; col <= numVisibleChunks; col++) {
//				glm::ivec2 worldPos = center + glm::ivec2{col, row};
//
//				// Only load chunks that are not already loaded
//				if (chunks.find(key(worldPos)) == chunks.end()) {
//					chunks.insert(std::make_pair(key(worldPos), Chunk(noise, worldPos, chunkSize, wireFrame)));
//				}
//			}
//		}


}

void Terrain::setNoise(Noise::Descriptor noiseDesc) {
	noise = Noise(noiseDesc);
	regenerate = true;
}

void Terrain::setWireFrame(bool wire) {
	if (wire == wireFrame) return;
	wireFrame = wire;
	regenerate = true;
}

bool Terrain::isWireFrame() {
	return wireFrame;
}

void Terrain::draw() {

}
