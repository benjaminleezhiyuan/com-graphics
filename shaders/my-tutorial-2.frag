/**
 * @file 	my-tutorial-2.frag
 * @author 	benjaminzhiyuan.lee@digipen.edu
 * @date	15/05/23
 * @brief 	Vertex shader source code for the application.
 * 
 *This shader receives the interpolated color from the vertex shader
 *and assigns it as the final color of the fragment.
 */
#version 450 core
layout (location=0) in vec3 vInterpColor;
layout (location=0) out vec4 fFragColor;
void main () {
fFragColor = vec4(vInterpColor, 1.0);
}