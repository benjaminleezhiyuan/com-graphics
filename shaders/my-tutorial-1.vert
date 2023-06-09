#version 450 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexColor;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vTexCoord;

void main()
{
    gl_Position = vec4(aVertexPosition, 1.0);
    vColor = aVertexColor;
    vTexCoord = aTexCoord;
}