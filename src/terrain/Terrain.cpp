#include "Terrain.h"

#include "../application.h"
#include "../shader.h"
#include "../world.h"


Terrain::Terrain(Noise::Descriptor noiseDesc) {


	terrainNoise = Noise(noiseDesc);
//	loadManager.addPointOfInterest(RegionOfInterest{center, numVisibleChunks});

//	chunk = Chunk(noise, center, chunkSize, wireFrame);
//	chunks.insert({center, chunk});


}

void Terrain::init(glm::ivec2 startChunk) {

	currentChunkPos = startChunk;

	loadManager.addPointOfInterest(RegionOfInterest(currentChunkPos, 1));


//	loadManager.updateChunkLists();
//
	for (auto& pos : loadManager.getChunksToLoad()) {
		chunks.insert({pos, Chunk(terrainNoise, pos)});
//		initChunkBuffers(chunks.at(pos));
//		initChunkUniforms(chunks.at(pos));
//		initChunkBindGroup(chunks.at(pos));
//		chunks.at(pos).mesh.getBuffers().valid = true;
	}

	notify(loadManager);
	loadManager.updateLoadStates();

}

// Updates the visible chunks list based on center position
void Terrain::update(glm::ivec2 newChunkPos) {
//	writeUniforms();
	if (newChunkPos != this->currentChunkPos) {
		loadManager.removePointOfInterest(RegionOfInterest(this->currentChunkPos, 0));
		loadManager.addPointOfInterest(RegionOfInterest(newChunkPos, 0));
		this->currentChunkPos = newChunkPos;


		for (auto& pos : loadManager.getChunksToUnload()) {
			chunks.erase({pos});
		}

		for (auto& pos : loadManager.getChunksToLoad()) {
			chunks.insert({pos, Chunk(terrainNoise, pos)});
		}

		notify(loadManager);

//		loadManager.removePointOfInterest(RegionOfInterest(this->center, numVisibleChunks));
//		loadManager.addPointOfInterest(RegionOfInterest(centerChunkPos, numVisibleChunks));
//
//		this->center = centerChunkPos;
//
//		loadManager.updateChunkLists();
//
//		// Unload old chunks
//		for (auto& pos : loadManager.chunksToUnload) {
//			chunks.erase(pos);
//			loadManager.chunksToUnload.erase(pos);
//		}
//
//		// Load new chunks
//		for (auto& pos : loadManager.chunksToLoad) {
//			chunks.insert({pos, Chunk(noise, pos, chunkSize, wireFrame)});
//			// Cant add to chunks to render until renderer creates buffers
//		}

	}

//	if (regenerate) {
//
//		for (auto& [pos, chunk] : chunks) {
//			loadManager.chunksToLoad.insert(pos);
//		}
//		chunks.clear();
//
//		for (auto& pos : loadManager.chunksToLoad) {
//			chunks.insert({pos, Chunk(noise, pos)});
//			initChunkBuffers(chunks.at(pos));
////			initChunkUniforms(chunks.at(pos));
////			initChunkBindGroup(chunks.at(pos));
//			chunks.at(pos).mesh.getBuffers().valid = true;
//			// Cant add to chunks to render until renderer creates buffers
//		}
//
//		loadManager.chunksToLoad.clear();
//		regenerate = false;
//	}




}

//void Terrain::setNoise(Noise::Descriptor noiseDesc) {
//	noise = Noise(noiseDesc);
//	regenerate = true;
//}



//void Terrain::render(wgpu::RenderPassEncoder &renderPass) {

//	if (wireFrame) {
//		renderPass.setPipeline(m_wireframePipeline);
//	}
//	else {
//		renderPass.setPipeline(m_pipeline);
//	}
//
//	for (auto& [key, chunk] : chunks) {
//		renderPass.setVertexBuffer(0, chunk.mesh.getBuffers().vertexBuffer, 0, chunk.mesh.getVertices().size() * sizeof(Vertex));
//		renderPass.setIndexBuffer(chunk.mesh.getBuffers().indexBuffer, wgpu::IndexFormat::Uint16, 0, chunk.mesh.getTriangles().size() * 3 * sizeof(uint16_t));
//		renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);
//		renderPass.drawIndexed(chunk.mesh.getTriangles().size() * 3, 1, 0, 0, 0);
//	}

//}
