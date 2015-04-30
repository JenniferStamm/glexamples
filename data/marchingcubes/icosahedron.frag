#version 150 core

in vec3 v_normal;

uniform vec4 a_cubeColor;

out vec4 fragColor;

void main()
{
    fragColor = a_cubeColor;
}
