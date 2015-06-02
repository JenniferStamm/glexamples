#version 140
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

uniform vec3 a_offset;

layout (location = 0) in vec3 in_position;

out float out_density;


void main()
{
    vec3 realPosition = in_position + a_offset;
	out_density = cos(realPosition.z * 5) / 2 + sin(realPosition.x * 8) / 3  - realPosition.y  + realPosition.x / 4 + 0.7;
}
