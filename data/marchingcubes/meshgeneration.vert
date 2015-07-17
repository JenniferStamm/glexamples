#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 1) in uint a_value;

out vec4 v_position0;
out vec4 v_position1;
out vec4 v_position2;
out vec3 v_normal0;
out vec3 v_normal1;
out vec3 v_normal2;

uniform ivec3 a_dim;
uniform int a_margin;
uniform vec3 a_offset;

uniform samplerBuffer densities;

uniform ivec2[12] a_edgeToVertices;

const ivec3[8] vertices = ivec3[8](
    ivec3(0,0,0),
    ivec3(0,1,0),
    ivec3(1,1,0),
    ivec3(1,0,0),
    ivec3(0,0,1),
    ivec3(0,1,1),
    ivec3(1,1,1),
    ivec3(1,0,1)
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

vec3 normalAtPosition(in ivec3 position) {
    int d = 1;
    float diffX = densityAtPosition(position + ivec3(d,0,0)) - densityAtPosition(position + ivec3(-d,0,0));
    float diffY = densityAtPosition(position + ivec3(0,d,0)) - densityAtPosition(position + ivec3(0,-d,0));
    float diffZ = densityAtPosition(position + ivec3(0,0,d)) - densityAtPosition(position + ivec3(0,0,-d));
    
    return normalize(-vec3(diffX,diffY,diffZ));
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


void main() {
    ivec3 voxel = ivec3((a_value >> 24) & 0x3F, (a_value >> 18) & 0x3F, (a_value >> 12 & 0x3F));
    
    ivec3 edges = ivec3((a_value >> 8) & 0x0F, (a_value >> 4) & 0x0F, a_value & 0x0F);
    
    vec4 v_position[3];
    vec3 v_normal[3];
    
    for (int j = 0; j < 3; j++) {
        int edge = edges[j];
        int vertexA = a_edgeToVertices[edge].x;
        int vertexB = a_edgeToVertices[edge].y;
        ivec3 vertexAPos = voxel + vertices[vertexA];
        ivec3 vertexBPos = voxel + vertices[vertexB];
        float vertexADensity = densityAtPosition(vertexAPos);
        float vertexBDensity = densityAtPosition(vertexBPos);
        float mixing = vertexADensity / (vertexADensity - vertexBDensity);
        vec3 vertexANormal = normalAtPosition(vertexAPos);
        vec3 vertexBNormal = normalAtPosition(vertexBPos);
        vec3 mixedNormal = mix(vertexANormal,vertexBNormal,mixing);
        v_normal[j] = mixedNormal;
        vec3 position = mix(vec3(vertexAPos), vec3(vertexBPos), mixing);
        vec3 scaledPosition = position / a_dim;
        
        float occlusion = 0;
        for (int dirCounter = 0; dirCounter < 26; ++dirCounter) {
            float dirVisibility = 1;
            for (int i = 1; i < 5; ++i) {
                vec3 checkPosition = position + directions[dirCounter] * i;
                dirVisibility *= clamp(densityAtFloatPosition(checkPosition) * 9999, 0, 1);
            }
            occlusion += dirVisibility;
        }
        occlusion /= 26;
        
        v_position[j] = vec4(scaledPosition,  1 - occlusion);
    }

    v_position0 = v_position[0];
    v_position1 = v_position[1];
    v_position2 = v_position[2];
    
    v_normal0 = v_normal[0];
    v_normal1 = v_normal[1];
    v_normal2 = v_normal[2];
}

