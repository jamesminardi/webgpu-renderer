

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

