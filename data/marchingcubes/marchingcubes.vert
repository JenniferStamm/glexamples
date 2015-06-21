#version 150 core
#extension GL_ARB_explicit_attrib_location : require

uniform mat4 transform;

uniform vec3 a_offset;

layout(location = 0) in vec4 a_vertex;
layout(location = 1) in vec3 a_normal;

out vec3 v_position;
out float v_occlusion;
out vec3 v_normal;

void main()
{
	v_position = a_vertex.xyz + a_offset;
    v_occlusion = a_vertex.a;
    v_normal = a_normal;
    gl_Position = transform * vec4(v_position, 1.0);
}
