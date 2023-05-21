/*!
@file    glapp.cpp
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		15/05/2023
@ammendent	Added function definitions for points_model, lines_model,trifans_model and
			

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <vector>
#include <glapp.h>
#include <glhelper.h>
#include <array>

// define singleton containers
std::vector<GLSLShader> GLApp::shdrpgms;
std::vector<GLApp::GLModel> GLApp::models;
std::list<GLApp::GLObject> GLApp::objects;
std::stringstream info;
/*	
*	@brief	Initialises neccesary variables and functions at the start of game loop.
*/
void GLApp::init() {
// Part 1: initialize OpenGL state ...
	// clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(1.f, 1.f, 1.f, 1.f);
// Part 2: use the entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);

// Part 3: create as many shared shader programs as required
// handles to shader programs must be contained in GLApp::shdrpgms

	GLApp::VPSS shdr_file_names{ // vertex & fragment shader files
	std::make_pair<std::string, std::string>
	("../shaders/my-tutorial-3.vert", "../shaders/my-tutorial-3.frag")
	};

// Part 4: initialize as many geometric models as required
// these geometric models must be contained in GLApp::models
// We don't need to add any objects to container GLApp::objects
// since the simulation begins with no objects being displayed

	// create shader program from shader files in vector shdr_file_names
	// and insert each shader program into container

	// std::vector<GLSLShader> GLApp::shdrpgms;
	GLApp::init_shdrpgms_cont(shdr_file_names);

	GLApp::init_models_cont();
}

/*	update
* 
*	Updates variables every game loop
*/
void GLApp::update() {
	// Part 1: Update polygon rasterization mode ...
	// Check if key 'P' is pressed
	// If pressed, update polygon rasterization mode

	// Part 2: Spawn or kill objects ...
	// Check if left mouse button is pressed
	// If maximum object limit is not reached, spawn new object(s)
	// Otherwise, kill oldest objects

	// Part 3:
	// for each object in container GLApp::objects
	// Update object's orientation
	// A more elaborate implementation would animate the object's movement
	// A much more elaborate implementation would animate the object's size
	// Using updated attributes, compute world-to-ndc transformation matrix
	for (auto& object : objects) 
	{
		// Update object's orientation
		// ...

		// Update object's movement
		// ...

		// Update object's size
		// ...

		// Compute world-to-ndc transformation matrix
		// ...

		// Part 4: Render the object
		object.draw();
	}
}


/**
 * @brief	Draw the application's window.
			This function renders multiple viewports and sets the window title.
 */
void GLApp::draw() 
{
	// Part 1: write window title with the following (see sample executable):
	// tutorial name - this should be "Tutorial 3"
	// object count - how many objects are being displayed?
	// how many of these objects are boxes?
	// and, how many of these objects are the mystery model?
	// current fps
	// separate each piece of information using " | "
	// see sample executable for example ...

	// Part 2: Clear back buffer of color buffer
		glClear(GL_COLOR_BUFFER_BIT);

	// Part 3: Special rendering modes
	// Use status of flag GLHelper::keystateP to set appropriate polygon
	// rasterization mode using glPolygonMode
	// if rendering GL_POINT, control diameter of rasterized points
		/*	For Points
		*	glPointSize(10.f);
			glVertexAttrib3f(1, 1.f, 0.0f, 0.f); // red color for points
			glDrawArrays(primitive_type, 0, draw_cnt);
			glPointSize(1.f);
		*/
	// using glPointSize
	// if rendering GL_LINE, control width of rasterized lines
	// using glLineWidth
		/* For Lines
		*	glLineWidth(3.f);
			glVertexAttrib3f(1, 0.f, 0.f, 1.f); // blue color for lines
			glDrawArrays(primitive_type, 0, draw_cnt);
			glLineWidth(1.f);
		*/
	
	// Part 4: Render each object in container GLApp::objects
	for (auto const& x : GLApp::objects) {
		x.draw(); // call member function GLObject::draw()
	}
}

void GLApp::cleanup() {
  // empty for now
}

void GLApp::GLObject::draw() const {
	// Part 1: Install the shader program used by this object to
	// render its model using GLSLShader::Use()
		GLApp::shdrpgms[shd_ref].Use();
	// Part 2: Bind object's VAO handle using glBindVertexArray
		glBindVertexArray(GLApp::models[mdl_ref].vaoid);
	// Part 3: Copy object's 3x3 model-to-NDC matrix to vertex shader
	
	// Part 4: Render using glDrawElements or glDrawArrays
	
	// Part 5: Clean up
		glBindVertexArray(0);
		GLApp::shdrpgms[shd_ref].UnUse();
}

GLApp::GLModel GLApp::box_model()
{
	//Define vertex pos and colour attributes
	std::vector<glm::vec2> pos_vtx{
	glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f),
	glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f)
	};
	std::vector<glm::vec3> clr_vtx{
	glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 1.f)
	};
}

GLApp::GLModel GLApp::mystery_model()
{
	
}

void GLApp::init_models_cont() {
	GLApp::models.emplace_back(GLApp::box_model());
	//GLApp::models.emplace_back(GLApp::mystery_model());
}

void GLApp::init_shdrpgms_cont(GLApp::VPSS const& vpss) {

	// Clear the existing shader programs container
	shdrpgms.clear();

	// Iterate over the shader program pairs
	for (const auto& x : vpss) {
		// Create a vector to hold the shader file pairs
		std::vector<std::pair<GLenum, std::string>> shdr_files;

		// Add the vertex shader file path to the vector
		shdr_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, x.first));

		// Add the fragment shader file path to the vector
		shdr_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, x.second));

		// Create a new GLSLShader object
		GLSLShader shdr_pgm;

		// Compile, link, and validate the shader program
		shdr_pgm.CompileLinkValidate(shdr_files);
	
		// Store the shader program in the shdrpgms container
		GLApp::shdrpgms.emplace_back(shdr_pgm);
	}
}



