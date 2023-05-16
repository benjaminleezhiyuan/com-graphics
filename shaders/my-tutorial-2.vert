/**
 * @file 	my-tutorial-2.vert
 * @author 	benjaminzhiyuan.lee@digipen.edu
 * @date	15/05/23
 * @brief 	Vertex shader source code for the application.
 * 
 * This shader takes the vertex position and color as input attributes
 * and passes the color to the fragment shader as a varying variable.
 */
#version 450 core
layout (location=0) in vec2 aVertexPosition;
layout (location=1) in vec3 aVertexColor;
layout (location=0) out vec3 vColor;
void main() {
gl_Position = vec4(aVertexPosition, 0.0, 1.0);
vColor = aVertexColor;
}