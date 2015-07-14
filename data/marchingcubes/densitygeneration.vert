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


void main()
{
    vec3 realPosition = in_position + a_offset;
    float xWarp = texture(noiseTexture1, realPosition * 0.04).r;
    float yWarp = texture(noiseTexture2, realPosition * 0.04).r;
    float zWarp = texture(noiseTexture3, realPosition * 0.04).r;
    
    realPosition += vec3(xWarp, yWarp, zWarp) * 4;
    
    out_density = -realPosition.y;
    out_density += texture(noiseTexture1, realPosition * 4.03).r * 0.1;
    out_density += texture(noiseTexture2, realPosition * 1.96).r * 0.2;
    out_density += texture(noiseTexture3, realPosition * 1.01).r * 0.4;
}
