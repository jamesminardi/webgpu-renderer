

### Init World
World -> create chunk

World -> Create terrain

World -> Create renderer

init camera stuff



### Load World (Right after init)

Init render pipeline
Init the chunk buffers, uniforms, bind group


### Render World

set pipeline


BE SURE TO NOT COPY BY VALUE CHUNKS OR OTHER DATA

### Random:

Want to make sure chunks (-1,-1) does not generate the same seed as (1,1) or that the terrain is the same across chunks. This is typically done via chunkseed = hash(hash(x)+hash(y)+seed) for example

### Noise could be:

Noise Interface (?)
-> Eval();
-> 

Class NoiseFunction
(FuncDesc)
-> Func: Linear, LinearCubic, Perlin, etc
-> Interpolation type
-> Eval(p)

Class FBM -> Extends Function
(fBmDesc, FuncDesc)
->->Interpolation type
-> Eval(p) { super.eval(p)  }


### Mesh

Mesh (Data)
-> Vertices
-> Indices (Triangles)

HeightMap (Data)
-> Heights

Chunk
-> mesh;
-> heightMap;
-> GenerateHeightMap()
-> 



## ChunkManager

Potential optimization would to do a resource management style where each chunk has a number of references that increment/decrement when ROIs are added/removed. So when a ROI is removed and its chunk refs are decremented, it doesn't have to search through all the ROIs and see if those chunks are referenced, but instead just check if the chunk ref is 0. This would save time complexity as the number of ROIs increase.

should the chunks be managed per chunk or per region? IM OVERTHINKING THIS LOL

Just do chunks.

Account for: Player going from one chunk to another and needing to load/unload chunks.
If you start removing chunks when the player moves, those chunks might still be needed for the next region the player is in.

So the actual chunk loading/unloading can't happen immediately, but should be per update. This works out considering chunk updates make sense to happen simultaneously at a known interval together, rather than independently somwhere in the player's update function.

So: Update the regions OR the state machine of chunks (we want to load or unload this chunk), but don't actually load or unload it yet until an update function.

That doesn't tell me whether to store chunks vs regions though.

If we were to do regions:
List of Regions:
On the addition of a new ROI:
1. Add all chunk positions to be loaded. Remove chunk positions that are in to Unload.
   1. This might not need to care about whether or not the chunks are actually loaded. So this resoucre could be used by a real manager of the chunk data without inner dependency.

Removal of an ROI:
1. Add all chunk positions to be removed
   1. If chunk positions are in To Load, move it to to unlood.

On Update:
1. Go through every chunk in to unload list and unload the chunks
2. Go through each chunk in the load list and load the chunk
3. If there is overlap between a chunk being in both lists, error of the add/remove ROI.


## Engine Notes:

### Game State (Screens/Scenes)

Let game states be "screens" that are stackable (i.e. screens can be drawn on top of each other). Screens can control whether or not screens below it are updatable, drawn, etc. Multiple screens can be active with self-contained logic and code specific to that screen without having to worry about what other screens might be up to.

Pause screen for example can be opened on top of the gameplay screen that disallows updates, but might still allow drawing. Inventory screen for example might let the game continue playing in the background.
Old XNA had an implementation here: https://github.com/tomizechsterson/game-state-management-monogame

Or try and look at monogame samples.

### Picking shaders

Interesting notes here on using a set of options to determine what shader to use:
https://www.gamedev.net/forums/topic/604899-frostbite-rendering-architecture-question/


### Renderer

Online notes:
- Resource system
  - Creates and loads meshes

Seems to be option between scenegraph with nodes versus rendering classes/subsystems?


Potential code smell where we can just update the data in the chunk buffers rather than constantly deleting and recreating it.
Since we use one set of vertex/index buffers per chunk, there's the risk of exploring so many chunks that the number of
buffers it too large when only a few are being rendered. (i.e. a chunk isn't loaded but its buffers still are). This would
require the use of a memory manager. Perhaps too much for this project scope