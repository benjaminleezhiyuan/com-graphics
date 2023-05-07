/*!
@file    glapp.cpp
@author  pghali@digipen.edu
@date    10/11/2016

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <glapp.h>
#include <glhelper.h>

//variables for colour change
GLfloat randomColor1[4];
GLfloat randomColor2[4];
GLfloat interpolatedColor[4];
float interpolationFactor = 0.0f;
const float interpolationDuration = 2.0f; // time to interpolate in seconds

/*                                                   objects with file scope
----------------------------------------------------------------------------- */
void GLApp::init() {
	// Part 1: clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(0.f, 1.f, 0.f, 1.f);
	// Part 2: use entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);

	//generate random color values for the two colors
	randomColor1[0] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX); // random value between 0 and 1
	randomColor1[1] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
	randomColor1[2] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
	randomColor1[3] = 1.0f;

	randomColor2[0] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX); // random value between 0 and 1
	randomColor2[1] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
	randomColor2[2] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
	randomColor2[3] = 1.0f;
}

void GLApp::update() {
	interpolationFactor += 0.0167 / interpolationDuration;
	if (interpolationFactor > 1.0f) 
	{
		interpolationFactor = 0.0f;
		// Generate new random colors for the next interpolation
		randomColor1[0] = randomColor2[0];
		randomColor1[1] = randomColor2[1];
		randomColor1[2] = randomColor2[2];
		randomColor1[3] = randomColor2[3];

		randomColor2[0] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
		randomColor2[1] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
		randomColor2[2] = static_cast <GLfloat> (rand()) / static_cast <GLfloat> (RAND_MAX);
		randomColor2[3] = 1.0f;
	}
	interpolatedColor[0] = (1.0f - interpolationFactor) * randomColor1[0] + interpolationFactor * randomColor2[0];
	interpolatedColor[1] = (1.0f - interpolationFactor) * randomColor1[1] + interpolationFactor * randomColor2[1];
	interpolatedColor[2] = (1.0f - interpolationFactor) * randomColor1[2] + interpolationFactor * randomColor2[2];
	interpolatedColor[3] = (1.0f - interpolationFactor) * randomColor1[3] + interpolationFactor * randomColor2[3];

	// Clear the color buffer with the interpolated color
	glClearColor(interpolatedColor[0], interpolatedColor[1], interpolatedColor[2], interpolatedColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GLApp::draw() {
  // empty for now
}

void GLApp::cleanup() {
  // empty for now
}
