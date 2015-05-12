#version 150 core

uniform sampler3D densities;

uniform mat4 transform;
uniform ivec3 a_dim;

uniform ivec3[1280] a_edgeConnectList;
uniform int[256] a_caseToNumPolys;
uniform ivec2[12] a_edgeToVertices;

layout(points) in;

layout(triangle_strip, max_vertices = 14) out;

out vec4 cubeColor;

const vec4 x  = vec4(0.5,0,0,0);
const vec4 y  = vec4(0,0.5,0,0);
const vec4 z  = vec4(0,0,0.5,0);

const vec4[8] vertices = vec4[8](
    -x - y + z,
    -x + y + z,
    x + y + z,
    x - y + z,
    -x - y - z,
    -x + y - z,
    x + y -z,
    x - y - z
    );
    


  
void main() {
    vec4 old = gl_in[0].gl_Position;
    vec3 center = old.xyz + 0.5;
    
    float[8] vertexDensities;
    for (int k = 0; k < vertices.length(); k++) {
        vertexDensities[k] = texture(densities, (center + (vertices[k]).xyz) / (a_dim - 1)).r;
    }
    
    int v0present =  int(vertexDensities[0] > 0);
    int v1present =  int(vertexDensities[1] > 0);
    int v2present =  int(vertexDensities[2] > 0);
    int v3present =  int(vertexDensities[3] > 0);
    int v4present =  int(vertexDensities[4] > 0);
    int v5present =  int(vertexDensities[5] > 0);
    int v6present =  int(vertexDensities[6] > 0);
    int v7present =  int(vertexDensities[7] > 0);
    
    int index = v0present * 1 + v1present * 2 + v2present * 4 + v3present * 8 + v4present * 16 + v5present * 32 + v6present * 64 + v7present * 128;
    
    int polygonCount = a_caseToNumPolys[index];
    cubeColor = vec4(vec3(float(polygonCount) / 5), 1.0);
    
    for (int i = 0; i < polygonCount; i++) {
        ivec3 edges = a_edgeConnectList[index * 5 + i];
        for (int j = 0; j < 3; j++) {
            int edge = edges[j];
            vec4 vertexA = vertices[a_edgeToVertices[edge].x];
            vec4 vertexB = vertices[a_edgeToVertices[edge].y];
            gl_Position = transform * ((vertexA + vertexB) / 2);
            EmitVertex();
        }
        EndPrimitive();
    }
}
