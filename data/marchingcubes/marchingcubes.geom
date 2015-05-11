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
const vec4 v1 = -x + y + z;
const vec4 v2 = x + y + z;
const vec4 v3 = x - y + z;
const vec4 v4 = -x - y - z;
const vec4 v5 = -x + y - z;
const vec4 v6 = x + y - z;
const vec4 v7 = x - y - z;

// 8 vertices per cube => 8 signs => 2^8 cases
const int case_to_numpolys[256] = int[256]
   (0, 1, 1, 2, 1, 2, 2, 3,  1, 2, 2, 3, 2, 3, 3, 2,  1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 2, 3, 4, 4, 3,  3, 4, 4, 3, 4, 5, 5, 2,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  
    2, 3, 3, 4, 3, 4, 2, 3,  3, 4, 4, 5, 4, 5, 3, 2,  3, 4, 4, 3, 4, 5, 3, 2,  4, 5, 5, 4, 5, 2, 4, 1,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 2, 4, 3, 4, 3, 5, 2,  
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  3, 4, 4, 3, 4, 5, 5, 4,  4, 3, 5, 2, 5, 4, 2, 1,  
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 2, 3, 3, 2,  3, 4, 4, 5, 4, 5, 5, 2,  4, 3, 5, 4, 3, 2, 4, 1,  
    3, 4, 4, 5, 4, 5, 3, 4,  4, 5, 5, 2, 3, 4, 2, 1,  2, 3, 3, 2, 3, 4, 2, 1,  3, 2, 4, 1, 2, 1, 1, 0);

