#pragma once

#include "../globals.h"
#include <glm/glm.hpp>
#include <map>
#include <set>


struct RegionOfInterest {
	glm::ivec2 center;

	/*
	 * Number of chunks visible in each direction from the center chunk.
	 * Ex. 3 = 7x7 grid.
	 * Total number of chunks = (2 * numVisibleChunks + 1)^2
	 */
	int distance = 0;
};

const auto posCmp2 = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
	if (a.first != b.first) {
		return a.first < b.first;
	}
	return a.second < b.second;
};

const auto posCmp = [](const glm::ivec2& a, const glm::ivec2& b) {
	if (a.x != b.x) {
		return a.x < b.x;
	}
	return a.y < b.y;
};

const auto roiCmp = [](const RegionOfInterest& a, const RegionOfInterest& b) {
	if (a.center != b.center) {
		return posCmp(a.center, b.center);
	}
	return a.distance < b.distance;
};

/**
 * Glorified state machine that manages the loading and unloading of chunks based on ROIs.
 *
 * Keeps an ongoing list of chunks that should be loaded and unloaded. Upon updating, the chunks to load and unload
 * update a list of chunks that should be currently loaded. The to load and to unload lists are then cleared for future
 * updates to ROIs.
 *
 * For a chunk to want to be loaded, this is defined as generating the chunk from scratch (i.e. generating the heightmap
 * and mesh).
 *
 * Main purpose is to really just be a dictionary look up for other systems, including the terrain renderer which will
 * generate buffers when the chunks to load are updated by the terrain.
 *
 */
class RoiManager {

private:
	std::set<glm::ivec2, decltype(posCmp)> chunksToLoad;
	std::set<glm::ivec2, decltype(posCmp)> chunksToUnload;
	std::set<glm::ivec2, decltype(posCmp)> loadedChunks;

	std::set<RegionOfInterest, decltype(roiCmp)> rois; // centers

public:

	~RoiManager() = default;

	/**
	 * @return set of chunks positions to load
	 */
	[[nodiscard]] const std::set<glm::ivec2, decltype(posCmp)>& getChunksToLoad() const {
		return chunksToLoad;
	}

	/**
	 * @return set of chunks positions to unload
	 */
	[[nodiscard]] const std::set<glm::ivec2, decltype(posCmp)>& getChunksToUnload() const {
		return chunksToUnload;
	}

	/**
	 * @return set of current ROIs
	 */
	[[nodiscard]] const std::set<RegionOfInterest, decltype(roiCmp)>& getPointOfInterests() const {
		return rois;
	}

	/**
	 * Clear the chunks to load and unload, and update the loaded chunks.
	 * ONLY CALL AFTER ACTUALLY LOADING/UNLOADING CHUNKS
	 */
	void updateLoadStates() {

		for (const auto& chunk : chunksToLoad) {
			loadedChunks.insert(chunk);
		}

		for (const auto& chunk : chunksToUnload) {
			loadedChunks.erase(chunk);
		}
		chunksToLoad.clear();
		chunksToUnload.clear();
	}

	/**
	 * Add ROI, and update the chunks to load or unload in the next update.
	 * @param pointOfInterest ROI to add
	 */
	void addPointOfInterest(const RegionOfInterest pointOfInterest) {
		rois.insert(pointOfInterest);

		// Load each chunk within the distance of the point of interest
		for (int row = -pointOfInterest.distance; row <= pointOfInterest.distance; row++) {
			for (int col = -pointOfInterest.distance; col <= pointOfInterest.distance; col++) {
				glm::ivec2 currentChunkPos = pointOfInterest.center + glm::ivec2{row, col};

				// Stop chunk from being unloaded if applicable
				if (chunksToUnload.contains(currentChunkPos)) {
					chunksToUnload.erase(currentChunkPos);
				}

				// If the chunk is already loaded, don't have to do anything
				if (loadedChunks.contains(currentChunkPos)) {
					break;
				}

				// Add chunk to load list if it is not already loaded
				chunksToLoad.insert(currentChunkPos);
			}
		}

	}

	/**
	 * Remove ROI, and update the chunks to load or unload in the next update.
	 * @param pointOfInterest ROI to remove
	 */
	void removePointOfInterest(const RegionOfInterest pointOfInterest) {

		if (rois.contains(pointOfInterest)) {
			rois.erase(pointOfInterest);
		}

		for (int row = -pointOfInterest.distance; row <= pointOfInterest.distance; row++) {
			for (int col = -pointOfInterest.distance; col <= pointOfInterest.distance; col++) {
				glm::ivec2 currentChunkPos = pointOfInterest.center + glm::ivec2{row, col};


				// Check if the chunk is within the distance of any other point of interest
				for (const auto&[center, distance] : rois) {
					if (glm::distance(glm::vec2(currentChunkPos), glm::vec2(center)) <= static_cast<float>(distance)) {
						break;
					}
				}

				// Chynk is isolated to this ROI, so check if it's loaded then add to unload list
				if (loadedChunks.contains(currentChunkPos)) {
					chunksToUnload.insert(currentChunkPos);
				}

				// *Might be redundant
				// This would remove the chunk from the load list if it was added by this ROI only, since
				// we know the chunk is only within range of this ROI
				if (chunksToLoad.contains(currentChunkPos)) {
					chunksToLoad.erase(currentChunkPos);
				}
			}
		}
	}

};

