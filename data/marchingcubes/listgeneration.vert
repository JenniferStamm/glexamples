#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 1) in vec3 a_vertex;

void main()
{
	gl_Position = vec4(a_vertex, 1.0);
}
