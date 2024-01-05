
// The `@location(0)` attribute means that this input variable is described
// by the vertex buffer layout at index 0 in the `pipelineDesc.vertex.buffers`
// array.
// The type `vec2f` must comply with what we will declare in the layout.
// The argument name `in_vertex_position` is up to you, it is only internal to
// the shader code!
@vertex
fn vs_main(@location(0) in_vertex_position: vec2f) -> @builtin(position) vec4f {
	return vec4f(in_vertex_position, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.1, 0.7, 1.0, 1.0);
}