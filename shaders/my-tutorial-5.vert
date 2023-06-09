/**
 * @file 	my-tutorial-5.vert
 * @author  	benjaminzhiyuan.lee@digipen.edu
 * @date   	06/06/2023
 
 * Vertex shader for the GLSL program.
 *
 * This vertex shader processes the input vertex attributes for position, color, and texture coordinates.
 * It sets the position of the vertex using the aVertexPosition attribute and transforms it with the gl_Position
 * output variable. The vertex color is passed to the fragment shader through the vColor output variable.
 * Additionally, the texture coordinates are passed to the fragment shader through the vTexCoord output variable.
 * The texture coordinates are scaled by the textureScale uniform variable to control the texture mapping.
 *

*/

#version 450 core

layout (location = 0) in vec2 aVertexPosition;
layout (location = 1) in vec3 aVertexColor;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec3 vColor;
layout (location = 1) out vec2 vTexCoord;  // Add this line to declare the output variable


uniform float textureScale; // Add a uniform variable for texture scaling
void main() {
    gl_Position = vec4(aVertexPosition, 0.0, 1.0);
    vColor = aVertexColor;
    vTexCoord = aTexCoord * textureScale;  // Pass the texture coordinates as an output
}
