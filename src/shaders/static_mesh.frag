#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 1, binding = 0) uniform sampler2D sampler_color_map;

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_uv;
layout (location = 3) in vec3 in_view_vec;
layout (location = 4) in vec3 in_light_vec;

layout(push_constant) uniform Material 
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float opacity;
} material;

layout (location = 0) out vec4 out_frag_color;

void main() 
{
	vec4 color = texture(sampler_color_map, in_uv) * vec4(in_color, 1.0);
	vec3 N = normalize(in_normal);
	vec3 L = normalize(in_light_vec);
	vec3 V = normalize(in_view_vec);
	vec3 R = reflect(-L, N);
	vec3 falme_color = vec3(226.f, 88.f, 34.f) * 0.05;
	vec3 diffuse = max(dot(N, L), 0.0) * material.diffuse.rgb;
	vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * material.specular.rgb * falme_color;
	out_frag_color = vec4((material.ambient.rgb + diffuse) * color.rgb + specular, 1.f/*-material.opacity*/);
}
