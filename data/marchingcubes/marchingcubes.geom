#version 150 core

uniform sampler3D densities;

uniform mat4 transform;
uniform ivec3 a_dim;

layout(points) in;

layout(triangle_strip, max_vertices = 14) out;

out vec4 cubeColor;

const vec4 x  = vec4(0.5,0,0,0);
const vec4 y  = vec4(0,0.5,0,0);
const vec4 z  = vec4(0,0,0.5,0);

const vec4 v0 = -x - y + z;
const vec4 v3 = x - y + z;
const vec4 v1 = -x + y + z;
const vec4 v2 = x + y + z;
const vec4 v4 = -x - y - z;
const vec4 v7 = x - y - z;
const vec4 v5 = -x + y - z;
const vec4 v6 = x + y - z;

void main() {
    vec4 old = gl_in[0].gl_Position;
    float density = texture(densities, old.xyz / (a_dim - 1)).r;
    
    if (density > 0)
        return;
    
    cubeColor = vec4(vec3(old.xyz / (a_dim - 1)), 1.0);
    
    gl_Position = transform * (old + v6);
    EmitVertex();
    gl_Position = transform * (old + v5);
    EmitVertex();
    gl_Position = transform * (old + v2);
    EmitVertex();
    gl_Position = transform * (old + v1);
    EmitVertex();
    gl_Position = transform * (old + v0);
    EmitVertex();
    gl_Position = transform * (old + v5);
    EmitVertex();
    gl_Position = transform * (old + v4);
    EmitVertex();
    gl_Position = transform * (old + v6);
    EmitVertex();
    gl_Position = transform * (old + v7);
    EmitVertex();
    gl_Position = transform * (old + v2);
    EmitVertex();
    gl_Position = transform * (old + v3);
    EmitVertex();
    gl_Position = transform * (old + v0);
    EmitVertex();
    gl_Position = transform * (old + v7);
    EmitVertex();
    gl_Position = transform * (old + v4);
    EmitVertex();
    
    EndPrimitive();
}
