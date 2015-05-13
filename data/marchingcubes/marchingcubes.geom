#version 150 core

uniform sampler3D densities;

uniform mat4 transform;
uniform ivec3 a_dim;

layout (std140) uniform edgeConnectList {
    ivec4 edgeConnectListEdges[1280];
}
;

uniform int[256] a_caseToNumPolys;
uniform ivec2[12] a_edgeToVertices;

layout(points) in;

layout(triangle_strip, max_vertices = 15) out;

out vec3 g_normal;

const vec4 x  = vec4(0.5,0,0,0);
const vec4 y  = vec4(0,0.5,0,0);
const vec4 z  = vec4(0,0,0.5,0);

const vec4[8] vertices = vec4[8](
    -x - y - z,
    -x + y - z,
    x + y - z,
    x - y - z,
    -x - y + z,
    -x + y + z,
    x + y + z,
    x - y + z
    );
  
vec3 normalAtPosition(in vec3 position) {
    float d = 1.0 / (a_dim - 1);
    float diffX = texture(densities, position * d + vec3(d,0,0)).r - texture(densities, position * d + vec3(-d,0,0)).r;
    
    float diffY = texture(densities, position * d + vec3(0,d,0)).r - texture(densities, position * d + vec3(0,-d,0)).r;
    
    float diffZ = texture(densities, position * d + vec3(0,0,d)).r - texture(densities, position * d + vec3(0,0,-d)).r;
    return normalize(-vec3(diffX,diffY,diffZ));
    
}
  
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
    
    for (int i = 0; i < polygonCount; i++) {
        ivec4 edges = edgeConnectListEdges[index * 5 + i];
        
        for (int j = 0; j < 3; j++) {
            int edge = edges[j];
            int vertexA = a_edgeToVertices[edge].x;
            int vertexB = a_edgeToVertices[edge].y;
            vec4 vertexAPos = vertices[vertexA];
            vec4 vertexBPos = vertices[vertexB];
            float vertexADensity = vertexDensities[vertexA];
            float vertexBDensity = vertexDensities[vertexB];
            float mixing = vertexADensity / (vertexADensity - vertexBDensity);
            vec3 vertexANormal = normalAtPosition(center + vertexAPos.xyz);
            vec3 vertexBNormal = normalAtPosition(center + vertexBPos.xyz);
            vec3 mixedNormal = mix(vertexANormal,vertexBNormal,mixing);
            g_normal = mixedNormal;
            gl_Position = transform * (vec4(center,1.0) + mix(vertexAPos, vertexBPos, mixing));
            EmitVertex();
        }
        EndPrimitive();
    }
}
