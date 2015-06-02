#version 150 core


uniform mat4 transform;
uniform ivec3 a_dim;
uniform vec3 a_offset;

layout (std140) uniform edgeConnectList {
    ivec4 edgeConnectListEdges[1280];
};

uniform samplerBuffer densities;

uniform int[256] a_caseToNumPolys;
uniform ivec2[12] a_edgeToVertices;

layout(points) in;

layout(triangle_strip, max_vertices = 15) out;

out vec3 g_normal;

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
    return position.z * (a_dim.x + 1) * (a_dim.y + 1) + position.y * (a_dim.x + 1) + position.x;
}

float densityAtPosition(in ivec3 position) {
    return densityAt(indexAtPosition(position));
}

vec3 normalAtPosition(in ivec3 position) {
    int d = 1;
    float diffX = densityAtPosition(position + ivec3(d,0,0)) - densityAtPosition(position + ivec3(-d,0,0));
    float diffY = densityAtPosition(position + ivec3(0,d,0)) - densityAtPosition(position + ivec3(0,-d,0));
    float diffZ = densityAtPosition(position + ivec3(0,0,d)) - densityAtPosition(position + ivec3(0,0,-d));
    
    return normalize(-vec3(diffX,diffY,diffZ));
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
    
    for (int i = 0; i < polygonCount; i++) {
        ivec4 edges = edgeConnectListEdges[index * 5 + i];
        
        for (int j = 0; j < 3; j++) {
            int edge = edges[j];
            int vertexA = a_edgeToVertices[edge].x;
            int vertexB = a_edgeToVertices[edge].y;
            vec3 vertexAPos = vertices[vertexA];
            vec3 vertexBPos = vertices[vertexB];
            float vertexADensity = vertexDensities[vertexA];
            float vertexBDensity = vertexDensities[vertexB];
            float mixing = vertexADensity / (vertexADensity - vertexBDensity);
            vec3 vertexANormal = normalAtPosition(ivec3(center + vertexAPos));
            vec3 vertexBNormal = normalAtPosition(ivec3(center + vertexBPos));
            vec3 mixedNormal = mix(vertexANormal,vertexBNormal,mixing);
            g_normal = mixedNormal;
            gl_Position = transform * (vec4((center + mix(vertexAPos, vertexBPos, mixing)) / a_dim + a_offset,1.0));
            EmitVertex();
        }
        EndPrimitive();
    }
}
