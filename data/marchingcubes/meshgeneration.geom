#version 150 core


uniform mat4 transform;
uniform ivec3 a_dim;
uniform int a_margin;
uniform vec3 a_offset;

uniform samplerBuffer densities;

uniform int[256] a_caseToNumPolys;
uniform ivec2[12] a_edgeToVertices;

layout(points) in;

layout(points, max_vertices = 3) out;

in vec4 v_position[][3];
in vec3 v_normal[][3];

out vec4 out_position;
out vec3 out_normal;
  
void main() {
    out_position = v_position[0][0];
    out_normal = v_normal[0][0];
    EmitVertex();
    EndPrimitive();
    out_position = v_position[0][1];
    out_normal = v_normal[0][1];
    EmitVertex();
    EndPrimitive();
    out_position = v_position[0][2];
    out_normal = v_normal[0][2];
    EmitVertex();
    EndPrimitive();
}
