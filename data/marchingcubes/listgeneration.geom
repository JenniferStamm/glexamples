#version 150 core

uniform ivec3 a_dim;
uniform int a_margin;

layout (std140) uniform edgeConnectList {
    ivec4 edgeConnectListEdges[1280];
};

uniform samplerBuffer densities;

uniform bool removeFloaters;

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
  

const float dd = 0.7;
const vec3[26] directions = vec3[26](
    vec3(-dd, -dd, -dd),
    vec3(-dd, -dd, 0),
    vec3(-dd, -dd, dd),
    vec3(-dd, 0, -dd),
    vec3(-dd, 0, 0),
    vec3(-dd, 0, dd),
    vec3(-dd, dd, -dd),
    vec3(-dd, dd, 0),
    vec3(-dd, dd, dd),
    vec3(0, -dd, -dd),
    vec3(0, -dd, 0),
    vec3(0, -dd, dd),
    vec3(0, 0, -dd),
    //vec3(0, 0, 0),
    vec3(0, 0, dd),
    vec3(0, dd, -dd),
    vec3(0, dd, 0),
    vec3(0, dd, dd),
    vec3(dd, -dd, -dd),
    vec3(dd, -dd, 0),
    vec3(dd, -dd, dd),
    vec3(dd, 0, -dd),
    vec3(dd, 0, 0),
    vec3(dd, 0, dd),
    vec3(dd, dd, -dd),
    vec3(dd, dd, 0),
    vec3(dd, dd, dd));
  
      
  
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
  
float densityAtFloatPosition(in vec3 positionC) {
    // According to http://bmia.bmt.tue.nl/people/BRomeny/Courses/8C080/Interpolation.pdf
    ivec3 position000 = ivec3(floor(positionC.x), floor(positionC.y), floor(positionC.z));
    ivec3 position111 = position000 + ivec3(1, 1, 1);
    
    float densityValues[8];
    densityValues[0] = densityAtPosition(ivec3(position000));
    densityValues[1] = densityAtPosition(ivec3(position000.xy, position000.z + 1));
    densityValues[2] = densityAtPosition(ivec3(position000.x, position000.y + 1, position000.z));
    densityValues[3] = densityAtPosition(ivec3(position000.x, position000.y + 1, position000.z + 1));
    densityValues[4] = densityAtPosition(ivec3(position000.x + 1, position000.yz));
    densityValues[5] = densityAtPosition(ivec3(position000.x + 1, position000.y, position000.z + 1));
    densityValues[6] = densityAtPosition(ivec3(position000.x + 1, position000.y + 1, position000.z));
    densityValues[7] = densityAtPosition(ivec3(position000.x + 1, position000.y + 1, position000.z + 1));
    float normalizedVolumes[8];
    normalizedVolumes[0] =  (position111.x - positionC.x) * (position111.y - positionC.y) * (position111.z - positionC.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[1] =  (position111.x - positionC.x) * (position111.y - positionC.y) * (positionC.z - position000.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[2] =  (position111.x - positionC.x) * (positionC.y - position000.y) * (position111.z - positionC.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[3] =  (position111.x - positionC.x) * (positionC.y - position000.y) * (positionC.z - position000.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[4] =  (positionC.x - position000.x) * (position111.y - positionC.y) * (position111.z - positionC.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[5] =  (positionC.x - position000.x) * (position111.y - positionC.y) * (positionC.z - position000.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[6] =  (positionC.x - position000.x) * (positionC.y - position000.y) * (position111.z - positionC.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    normalizedVolumes[7] =  (positionC.x - position000.x) * (positionC.y - position000.y) * (positionC.z - position000.z) /
                            (position111.x - position000.x) * (position111.y - position000.y) * (position111.z - position000.z);
    
    
    
    float densityC = densityValues[0] * normalizedVolumes[0] +
                     densityValues[1] * normalizedVolumes[1] +
                     densityValues[2] * normalizedVolumes[2] +
                     densityValues[3] * normalizedVolumes[3] +
                     densityValues[4] * normalizedVolumes[4] +
                     densityValues[5] * normalizedVolumes[5] +
                     densityValues[6] * normalizedVolumes[6] +
                     densityValues[7] * normalizedVolumes[7];

    return densityC;
    //return densityAtPosition(ivec3(round(positionC.x), round(positionC.y), round(positionC.z)));
}

// All floaters smaller than this threshold are removed
const float floaterThreshold = 3;

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
    
    
    if (removeFloaters && polygonCount > 0) {
        float longestRay = 0;
        
        for (int dirCounter = 0; dirCounter < 26; ++dirCounter) {
            float stillIn = 1.0;
            for (int i = 1; i < 5; ++i) {
                vec3 dir = directions[dirCounter] * i;
                vec3 checkPosition = center + dir;
                float density = densityAtFloatPosition(checkPosition);
                stillIn = stillIn * step(0, density);
                longestRay = max(longestRay, length(dir) * stillIn);
            }
        }
        
        if (longestRay < floaterThreshold)
            return;
    }
    
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
