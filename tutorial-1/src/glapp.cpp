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
#include <iostream>
#include <glapp.h>
#include <glhelper.h>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>


//variables for colour change
GLfloat randomColor1[4];
GLfloat randomColor2[4];
GLfloat interpolatedColor[4];
float interpolationFactor = 0.0f;
const float interpolationDuration = 10.0f;
struct GLApp::GLModel GLApp::mdl;

/*                                                   objects with file scope
----------------------------------------------------------------------------- */
void GLApp::init() {
	// Part 1: clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(1.f, 0.f, 0.f, 1.f);
	// Part 2: use the entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);
	// Part 3: initialize VAO and create shader program
	mdl.setup_vao();
	mdl.setup_shdrpgm();

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

	interpolationFactor += 0.0167f / interpolationDuration;

	if (interpolationFactor > 1.0f) 
	{
		interpolationFactor = 0.0f;
		// Set current colour
		randomColor1[0] = randomColor2[0];
		randomColor1[1] = randomColor2[1];
		randomColor1[2] = randomColor2[2];
		randomColor1[3] = randomColor2[3];
		// Set new random colour
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
	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT);
	// now, render rectangular model from NDC coordinates to viewport
	mdl.draw();

	//Output to window bar title
	char tmp[128];
	sprintf(tmp, "Tutorial 1 | Benjamin Lee | %.2f", GLHelper::fps);
	glfwSetWindowTitle(GLHelper::ptr_window, tmp);
}

void GLApp::cleanup() {
  // empty for now
}

void GLApp::GLModel::setup_vao()
{
	//Define vertex pos and colour attributes
	std::array<glm::vec2, 4> pos_vtx{
	glm::vec2(0.5f, -0.5f), glm::vec2(0.5f, 0.5f),
	glm::vec2(-0.5f, 0.5f), glm::vec2(-0.5f, -0.5f)
	};
	std::array<glm::vec3, 4> clr_vtx{
	glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f),
	glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 1.f, 1.f)
	};

	//transfer vertex pos and colour attributes to VBO
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size() + sizeof(glm::vec3) * clr_vtx.size(),
		nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo_hdl, 0,
		sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data());
	glNamedBufferSubData(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data());
	
	// encapsulate information about contents of VBO and VBO handle
	// to another object called VAO
	glCreateVertexArrays(1, &vaoid);
	// for vertex position array, we use vertex attribute index 8
	// and vertex buffer binding point 3
	glEnableVertexArrayAttrib(vaoid, 8);
	glVertexArrayVertexBuffer(vaoid, 3, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 8, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 8, 3);
	// for vertex color array, we use vertex attribute index 9
	// and vertex buffer binding point 4
	glEnableVertexArrayAttrib(vaoid, 9);
	glVertexArrayVertexBuffer(vaoid, 4, vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size(), sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 9, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 9, 4);

	primitive_type = GL_TRIANGLES;
	// represents indices of vertices that will define 2 triangles with
	// counterclockwise winding
	std::array<GLushort, 6> idx_vtx{
	0, 1, 2, // 1st triangle with counterclockwise winding is specified by
	// vertices in VBOs with indices 0, 1, 2
	2, 3, 0 // 2nd triangle with counterclockwise winding
	};

	idx_elem_cnt = idx_vtx.size();
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl,
		sizeof(GLushort) * idx_elem_cnt,
		reinterpret_cast<GLvoid*>(idx_vtx.data()),
		GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vaoid, ebo_hdl);
	glBindVertexArray(0);
}

void GLApp::GLModel::setup_shdrpgm() {
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	shdr_files.emplace_back(std::make_pair(
		GL_VERTEX_SHADER,
		"../shaders/my-tutorial-1.vert"));
	shdr_files.emplace_back(std::make_pair(
		GL_FRAGMENT_SHADER,
		"../shaders/my-tutorial-1.frag"));
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked()) {
		std::cout << "Unable to compile/link/validate shader programs" << "\n";
		std::cout << shdr_pgm.GetLog() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

void GLApp::GLModel::draw() {
	// there are many shader programs initialized - here we're saying
	// which specific shader program should be used to render geometry
	shdr_pgm.Use();
	// there are many models, each with their own initialized VAO object
	// here, we're saying which VAO's state should be used to set up pipe
	glBindVertexArray(vaoid);
	// here, we're saying what primitive is to be rendered and how many
	// such primitives exist.
	// the graphics driver knows where to get the indices because the VAO
	// containing this state information has been made current ...
	glDrawElements(primitive_type, idx_elem_cnt, GL_UNSIGNED_SHORT, NULL);
	// after completing the rendering, we tell the driver that VAO
	// vaoid and current shader program are no longer current
	glBindVertexArray(0);
	shdr_pgm.UnUse();
}


