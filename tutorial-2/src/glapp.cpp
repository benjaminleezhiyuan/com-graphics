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
#include <vector>
#include <glapp.h>
#include <glhelper.h>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

std::vector<GLApp::GLViewport> GLApp::vps;
std::vector<GLApp::GLModel> GLApp::models;

/*	init
* 
*	Initialises neccesary variables and functions at the start of game loop.
*/
void GLApp::init() {
	// Part 1: clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(1.f, 1.f, 1.f, 1.f);
	GLint w{ GLHelper::width }, h{ GLHelper::height };
	// push top-left viewport's specification in vps ...
	vps.push_back({ GLViewport(0, h / 2, w / 2, h / 2) });
	// push top-right viewport's specification in vps ...
	vps.push_back({ GLViewport(w / 2,h / 2,w / 2, h / 2) });
	// push bottom-left viewport's specification in vps ...
	vps.push_back({ GLViewport(0, 0, w / 2,h / 2) });
	// push bottom-right viewport's specification in vps ...
	vps.push_back({ GLViewport(w / 2, 0 / 2, w / 2, h / 2) });

	// Part 3: create different geometries and insert them into
	// repository container GLApp::models ...
	GLApp::models.emplace_back(
		GLApp::points_model("../shaders/my-tutorial-2.vert",
			"../shaders/my-tutorial-2.frag"));
	
}

/*	update
* 
*	Updates variables every game loop
*/
void GLApp::update() {
//nothing

}
 
/*	GLApp::draw
* 
*	Renders objects set in the buffer
*/
void GLApp::draw() {
	// write window title with stuff similar to sample ...
	// how? collect everything you want written to title bar in a
	// std::string object named stitle
	std::string stitle{ GLHelper::title };
	glfwSetWindowTitle(GLHelper::ptr_window, stitle.c_str());
	// clear back buffer as before ...
	glClear(GL_COLOR_BUFFER_BIT);
	// render rectangular shape from NDC coordinates to viewport
	glViewport(vps[0].x, vps[0].y, vps[0].width, vps[0].height);
	GLApp::models[0].draw();

}

void GLApp::cleanup() {
  // empty for now
}

/*	setup_shdrpgm
	
	Compiles the shaders and links the shader objects into a shader program.
*/
void GLApp::GLModel::setup_shdrpgm(std::string vtx_shdr,
	std::string frg_shdr) {
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	shdr_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, vtx_shdr));
	shdr_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, frg_shdr));
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked()) {
		std::cout << "Unable to compile/link/validate shader programs\n";
		std::cout << shdr_pgm.GetLog() << "\n";
		std::exit(EXIT_FAILURE);
	}
}

GLApp::GLModel GLApp::points_model(std::string vtx_shdr,
	std::string frg_shdr) {
	std::vector<glm::vec2> pos_vtx{
	glm::vec2(0.25f, 0.25f), glm::vec2(-0.25f, 0.25f),
	glm::vec2(-0.25f, -0.25f), glm::vec2(0.25f, -0.25f)
	};
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);
	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 0);
	glBindVertexArray(0);
	GLApp::GLModel mdl;
	mdl.vaoid = vaoid;
	mdl.primitive_type = GL_POINTS;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = pos_vtx.size(); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt; // number of primitives (unused)
	return mdl;
} // end of function GLApp::points_model

/*	GLApp::GLModel::draw
* 
*	Renders model, issues draw calls to graphics hardware
*/
void GLApp::GLModel::draw() {
	shdr_pgm.Use();
	glBindVertexArray(vaoid);
	if (primitive_type == GL_POINTS) {
		glPointSize(10.f);
		glVertexAttrib3f(1, 1.f, 0.0f, 0.f); // red color for points
		glDrawArrays(primitive_type, 0, draw_cnt);
		glPointSize(1.f);
	}

	glBindVertexArray(0);
	shdr_pgm.UnUse();
}



