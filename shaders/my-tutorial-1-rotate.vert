#version 450 core
layout (location=8) in vec2 aVertexPosition;
layout (location=9) in vec3 aVertexColor;

uniform mat3 rot_mtx;

layout (location=0) out vec3 vColor;

void main() {
vec3 tmp_vtx = rot_mtx * vec3(aVertexPosition, 1.0);
gl_Position = vec4(tmp_vtx.x,tmp_vtx.y, 0.0, 1.0);
vColor = aVertexColor;
}