// 5 entries per case: 256 * 5 = 1280
const ivec3 edge_connect_list[1280] = ivec3[1280]
   (ivec3(-1, -1, -1 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  3 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  1,  9 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  8,  3 ), ivec3(  9,  8,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 10 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  3 ), ivec3(  1,  2, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  2, 10 ), ivec3(  0,  2,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  8,  3 ), ivec3(  2, 10,  8), ivec3( 10,  9,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3, 11,  2 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0, 11,  2 ), ivec3(  8, 11,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  9,  0 ), ivec3(  2,  3, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1, 11,  2 ), ivec3(  1,  9, 11), ivec3(  9,  8, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3, 10,  1 ), ivec3( 11, 10,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0, 10,  1 ), ivec3(  0,  8, 10), ivec3(  8, 11, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  9,  0 ), ivec3(  3, 11,  9), ivec3( 11, 10,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  8, 10 ), ivec3( 10,  8, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  7,  8 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  3,  0 ), ivec3(  7,  3,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  1,  9 ), ivec3(  8,  4,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  1,  9 ), ivec3(  4,  7,  1), ivec3(  7,  3,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 10 ), ivec3(  8,  4,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  4,  7 ), ivec3(  3,  0,  4), ivec3(  1,  2, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  2, 10 ), ivec3(  9,  0,  2), ivec3(  8,  4,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2, 10,  9 ), ivec3(  2,  9,  7), ivec3(  2,  7,  3), ivec3(  7,  9,  4), ivec3( -1, -1, -1),
    ivec3( 8,  4,  7 ), ivec3(  3, 11,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(11,  4,  7 ), ivec3( 11,  2,  4), ivec3(  2,  0,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  0,  1 ), ivec3(  8,  4,  7), ivec3(  2,  3, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  7, 11 ), ivec3(  9,  4, 11), ivec3(  9, 11,  2), ivec3(  9,  2,  1), ivec3( -1, -1, -1),
    ivec3( 3, 10,  1 ), ivec3(  3, 11, 10), ivec3(  7,  8,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1, 11, 10 ), ivec3(  1,  4, 11), ivec3(  1,  0,  4), ivec3(  7, 11,  4), ivec3( -1, -1, -1),
    ivec3( 4,  7,  8 ), ivec3(  9,  0, 11), ivec3(  9, 11, 10), ivec3( 11,  0,  3), ivec3( -1, -1, -1),
    ivec3( 4,  7, 11 ), ivec3(  4, 11,  9), ivec3(  9, 11, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  5,  4 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  5,  4 ), ivec3(  0,  8,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  5,  4 ), ivec3(  1,  5,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  5,  4 ), ivec3(  8,  3,  5), ivec3(  3,  1,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 10 ), ivec3(  9,  5,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  0,  8 ), ivec3(  1,  2, 10), ivec3(  4,  9,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5,  2, 10 ), ivec3(  5,  4,  2), ivec3(  4,  0,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2, 10,  5 ), ivec3(  3,  2,  5), ivec3(  3,  5,  4), ivec3(  3,  4,  8), ivec3( -1, -1, -1),
    ivec3( 9,  5,  4 ), ivec3(  2,  3, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0, 11,  2 ), ivec3(  0,  8, 11), ivec3(  4,  9,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  5,  4 ), ivec3(  0,  1,  5), ivec3(  2,  3, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  1,  5 ), ivec3(  2,  5,  8), ivec3(  2,  8, 11), ivec3(  4,  8,  5), ivec3( -1, -1, -1),
    ivec3(10,  3, 11 ), ivec3( 10,  1,  3), ivec3(  9,  5,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  9,  5 ), ivec3(  0,  8,  1), ivec3(  8, 10,  1), ivec3(  8, 11, 10), ivec3( -1, -1, -1),
    ivec3( 5,  4,  0 ), ivec3(  5,  0, 11), ivec3(  5, 11, 10), ivec3( 11,  0,  3), ivec3( -1, -1, -1),
    ivec3( 5,  4,  8 ), ivec3(  5,  8, 10), ivec3( 10,  8, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  7,  8 ), ivec3(  5,  7,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  3,  0 ), ivec3(  9,  5,  3), ivec3(  5,  7,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  7,  8 ), ivec3(  0,  1,  7), ivec3(  1,  5,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  5,  3 ), ivec3(  3,  5,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  7,  8 ), ivec3(  9,  5,  7), ivec3( 10,  1,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  1,  2 ), ivec3(  9,  5,  0), ivec3(  5,  3,  0), ivec3(  5,  7,  3), ivec3( -1, -1, -1),
    ivec3( 8,  0,  2 ), ivec3(  8,  2,  5), ivec3(  8,  5,  7), ivec3( 10,  5,  2), ivec3( -1, -1, -1),
    ivec3( 2, 10,  5 ), ivec3(  2,  5,  3), ivec3(  3,  5,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 7,  9,  5 ), ivec3(  7,  8,  9), ivec3(  3, 11,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  5,  7 ), ivec3(  9,  7,  2), ivec3(  9,  2,  0), ivec3(  2,  7, 11), ivec3( -1, -1, -1),
    ivec3( 2,  3, 11 ), ivec3(  0,  1,  8), ivec3(  1,  7,  8), ivec3(  1,  5,  7), ivec3( -1, -1, -1),
    ivec3(11,  2,  1 ), ivec3( 11,  1,  7), ivec3(  7,  1,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  5,  8 ), ivec3(  8,  5,  7), ivec3( 10,  1,  3), ivec3( 10,  3, 11), ivec3( -1, -1, -1),
    ivec3( 5,  7,  0 ), ivec3(  5,  0,  9), ivec3(  7, 11,  0), ivec3(  1,  0, 10), ivec3( 11, 10,  0),
    ivec3(11, 10,  0 ), ivec3( 11,  0,  3), ivec3( 10,  5,  0), ivec3(  8,  0,  7), ivec3(  5,  7,  0),
    ivec3(11, 10,  5 ), ivec3(  7, 11,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  6,  5 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  3 ), ivec3(  5, 10,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  0,  1 ), ivec3(  5, 10,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  8,  3 ), ivec3(  1,  9,  8), ivec3(  5, 10,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  6,  5 ), ivec3(  2,  6,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  6,  5 ), ivec3(  1,  2,  6), ivec3(  3,  0,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  6,  5 ), ivec3(  9,  0,  6), ivec3(  0,  2,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5,  9,  8 ), ivec3(  5,  8,  2), ivec3(  5,  2,  6), ivec3(  3,  2,  8), ivec3( -1, -1, -1),
    ivec3( 2,  3, 11 ), ivec3( 10,  6,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(11,  0,  8 ), ivec3( 11,  2,  0), ivec3( 10,  6,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  1,  9 ), ivec3(  2,  3, 11), ivec3(  5, 10,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5, 10,  6 ), ivec3(  1,  9,  2), ivec3(  9, 11,  2), ivec3(  9,  8, 11), ivec3( -1, -1, -1),
    ivec3( 6,  3, 11 ), ivec3(  6,  5,  3), ivec3(  5,  1,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8, 11 ), ivec3(  0, 11,  5), ivec3(  0,  5,  1), ivec3(  5, 11,  6), ivec3( -1, -1, -1),
    ivec3( 3, 11,  6 ), ivec3(  0,  3,  6), ivec3(  0,  6,  5), ivec3(  0,  5,  9), ivec3( -1, -1, -1),
    ivec3( 6,  5,  9 ), ivec3(  6,  9, 11), ivec3( 11,  9,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5, 10,  6 ), ivec3(  4,  7,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  3,  0 ), ivec3(  4,  7,  3), ivec3(  6,  5, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  9,  0 ), ivec3(  5, 10,  6), ivec3(  8,  4,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  6,  5 ), ivec3(  1,  9,  7), ivec3(  1,  7,  3), ivec3(  7,  9,  4), ivec3( -1, -1, -1),
    ivec3( 6,  1,  2 ), ivec3(  6,  5,  1), ivec3(  4,  7,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2,  5 ), ivec3(  5,  2,  6), ivec3(  3,  0,  4), ivec3(  3,  4,  7), ivec3( -1, -1, -1),
    ivec3( 8,  4,  7 ), ivec3(  9,  0,  5), ivec3(  0,  6,  5), ivec3(  0,  2,  6), ivec3( -1, -1, -1),
    ivec3( 7,  3,  9 ), ivec3(  7,  9,  4), ivec3(  3,  2,  9), ivec3(  5,  9,  6), ivec3(  2,  6,  9),
    ivec3( 3, 11,  2 ), ivec3(  7,  8,  4), ivec3( 10,  6,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5, 10,  6 ), ivec3(  4,  7,  2), ivec3(  4,  2,  0), ivec3(  2,  7, 11), ivec3( -1, -1, -1),
    ivec3( 0,  1,  9 ), ivec3(  4,  7,  8), ivec3(  2,  3, 11), ivec3(  5, 10,  6), ivec3( -1, -1, -1),
    ivec3( 9,  2,  1 ), ivec3(  9, 11,  2), ivec3(  9,  4, 11), ivec3(  7, 11,  4), ivec3(  5, 10,  6),
    ivec3( 8,  4,  7 ), ivec3(  3, 11,  5), ivec3(  3,  5,  1), ivec3(  5, 11,  6), ivec3( -1, -1, -1),
    ivec3( 5,  1, 11 ), ivec3(  5, 11,  6), ivec3(  1,  0, 11), ivec3(  7, 11,  4), ivec3(  0,  4, 11),
    ivec3( 0,  5,  9 ), ivec3(  0,  6,  5), ivec3(  0,  3,  6), ivec3( 11,  6,  3), ivec3(  8,  4,  7),
    ivec3( 6,  5,  9 ), ivec3(  6,  9, 11), ivec3(  4,  7,  9), ivec3(  7, 11,  9), ivec3( -1, -1, -1),
    ivec3(10,  4,  9 ), ivec3(  6,  4, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4, 10,  6 ), ivec3(  4,  9, 10), ivec3(  0,  8,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  0,  1 ), ivec3( 10,  6,  0), ivec3(  6,  4,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  3,  1 ), ivec3(  8,  1,  6), ivec3(  8,  6,  4), ivec3(  6,  1, 10), ivec3( -1, -1, -1),
    ivec3( 1,  4,  9 ), ivec3(  1,  2,  4), ivec3(  2,  6,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  0,  8 ), ivec3(  1,  2,  9), ivec3(  2,  4,  9), ivec3(  2,  6,  4), ivec3( -1, -1, -1),
    ivec3( 0,  2,  4 ), ivec3(  4,  2,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  3,  2 ), ivec3(  8,  2,  4), ivec3(  4,  2,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  4,  9 ), ivec3( 10,  6,  4), ivec3( 11,  2,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  2 ), ivec3(  2,  8, 11), ivec3(  4,  9, 10), ivec3(  4, 10,  6), ivec3( -1, -1, -1),
    ivec3( 3, 11,  2 ), ivec3(  0,  1,  6), ivec3(  0,  6,  4), ivec3(  6,  1, 10), ivec3( -1, -1, -1),
    ivec3( 6,  4,  1 ), ivec3(  6,  1, 10), ivec3(  4,  8,  1), ivec3(  2,  1, 11), ivec3(  8, 11,  1),
    ivec3( 9,  6,  4 ), ivec3(  9,  3,  6), ivec3(  9,  1,  3), ivec3( 11,  6,  3), ivec3( -1, -1, -1),
    ivec3( 8, 11,  1 ), ivec3(  8,  1,  0), ivec3( 11,  6,  1), ivec3(  9,  1,  4), ivec3(  6,  4,  1),
    ivec3( 3, 11,  6 ), ivec3(  3,  6,  0), ivec3(  0,  6,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 6,  4,  8 ), ivec3( 11,  6,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 7, 10,  6 ), ivec3(  7,  8, 10), ivec3(  8,  9, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  7,  3 ), ivec3(  0, 10,  7), ivec3(  0,  9, 10), ivec3(  6,  7, 10), ivec3( -1, -1, -1),
    ivec3(10,  6,  7 ), ivec3(  1, 10,  7), ivec3(  1,  7,  8), ivec3(  1,  8,  0), ivec3( -1, -1, -1),
    ivec3(10,  6,  7 ), ivec3( 10,  7,  1), ivec3(  1,  7,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2,  6 ), ivec3(  1,  6,  8), ivec3(  1,  8,  9), ivec3(  8,  6,  7), ivec3( -1, -1, -1),
    ivec3( 2,  6,  9 ), ivec3(  2,  9,  1), ivec3(  6,  7,  9), ivec3(  0,  9,  3), ivec3(  7,  3,  9),
    ivec3( 7,  8,  0 ), ivec3(  7,  0,  6), ivec3(  6,  0,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 7,  3,  2 ), ivec3(  6,  7,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  3, 11 ), ivec3( 10,  6,  8), ivec3( 10,  8,  9), ivec3(  8,  6,  7), ivec3( -1, -1, -1),
    ivec3( 2,  0,  7 ), ivec3(  2,  7, 11), ivec3(  0,  9,  7), ivec3(  6,  7, 10), ivec3(  9, 10,  7),
    ivec3( 1,  8,  0 ), ivec3(  1,  7,  8), ivec3(  1, 10,  7), ivec3(  6,  7, 10), ivec3(  2,  3, 11),
    ivec3(11,  2,  1 ), ivec3( 11,  1,  7), ivec3( 10,  6,  1), ivec3(  6,  7,  1), ivec3( -1, -1, -1),
    ivec3( 8,  9,  6 ), ivec3(  8,  6,  7), ivec3(  9,  1,  6), ivec3( 11,  6,  3), ivec3(  1,  3,  6),
    ivec3( 0,  9,  1 ), ivec3( 11,  6,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 7,  8,  0 ), ivec3(  7,  0,  6), ivec3(  3, 11,  0), ivec3( 11,  6,  0), ivec3( -1, -1, -1),
    ivec3( 7, 11,  6 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 7,  6, 11 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  0,  8 ), ivec3( 11,  7,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  1,  9 ), ivec3( 11,  7,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  1,  9 ), ivec3(  8,  3,  1), ivec3( 11,  7,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  1,  2 ), ivec3(  6, 11,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 10 ), ivec3(  3,  0,  8), ivec3(  6, 11,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  9,  0 ), ivec3(  2, 10,  9), ivec3(  6, 11,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 6, 11,  7 ), ivec3(  2, 10,  3), ivec3( 10,  8,  3), ivec3( 10,  9,  8), ivec3( -1, -1, -1),
    ivec3( 7,  2,  3 ), ivec3(  6,  2,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 7,  0,  8 ), ivec3(  7,  6,  0), ivec3(  6,  2,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  7,  6 ), ivec3(  2,  3,  7), ivec3(  0,  1,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  6,  2 ), ivec3(  1,  8,  6), ivec3(  1,  9,  8), ivec3(  8,  7,  6), ivec3( -1, -1, -1),
    ivec3(10,  7,  6 ), ivec3( 10,  1,  7), ivec3(  1,  3,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  7,  6 ), ivec3(  1,  7, 10), ivec3(  1,  8,  7), ivec3(  1,  0,  8), ivec3( -1, -1, -1),
    ivec3( 0,  3,  7 ), ivec3(  0,  7, 10), ivec3(  0, 10,  9), ivec3(  6, 10,  7), ivec3( -1, -1, -1),
    ivec3( 7,  6, 10 ), ivec3(  7, 10,  8), ivec3(  8, 10,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 6,  8,  4 ), ivec3( 11,  8,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  6, 11 ), ivec3(  3,  0,  6), ivec3(  0,  4,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  6, 11 ), ivec3(  8,  4,  6), ivec3(  9,  0,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  4,  6 ), ivec3(  9,  6,  3), ivec3(  9,  3,  1), ivec3( 11,  3,  6), ivec3( -1, -1, -1),
    ivec3( 6,  8,  4 ), ivec3(  6, 11,  8), ivec3(  2, 10,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 10 ), ivec3(  3,  0, 11), ivec3(  0,  6, 11), ivec3(  0,  4,  6), ivec3( -1, -1, -1),
    ivec3( 4, 11,  8 ), ivec3(  4,  6, 11), ivec3(  0,  2,  9), ivec3(  2, 10,  9), ivec3( -1, -1, -1),
    ivec3(10,  9,  3 ), ivec3( 10,  3,  2), ivec3(  9,  4,  3), ivec3( 11,  3,  6), ivec3(  4,  6,  3),
    ivec3( 8,  2,  3 ), ivec3(  8,  4,  2), ivec3(  4,  6,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  4,  2 ), ivec3(  4,  6,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  9,  0 ), ivec3(  2,  3,  4), ivec3(  2,  4,  6), ivec3(  4,  3,  8), ivec3( -1, -1, -1),
    ivec3( 1,  9,  4 ), ivec3(  1,  4,  2), ivec3(  2,  4,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  1,  3 ), ivec3(  8,  6,  1), ivec3(  8,  4,  6), ivec3(  6, 10,  1), ivec3( -1, -1, -1),
    ivec3(10,  1,  0 ), ivec3( 10,  0,  6), ivec3(  6,  0,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  6,  3 ), ivec3(  4,  3,  8), ivec3(  6, 10,  3), ivec3(  0,  3,  9), ivec3( 10,  9,  3),
    ivec3(10,  9,  4 ), ivec3(  6, 10,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  9,  5 ), ivec3(  7,  6, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  3 ), ivec3(  4,  9,  5), ivec3( 11,  7,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5,  0,  1 ), ivec3(  5,  4,  0), ivec3(  7,  6, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(11,  7,  6 ), ivec3(  8,  3,  4), ivec3(  3,  5,  4), ivec3(  3,  1,  5), ivec3( -1, -1, -1),
    ivec3( 9,  5,  4 ), ivec3( 10,  1,  2), ivec3(  7,  6, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 6, 11,  7 ), ivec3(  1,  2, 10), ivec3(  0,  8,  3), ivec3(  4,  9,  5), ivec3( -1, -1, -1),
    ivec3( 7,  6, 11 ), ivec3(  5,  4, 10), ivec3(  4,  2, 10), ivec3(  4,  0,  2), ivec3( -1, -1, -1),
    ivec3( 3,  4,  8 ), ivec3(  3,  5,  4), ivec3(  3,  2,  5), ivec3( 10,  5,  2), ivec3( 11,  7,  6),
    ivec3( 7,  2,  3 ), ivec3(  7,  6,  2), ivec3(  5,  4,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  5,  4 ), ivec3(  0,  8,  6), ivec3(  0,  6,  2), ivec3(  6,  8,  7), ivec3( -1, -1, -1),
    ivec3( 3,  6,  2 ), ivec3(  3,  7,  6), ivec3(  1,  5,  0), ivec3(  5,  4,  0), ivec3( -1, -1, -1),
    ivec3( 6,  2,  8 ), ivec3(  6,  8,  7), ivec3(  2,  1,  8), ivec3(  4,  8,  5), ivec3(  1,  5,  8),
    ivec3( 9,  5,  4 ), ivec3( 10,  1,  6), ivec3(  1,  7,  6), ivec3(  1,  3,  7), ivec3( -1, -1, -1),
    ivec3( 1,  6, 10 ), ivec3(  1,  7,  6), ivec3(  1,  0,  7), ivec3(  8,  7,  0), ivec3(  9,  5,  4),
    ivec3( 4,  0, 10 ), ivec3(  4, 10,  5), ivec3(  0,  3, 10), ivec3(  6, 10,  7), ivec3(  3,  7, 10),
    ivec3( 7,  6, 10 ), ivec3(  7, 10,  8), ivec3(  5,  4, 10), ivec3(  4,  8, 10), ivec3( -1, -1, -1),
    ivec3( 6,  9,  5 ), ivec3(  6, 11,  9), ivec3( 11,  8,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  6, 11 ), ivec3(  0,  6,  3), ivec3(  0,  5,  6), ivec3(  0,  9,  5), ivec3( -1, -1, -1),
    ivec3( 0, 11,  8 ), ivec3(  0,  5, 11), ivec3(  0,  1,  5), ivec3(  5,  6, 11), ivec3( -1, -1, -1),
    ivec3( 6, 11,  3 ), ivec3(  6,  3,  5), ivec3(  5,  3,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 10 ), ivec3(  9,  5, 11), ivec3(  9, 11,  8), ivec3( 11,  5,  6), ivec3( -1, -1, -1),
    ivec3( 0, 11,  3 ), ivec3(  0,  6, 11), ivec3(  0,  9,  6), ivec3(  5,  6,  9), ivec3(  1,  2, 10),
    ivec3(11,  8,  5 ), ivec3( 11,  5,  6), ivec3(  8,  0,  5), ivec3( 10,  5,  2), ivec3(  0,  2,  5),
    ivec3( 6, 11,  3 ), ivec3(  6,  3,  5), ivec3(  2, 10,  3), ivec3( 10,  5,  3), ivec3( -1, -1, -1),
    ivec3( 5,  8,  9 ), ivec3(  5,  2,  8), ivec3(  5,  6,  2), ivec3(  3,  8,  2), ivec3( -1, -1, -1),
    ivec3( 9,  5,  6 ), ivec3(  9,  6,  0), ivec3(  0,  6,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  5,  8 ), ivec3(  1,  8,  0), ivec3(  5,  6,  8), ivec3(  3,  8,  2), ivec3(  6,  2,  8),
    ivec3( 1,  5,  6 ), ivec3(  2,  1,  6), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  3,  6 ), ivec3(  1,  6, 10), ivec3(  3,  8,  6), ivec3(  5,  6,  9), ivec3(  8,  9,  6),
    ivec3(10,  1,  0 ), ivec3( 10,  0,  6), ivec3(  9,  5,  0), ivec3(  5,  6,  0), ivec3( -1, -1, -1),
    ivec3( 0,  3,  8 ), ivec3(  5,  6, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  5,  6 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(11,  5, 10 ), ivec3(  7,  5, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(11,  5, 10 ), ivec3( 11,  7,  5), ivec3(  8,  3,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5, 11,  7 ), ivec3(  5, 10, 11), ivec3(  1,  9,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(10,  7,  5 ), ivec3( 10, 11,  7), ivec3(  9,  8,  1), ivec3(  8,  3,  1), ivec3( -1, -1, -1),
    ivec3(11,  1,  2 ), ivec3( 11,  7,  1), ivec3(  7,  5,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  3 ), ivec3(  1,  2,  7), ivec3(  1,  7,  5), ivec3(  7,  2, 11), ivec3( -1, -1, -1),
    ivec3( 9,  7,  5 ), ivec3(  9,  2,  7), ivec3(  9,  0,  2), ivec3(  2, 11,  7), ivec3( -1, -1, -1),
    ivec3( 7,  5,  2 ), ivec3(  7,  2, 11), ivec3(  5,  9,  2), ivec3(  3,  2,  8), ivec3(  9,  8,  2),
    ivec3( 2,  5, 10 ), ivec3(  2,  3,  5), ivec3(  3,  7,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  2,  0 ), ivec3(  8,  5,  2), ivec3(  8,  7,  5), ivec3( 10,  2,  5), ivec3( -1, -1, -1),
    ivec3( 9,  0,  1 ), ivec3(  5, 10,  3), ivec3(  5,  3,  7), ivec3(  3, 10,  2), ivec3( -1, -1, -1),
    ivec3( 9,  8,  2 ), ivec3(  9,  2,  1), ivec3(  8,  7,  2), ivec3( 10,  2,  5), ivec3(  7,  5,  2),
    ivec3( 1,  3,  5 ), ivec3(  3,  7,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  7 ), ivec3(  0,  7,  1), ivec3(  1,  7,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  0,  3 ), ivec3(  9,  3,  5), ivec3(  5,  3,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9,  8,  7 ), ivec3(  5,  9,  7), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5,  8,  4 ), ivec3(  5, 10,  8), ivec3( 10, 11,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 5,  0,  4 ), ivec3(  5, 11,  0), ivec3(  5, 10, 11), ivec3( 11,  3,  0), ivec3( -1, -1, -1),
    ivec3( 0,  1,  9 ), ivec3(  8,  4, 10), ivec3(  8, 10, 11), ivec3( 10,  4,  5), ivec3( -1, -1, -1),
    ivec3(10, 11,  4 ), ivec3( 10,  4,  5), ivec3( 11,  3,  4), ivec3(  9,  4,  1), ivec3(  3,  1,  4),
    ivec3( 2,  5,  1 ), ivec3(  2,  8,  5), ivec3(  2, 11,  8), ivec3(  4,  5,  8), ivec3( -1, -1, -1),
    ivec3( 0,  4, 11 ), ivec3(  0, 11,  3), ivec3(  4,  5, 11), ivec3(  2, 11,  1), ivec3(  5,  1, 11),
    ivec3( 0,  2,  5 ), ivec3(  0,  5,  9), ivec3(  2, 11,  5), ivec3(  4,  5,  8), ivec3( 11,  8,  5),
    ivec3( 9,  4,  5 ), ivec3(  2, 11,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  5, 10 ), ivec3(  3,  5,  2), ivec3(  3,  4,  5), ivec3(  3,  8,  4), ivec3( -1, -1, -1),
    ivec3( 5, 10,  2 ), ivec3(  5,  2,  4), ivec3(  4,  2,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3, 10,  2 ), ivec3(  3,  5, 10), ivec3(  3,  8,  5), ivec3(  4,  5,  8), ivec3(  0,  1,  9),
    ivec3( 5, 10,  2 ), ivec3(  5,  2,  4), ivec3(  1,  9,  2), ivec3(  9,  4,  2), ivec3( -1, -1, -1),
    ivec3( 8,  4,  5 ), ivec3(  8,  5,  3), ivec3(  3,  5,  1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  4,  5 ), ivec3(  1,  0,  5), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 8,  4,  5 ), ivec3(  8,  5,  3), ivec3(  9,  0,  5), ivec3(  0,  3,  5), ivec3( -1, -1, -1),
    ivec3( 9,  4,  5 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4, 11,  7 ), ivec3(  4,  9, 11), ivec3(  9, 10, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  8,  3 ), ivec3(  4,  9,  7), ivec3(  9, 11,  7), ivec3(  9, 10, 11), ivec3( -1, -1, -1),
    ivec3( 1, 10, 11 ), ivec3(  1, 11,  4), ivec3(  1,  4,  0), ivec3(  7,  4, 11), ivec3( -1, -1, -1),
    ivec3( 3,  1,  4 ), ivec3(  3,  4,  8), ivec3(  1, 10,  4), ivec3(  7,  4, 11), ivec3( 10, 11,  4),
    ivec3( 4, 11,  7 ), ivec3(  9, 11,  4), ivec3(  9,  2, 11), ivec3(  9,  1,  2), ivec3( -1, -1, -1),
    ivec3( 9,  7,  4 ), ivec3(  9, 11,  7), ivec3(  9,  1, 11), ivec3(  2, 11,  1), ivec3(  0,  8,  3),
    ivec3(11,  7,  4 ), ivec3( 11,  4,  2), ivec3(  2,  4,  0), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(11,  7,  4 ), ivec3( 11,  4,  2), ivec3(  8,  3,  4), ivec3(  3,  2,  4), ivec3( -1, -1, -1),
    ivec3( 2,  9, 10 ), ivec3(  2,  7,  9), ivec3(  2,  3,  7), ivec3(  7,  4,  9), ivec3( -1, -1, -1),
    ivec3( 9, 10,  7 ), ivec3(  9,  7,  4), ivec3( 10,  2,  7), ivec3(  8,  7,  0), ivec3(  2,  0,  7),
    ivec3( 3,  7, 10 ), ivec3(  3, 10,  2), ivec3(  7,  4, 10), ivec3(  1, 10,  0), ivec3(  4,  0, 10),
    ivec3( 1, 10,  2 ), ivec3(  8,  7,  4), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  9,  1 ), ivec3(  4,  1,  7), ivec3(  7,  1,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  9,  1 ), ivec3(  4,  1,  7), ivec3(  0,  8,  1), ivec3(  8,  7,  1), ivec3( -1, -1, -1),
    ivec3( 4,  0,  3 ), ivec3(  7,  4,  3), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 4,  8,  7 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9, 10,  8 ), ivec3( 10, 11,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  0,  9 ), ivec3(  3,  9, 11), ivec3( 11,  9, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  1, 10 ), ivec3(  0, 10,  8), ivec3(  8, 10, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  1, 10 ), ivec3( 11,  3, 10), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  2, 11 ), ivec3(  1, 11,  9), ivec3(  9, 11,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  0,  9 ), ivec3(  3,  9, 11), ivec3(  1,  2,  9), ivec3(  2, 11,  9), ivec3( -1, -1, -1),
    ivec3( 0,  2, 11 ), ivec3(  8,  0, 11), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 3,  2, 11 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  3,  8 ), ivec3(  2,  8, 10), ivec3( 10,  8,  9), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 9, 10,  2 ), ivec3(  0,  9,  2), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 2,  3,  8 ), ivec3(  2,  8, 10), ivec3(  0,  1,  8), ivec3(  1, 10,  8), ivec3( -1, -1, -1),
    ivec3( 1, 10,  2 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 1,  3,  8 ), ivec3(  9,  1,  8), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  9,  1 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3( 0,  3,  8 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1),
    ivec3(-1, -1, -1 ), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1), ivec3( -1, -1, -1));
  
