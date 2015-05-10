#version 150 core

uniform vec4 a_cubeColor;

in vec4 cubeColor;

out vec4 fragColor;

void main()
{
    fragColor = cubeColor;
}
