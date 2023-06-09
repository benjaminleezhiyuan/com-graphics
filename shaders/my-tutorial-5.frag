
/**
 * @file 	my-tutorial-5.frag
 * @author  	benjaminzhiyuan.lee@digipen.edu
 * @date   	06/06/2023

 * Fragment shader for the GLSL program.
 *
 * This fragment shader handles the per-fragment color attribute supplied by the application.
 * The resulting fragment color is assigned to fFragColor as a vec4 with the assigned color value
 * and an alpha value of 1.0. The shader also supports texture mapping and modulation based on the
 * modulation flag.

*/
 
 
#version 450 core
layout(location = 0) in vec3 vColor;
layout(location = 0) out vec4 fFragColor;
layout(location = 1) in vec2 vTexCoord;

uniform sampler2D uTex2d; //For texture mapping
uniform float size; // Size of each tile
uniform bool modulation; // Toggle for modulation

uniform int task; //Task cycle

// Convert fragment coordinates to tile coordinates
ivec2 tileCoord = ivec2(gl_FragCoord.xy / size) % 2;

// Determine the color of the current tile based on its coordinates
int c = (tileCoord.x + tileCoord.y) % 2;

// Define the two checkerboard colors
vec3 color0 = vec3(1.0, 0.0, 1.0);
vec3 color1 = vec3(0.0, 0.68, 0.94);
void main() {
fFragColor = vec4(vColor, 1.0);
  if (task == 0) {
    fFragColor = vec4(vColor, 1.0);
  } 
  else if (task == 1 || task == 2) {

    if (modulation && task != 0) {
      // Modulate the two colors component-wise
      vec3 finalColor = (c == 0) ? color0 : color1;
      fFragColor = vec4(finalColor * vColor, 1.0);
    } else {
      // Assign colors based on the checkerboard pattern
      vec3 finalColor = (c == 0) ? color0 : color1;
      fFragColor = vec4(finalColor, 1.0);
    }
  } 
  else if (task == 3 || task == 4 || task == 5 || task == 6) {
    vec4 textureColor = texture(uTex2d, vTexCoord);

    if (modulation) {
      // Modulate the texture color with the vertex color component-wise
      vec3 finalColor = textureColor.rgb * vColor.rgb;
      fFragColor = vec4(finalColor, textureColor.a);
    } else {
      fFragColor = textureColor;
    }
  } 
}