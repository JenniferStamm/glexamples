#version 150 core


uniform mat4 transform;

layout(points) in;

layout(triangle_strip, max_vertices = 14) out;

const vec4 x  = vec4(0.5,0,0,0);
const vec4 y  = vec4(0,0.5,0,0);
const vec4 z  = vec4(0,0,0.5,0);

const vec4 a = -x - y + z;
const vec4 b = x - y + z;
const vec4 c = -x + y + z;
const vec4 d = x + y + z;
const vec4 e = -x - y - z;
const vec4 f = x - y - z;
const vec4 g = -x + y - z;
const vec4 h = x + y - z;

void main() {
    vec4 old = gl_in[0].gl_Position;
    
    gl_Position = transform * (old + h);
    EmitVertex();
    gl_Position = transform * (old + g);
    EmitVertex();
    gl_Position = transform * (old + d);
    EmitVertex();
    gl_Position = transform * (old + c);
    EmitVertex();
    gl_Position = transform * (old + a);
    EmitVertex();
    gl_Position = transform * (old + g);
    EmitVertex();
    gl_Position = transform * (old + e);
    EmitVertex();
    gl_Position = transform * (old + h);
    EmitVertex();
    gl_Position = transform * (old + f);
    EmitVertex();
    gl_Position = transform * (old + d);
    EmitVertex();
    gl_Position = transform * (old + b);
    EmitVertex();
    gl_Position = transform * (old + a);
    EmitVertex();
    gl_Position = transform * (old + f);
    EmitVertex();
    gl_Position = transform * (old + e);
    EmitVertex();
    
    EndPrimitive();
}