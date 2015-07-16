#version 150 core


uniform mat4 transform;
uniform ivec3 a_dim;
uniform int a_margin;
uniform vec3 a_offset;

layout (std140) uniform edgeConnectList {
    ivec4 edgeConnectListEdges[1280];
};

uniform samplerBuffer densities;

uniform int[256] a_caseToNumPolys;
uniform ivec2[12] a_edgeToVertices;

layout(points) in;

layout(points, max_vertices = 15) out;

out vec4 out_position;
out vec3 out_normal;

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
            out_normal = mixedNormal;
            vec3 position = center + mix(vertexAPos, vertexBPos, mixing);
            vec3 scaledPosition = position / a_dim;
            
            ivec3 dir = ivec3(1, 1, 1);
            float occlusion = 1;
            for (int i = 1; i < 5; ++i) {
                vec3 checkPosition = vec3(position.x + dir.x * i, position.y + dir.y * i, position.z + dir.z * i);
                occlusion *= 1 - (0.3 * step(0, densityAtFloatPosition(checkPosition)));
            }
            
            out_position = vec4(scaledPosition, occlusion);
            EmitVertex();
            EndPrimitive();
        }
    }
}
