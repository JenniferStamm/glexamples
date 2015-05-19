#version 140
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require


layout (location = 0) in vec3 in_position;

out float out_density;


void main()
{
	out_density = 1.0;
}
