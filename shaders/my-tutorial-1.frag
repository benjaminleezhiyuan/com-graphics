#version 450 core

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 fFragColor;

void main()
{
    fFragColor = vec4(vColor, 1.0);
}