#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 1, binding = 0) uniform sampler2D flame_sampler;

layout (location = 0) in vec4 in_color;
layout (location = 1) in float in_alpha;
layout (location = 3) in float in_rotation;


layout (location = 0) out vec4 out_frag_color;

void main () 
{
	vec4 color;
	float alpha = (in_alpha <= 1.0) ? in_alpha : 2.0 - in_alpha;
	
	// Rotate texture coordinates
	// Rotate UV	
	float rotCenter = 0.5;
	float rotCos = cos(in_rotation);
	float rotSin = sin(in_rotation);
	vec2 rotUV = vec2(
		rotCos * (gl_PointCoord.x - rotCenter) + rotSin * (gl_PointCoord.y - rotCenter) + rotCenter,
		rotCos * (gl_PointCoord.y - rotCenter) - rotSin * (gl_PointCoord.x - rotCenter) + rotCenter);

	
    color = texture(flame_sampler, rotUV);
    out_frag_color.a = 0.0;
	
	
	out_frag_color.rgb = color.rgb * in_color.rgb * alpha;	
}
