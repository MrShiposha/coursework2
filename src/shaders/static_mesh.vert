#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in vec3 in_color;

layout (set = 0, binding = 0) uniform StaticUniformBuffer 
{
	mat4 projection;
	mat4 view;
	vec4 light_position;
} static_uniform;

layout (set = 0, binding = 1) uniform DynamicUniformBuffer
{
    mat4 model;
} dynamic_uniform;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec2 out_uv;
layout (location = 3) out vec3 out_view_vec;
layout (location = 4) out vec3 out_light_vec;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	out_normal = in_normal;
	out_color = in_color;
	out_uv = in_uv;

	mat4 modelView = static_uniform.view * dynamic_uniform.model;

	gl_Position = static_uniform.projection * modelView * vec4(in_position.xyz, 1.0);
	
	vec4 pos = modelView * vec4(in_position, 0.0);
	out_normal = mat3(dynamic_uniform.model) * in_normal;
	vec3 lPos = mat3(dynamic_uniform.model) * static_uniform.light_position.xyz;
	out_light_vec = lPos - (dynamic_uniform.model * vec4(in_position, 1.0)).xyz;
	out_view_vec = -(dynamic_uniform.model * vec4(in_position, 1.0)).xyz;		
}
