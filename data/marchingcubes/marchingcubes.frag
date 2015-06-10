#version 150 core

uniform vec4 a_cubeColor;

in vec3 v_normal;

out vec4 fragColor;

void main()
{
    fragColor = vec4(v_normal, 1.0);
}
