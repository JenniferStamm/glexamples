#version 150 core
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 transform;

uniform vec3 a_offset;

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_normal;

void main()
{
	gl_Position = transform * vec4(a_vertex + a_offset, 1.0);
    v_normal = a_normal;
}
