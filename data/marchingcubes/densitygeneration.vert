#version 140
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

uniform vec3 a_offset;

uniform vec3 a_addingTerrainPositions[64];
uniform int a_addingTerrainPositionCount;
uniform vec3 a_removingTerrainPositions[64];
uniform int a_removingTerrainPositionCount;

uniform sampler3D noiseTexture1;
uniform sampler3D noiseTexture2;
uniform sampler3D noiseTexture3;
uniform sampler3D noiseTexture4;

uniform vec3 rotationVector1;
uniform vec3 rotationVector2;
uniform float warpFactor;
uniform float modificationRadius;

layout (location = 0) in vec3 in_position;

out float out_density;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

float additionalDensity(vec3 realPosition, vec3 influencePosition, float radius, float maxInfluence) {
    float dist = distance(realPosition, influencePosition);
    
    // Clamp influence to the radius, invert it
    float additionalDensity = radius - clamp(dist, 0, radius);
    
    // Scale to the maximum influence
    additionalDensity *= maxInfluence / radius;
    return additionalDensity;
}

void main()
{
    vec3 realPosition = in_position + a_offset;
    float xWarp = texture(noiseTexture1, realPosition * 0.21).r;
    float yWarp = texture(noiseTexture2, realPosition * 0.24).r;
    float zWarp = texture(noiseTexture3, realPosition * 0.18).r;
    
    vec3 warpedPosition = realPosition + vec3(xWarp, yWarp, zWarp) * warpFactor;
    
    mat4 rotate1 = rotationMatrix(rotationVector1, 3);
    mat4 rotate2 = rotationMatrix(rotationVector2, 5);
    vec3 rotatedPosition1 = (rotate1 * vec4(realPosition, 1.0)).xyz;
    vec3 rotatedPosition2 = (rotate2 * vec4(realPosition, 1.0)).xyz;
    
    out_density = -realPosition.y;
    out_density += -warpedPosition.y;
    out_density += texture(noiseTexture1, warpedPosition * 3.23).r * 0.3;
    out_density += texture(noiseTexture2, rotatedPosition2 * 1.8).r * 1.0;
    out_density += texture(noiseTexture3, rotatedPosition1 * 1.01).r * 1.5;
    
    
    // Terrain Modification
    
    // Maximum influence at the clicked position
    float maxInfluence = 0.5;
    
    for (int i = 0; i < a_addingTerrainPositionCount; i++) {
        out_density += additionalDensity(realPosition, a_addingTerrainPositions[i], modificationRadius, maxInfluence);
    }
    
    for (int i = 0; i < a_removingTerrainPositionCount; i++) {
        out_density -= additionalDensity(realPosition, a_removingTerrainPositions[i], modificationRadius, maxInfluence);
    }
}
