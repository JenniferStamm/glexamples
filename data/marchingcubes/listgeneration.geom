#version 150 core

uniform ivec3 a_dim;
uniform int a_margin;

layout (std140) uniform edgeConnectList {
    ivec4 edgeConnectListEdges[1280];
};

uniform samplerBuffer densities;

uniform int[256] a_caseToNumPolys;

layout(points) in;

layout(points, max_vertices = 5) out;

out uint out_value;

const vec3 x  = vec3(0.5,0,0);
const vec3 y  = vec3(0,0.5,0);
const vec3 z  = vec3(0,0,0.5);

const vec3[8] vertices = vec3[8](
    -x - y - z,
    -x + y - z,
    x + y - z,
    x - y - z,
    -x - y + z,
    -x + y + z,
    x + y + z,
    x - y + z
    );
  
float densityAt(in int index) {
    return texelFetch(densities, index).r;
}

int indexAtPosition(in ivec3 position) {
    ivec3 densityDims = a_dim + 2 * a_margin + 1;
    int densitySize = densityDims.x * densityDims.y * densityDims.z;

    return clamp((position.z + a_margin) * (a_dim.x + 2 * a_margin + 1) * (a_dim.y + 2 * a_margin + 1) + (position.y + a_margin) * (a_dim.x + 2 * a_margin + 1) + (position.x + a_margin), 0, densitySize - 1);
}

float densityAtPosition(in ivec3 position) {
    return densityAt(indexAtPosition(position));
}
  
void main() {
    vec4 old = gl_in[0].gl_Position;
    vec3 center = old.xyz * a_dim + 0.5;
    
    float[8] vertexDensities;
    for (int k = 0; k < vertices.length(); k++) {
        ivec3 pos = ivec3(center + vertices[k]);
        vertexDensities[k] = densityAtPosition(pos); 
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
    
    uint result = 0u;
        
    ivec3 iPos = ivec3(center);
    result += uint(iPos.x) << 24;
    result += uint(iPos.y) << 18;
    result += uint(iPos.z) << 12;
    
    for (int i = 0; i < polygonCount; i++) {
        ivec4 edges = edgeConnectListEdges[index * 5 + i];
        
        out_value = result;
        out_value += uint(edges.x) << 8;
        out_value += uint(edges.y) << 4;
        out_value += uint(edges.z) << 0;
        
            
        EmitVertex();
        EndPrimitive();
    }
}