void main() {
    vec4 old = gl_in[0].gl_Position;
    vec3 center = old.xyz + 0.5;
    float v0density = texture(densities, (center + v0.xyz) / (a_dim - 1)).r;
    float v1density = texture(densities, (center + v1.xyz) / (a_dim - 1)).r;
    float v2density = texture(densities, (center + v2.xyz) / (a_dim - 1)).r;
    float v3density = texture(densities, (center + v3.xyz) / (a_dim - 1)).r;
    float v4density = texture(densities, (center + v4.xyz) / (a_dim - 1)).r;
    float v5density = texture(densities, (center + v5.xyz) / (a_dim - 1)).r;
    float v6density = texture(densities, (center + v6.xyz) / (a_dim - 1)).r;
    float v7density = texture(densities, (center + v7.xyz) / (a_dim - 1)).r;
    
    int v0present =  int(v0density > 0);
    int v1present =  int(v1density > 0);
    int v2present =  int(v2density > 0);
    int v3present =  int(v3density > 0);
    int v4present =  int(v4density > 0);
    int v5present =  int(v5density > 0);
    int v6present =  int(v6density > 0);
    int v7present =  int(v7density > 0);
    
    int index = v0present * 1 + v1present * 2 + v2present * 4 + v3present * 8 + v4present * 16 + v5present * 32 + v6present * 64 + v7present * 128;
    
    if (index == 0)
        return;
        
    if (index == 255)
        return;
    
    int polygonCount = case_to_numpolys[index];
    
    
    cubeColor = vec4(vec3(float(polygonCount) / 5), 1.0);
    
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
