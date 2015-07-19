#version 140
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_include : require

uniform vec3 a_offset;

uniform sampler3D noiseTexture1;
uniform sampler3D noiseTexture2;
uniform sampler3D noiseTexture3;
uniform sampler3D noiseTexture4;

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


void main()
{
    vec3 realPosition = in_position + a_offset;
    float xWarp = texture(noiseTexture1, realPosition * 0.21).r;
    float yWarp = texture(noiseTexture2, realPosition * 0.24).r;
    float zWarp = texture(noiseTexture3, realPosition * 0.18).r;
    
    vec3 warpedPosition = realPosition + vec3(xWarp, yWarp, zWarp) * 3.4;
    
    mat4 rotate1 = rotationMatrix(vec3(1, 0.3, 0.1), 3);
    mat4 rotate2 = rotationMatrix(vec3(0.1, 0.5, 0.3), 5);
    vec3 rotatedPosition1 = (rotate1 * vec4(realPosition, 1.0)).xyz;
    vec3 rotatedPosition2 = (rotate2 * vec4(realPosition, 1.0)).xyz;
    
    out_density = -realPosition.y;
    out_density += -warpedPosition.y;
    out_density += texture(noiseTexture1, warpedPosition * 3.23).r * 0.3;
    out_density += texture(noiseTexture2, rotatedPosition2 * 1.8).r * 1.0;
    out_density += texture(noiseTexture1, rotatedPosition1 * 1.01).r * 1.5;
}
