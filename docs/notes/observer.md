# Observer Pattern

In hopes to decouple a lot of the rendering code from the application, I refactored code such as the terrain and chunk classes to have a separate rendering system. This way, the chunk generation code, for example, doesn't care about any wgpu objects like buffers.

However, for the TerrainRenderer class to successfully render the terrain, it needs to create the proper buffers for all the chunks in the world. In order to not create tight dependencies between these classes, I opted for an observer interface. This allows for the terrain renderer to "subscribe" itself to the terrain class and complete actions when the terrain performs some action of interest. In this case, the terrain renderer needs to update its buffer data with chunks that have been newly generated in order to render it to the screen.

See src/terrain/Observer.h to see how I did it in this project.


Below are random resources that I'm putting down for future reference. I could've went with a basic observer implementation since the systems are rather simple right now, but I wanted to get a better understanding of how it can be improved or extended, especially in C++. Thus, I use templates, and might use function pointers, std::function, or other methods mentioned in the resources below:


Basics, and Push vs Pull
https://stackoverflow.com/questions/34706186/push-pull-mechanism-observer-pattern

Discusses basics, push vs pull, overriding handler vs arbitrary, etc
https://stackoverflow.com/questions/14633808/the-observer-pattern-further-considerations-and-generalised-c-implementation