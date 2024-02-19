
// The `@location(0)` attribute means that this input variable is described
// by the vertex buffer layout at index 0 in the `pipelineDesc.vertex.buffers`
// array.
// The type `vec2f` must comply with what we will declare in the layout.
// The argument name `in_vertex_position` is up to you, it is only internal to
// the shader code!

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) color: vec3f,
}


// Number of Components forwarded to fragment shader is three float components.
// Pos is handled automatically, but color is not because the location is a number value.
struct VertexOutput {
    @builtin(position) position: vec4f,

    // The location here does not refer to a vertex attribute, it just means
    // that this field must be handled by the rasterizer.
    // (It can also refer to another field of another struct that would be used
    // as input to the fragment shader.)
    @location(0) color: vec3f
}

struct ShaderUniforms {
    projectionMatrix: mat4x4f,
    viewMatrix: mat4x4f,
    modelMatrix: mat4x4f,
    color: vec4f,
    time: f32,
};

// Instead of the simple uTime variable, our uniform variable is a struct
@group(0) @binding(0) var<uniform> uShaderUniforms: ShaderUniforms;



@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	out.position = uShaderUniforms.projectionMatrix * uShaderUniforms.viewMatrix * uShaderUniforms.modelMatrix * vec4f(in.position.xyz,  1.0);
	out.color = in.color * in.position.y;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {

    // Gamma Correction for WGPU, Dawn is fine without it.
//	let color = in.color;
//	// Gamma-correction
//	let corrected_color = pow(color, vec3f(2.2));
//	return vec4f(corrected_color, uShaderUniforms.color.a);
	return vec4f(in.color, 1.0);
}