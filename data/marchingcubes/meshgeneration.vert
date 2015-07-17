#version 150 core
#extension GL_ARB_explicit_attrib_location : require

layout(location = 1) in uint a_value;

out vec4 v_position[3];
out vec3 v_normal[3];

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


void main() {
    ivec3 voxel = ivec3((a_value >> 24) & 0x3F, (a_value >> 18) & 0x3F, (a_value >> 12 & 0x3F));
    
    ivec3 edges = ivec3((a_value >> 8) & 0x0F, (a_value >> 4) & 0x0F, a_value & 0x0F);
    
    
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
        
        ivec3 dir = ivec3(1, 1, 1);
        float occlusion = 1;
        for (int i = 1; i < 5; ++i) {
            ivec3 checkPosition = ivec3(round(position.x + dir.x * i), round(position.y + dir.y * i), round(position.z + dir.z * i));
            occlusion *= 1 - (0.3 * step(0, densityAtPosition(checkPosition)));
        }
        v_position[j] = vec4(scaledPosition, occlusion);
    }
    
}

