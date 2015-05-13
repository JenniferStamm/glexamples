#version 150 core

uniform vec4 a_cubeColor;

in vec3 g_normal;

out vec4 fragColor;

void main()
{
    fragColor = vec4(g_normal, 1.0);
}
