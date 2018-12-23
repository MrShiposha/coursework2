#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in float in_alpha;
layout (location = 3) in float in_size;
layout (location = 4) in float in_rotation;

layout (location = 0) out vec4 out_color;
layout (location = 1) out float out_alpha;
layout (location = 3) out float out_rotation;

layout (set = 0, binding = 0) uniform StaticUniformBuffer 
{
	mat4 projection;
	mat4 view;
	vec4 light_position;
    vec2 viewport_dimension;
} static_uniform;

layout (set = 0, binding = 1) uniform DynamicUniformBuffer
{
    mat4 model;
} dynamic_uniform;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main () 
{
	out_color = in_color;
	out_alpha = in_alpha;
	out_rotation = in_rotation;
	  
	gl_Position = static_uniform.projection * static_uniform.view * dynamic_uniform.model * vec4(in_position.xyz, 1.0);	
	
	// Base size of the point sprites
	float spriteSize = 8.0 * in_size;

	// Scale particle size depending on camera projection
	vec4 eyePos = static_uniform.view * dynamic_uniform.model * vec4(in_position.xyz, 1.0);
	vec4 projectedCorner = static_uniform.projection * vec4(0.025 * spriteSize, 0.025 * spriteSize, eyePos.z, eyePos.w);
	gl_PointSize = static_uniform.viewport_dimension.x * projectedCorner.x / projectedCorner.w;
	
}