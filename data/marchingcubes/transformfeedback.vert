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
	//out_density = cos(realPosition.z * 5) / 2 + sin(realPosition.x * 8) / 3  - realPosition.y  + realPosition.x / 4 + 0.7;
    out_density = -realPosition.y + 1;
    out_density += texture(noiseTexture1, realPosition * 4.03).r * 0.1;
    out_density += texture(noiseTexture2, realPosition * 1.96).r * 0.2;
    out_density += texture(noiseTexture3, realPosition * 1.01).r * 0.4;
}
