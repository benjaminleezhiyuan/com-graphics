/*!
@file    glapp.cpp
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		10/05/2023
@ammendent	Added function definitions for functions init, update, draw
			setup_vao, setup_shdrpgm.

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

struct GLApp::GLModel GLApp::mdl;

/*	init
* 
*	Initialises neccesary variables and functions at the start of game loop.
*/
void GLApp::init() {
	// Part 1: clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(1.f, 0.f, 0.f, 1.f);
	// Part 2: use the entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);
	// Part 3: initialize VAO and create shader program
	mdl.setup_vao();
	mdl.setup_shdrpgm();
}

/*	update
* 
*	Updates variables every game loop
*/
void GLApp::update() {
	// Clear the color buffer with the interpolated color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
 
/*	GLApp::draw
* 
*	Renders objects set in the buffer
*/
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

/*	setup_vao
	
	Vertex Array Object setup for both position and coor attributes

*/
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
	// for vertex position array, we use vertex attribute index 0
	// and vertex buffer binding point 3
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 3, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 3);
	// for vertex color array, we use vertex attribute index 1
	// and vertex buffer binding point 4
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayVertexBuffer(vaoid, 4, vbo_hdl,
		sizeof(glm::vec2) * pos_vtx.size(), sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 1, 4);

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

/*	setup_shdrpgm
	
	Compiles the shaders and links the shader objects into a shader program.
*/
void GLApp::GLModel::setup_shdrpgm() {
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	shdr_files.emplace_back(std::make_pair(
		GL_VERTEX_SHADER,
		"../shaders/my-tutorial-2.vert"));
	shdr_files.emplace_back(std::make_pair(
		GL_FRAGMENT_SHADER,
		"../shaders/my-tutorial-2.frag"));
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked()) {
		std::cout << "Unable to compile/link/validate shader programs" << "\n";
		std::cout << shdr_pgm.GetLog() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

/*	GLApp::GLModel::draw
* 
*	Renders model, issues draw calls to graphics hardware
*/
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


