/**
 * @file 	my-tutorial-3.frag
 * @author 	benjaminzhiyuan.lee@digipen.edu
 * @date	26/05/23
 * @brief 	Vertex shader source code for the application.
 * 
 *This shader receives the interpolated color from the vertex shader
 *and assigns it as the final color of the fragment.
 */
#version 450 core
/*
A per-fragment color attribute is no longer received from rasterizer.
Instead per-fragment color attribute is supplied by the application to
a uniform variable:
uniform vec3 uColor;
The uniform variable will retain the value for every invocation of the
fragment shader. That is why every fragment of the triangle primitive
rendered by an object has the same color
*/
//layout(location=0) in vec3 vColor;
uniform vec3 uColor;
layout (location=0) out vec4 fFragColor;
void main() {
//fFragColor = vec4(vColor, 1.0);
fFragColor = vec4(uColor, 1.0);
}
