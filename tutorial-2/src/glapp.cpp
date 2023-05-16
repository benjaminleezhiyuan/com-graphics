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
#include <random>
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
	vps.emplace_back( GLViewport(0, h / 2, w / 2, h / 2) );
	// push top-right viewport's specification in vps ...
	vps.emplace_back( GLViewport(w / 2,h / 2,w / 2, h / 2) );
	// push bottom-left viewport's specification in vps ...
	vps.emplace_back( GLViewport(0, 0, w / 2,h / 2) );
	// push bottom-right viewport's specification in vps ...
	vps.emplace_back( GLViewport(w / 2, 0 / 2, w / 2, h / 2) );

	// Part 3: create different geometries and insert them into
	// repository container GLApp::models ...
	
	//Set first geometry to points
	GLApp::models.emplace_back(
		GLApp::points_model(20,20,"../shaders/my-tutorial-2.vert",
			"../shaders/my-tutorial-2.frag"));

	//Set second geometry to lines
	GLApp::models.emplace_back(GLApp::lines_model(40, 40,
		"../shaders/my-tutorial-2.vert",
		"../shaders/my-tutorial-2.frag"));

	//Set third geometry to circle
	GLApp::models.emplace_back(GLApp::trifans_model(50,
		"../shaders/my-tutorial-2.vert",
		"../shaders/my-tutorial-2.frag"));

	//set fourth geometry to triangle strip
	GLApp::models.emplace_back(GLApp::tristrip_model(10,15,
		"../shaders/my-tutorial-2.vert",
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

	//render top left viewport
	glViewport(vps[0].x, vps[0].y, vps[0].width, vps[0].height);
	GLApp::models[0].draw();

	//render top right viewport
	glViewport(vps[1].x, vps[1].y, vps[1].width, vps[1].height);
	GLApp::models[1].draw();

	//render bottom left viewport
	glViewport(vps[2].x, vps[2].y, vps[2].width, vps[2].height);
	GLApp::models[2].draw();

	glViewport(vps[3].x, vps[3].y, vps[3].width, vps[3].height);
	GLApp::models[3].draw();
	
}

void GLApp::cleanup() {
  // empty for now
}

/*	setup_shdrpgm
	
	Compiles the shaders and links the shader objects into a shader program.
*/
void GLApp::GLModel::setup_shdrpgm(std::string vtx_shdr,
	std::string frg_shdr) 
{
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

/**

@brief Generates a model consisting of points forming a grid pattern.

This function procedurally generates a model containing points forming a grid pattern.

The grid is created by placing points at regular intervals on the x and y axes.

The number of points on the x-axis is determined by the slices parameter,

and the number of points on the y-axis is determined by the stacks parameter.

@param slices The number of points on the x-axis.

@param stacks The number of points on the y-axis.

@param vtx_shdr The vertex shader code for the model.

@param frg_shdr The fragment shader code for the model.

@return A GLModel object representing the generated grid model.
*/
GLApp::GLModel GLApp::points_model(int slices,int stacks, std::string vtx_shdr,
	std::string frg_shdr) 
{
	std::vector<glm::vec2> pos_vtx;

	float n_slices = 2.0f / slices;
	float n_stacks = 2.0f / stacks;

	for (int x = 0; x < slices + 1; ++x)
	{
		float x_slices = n_slices * x - 1.f;
		
		for (int y = 0; y < stacks + 1; ++y)
		{
			float y_stacks = n_stacks * y - 1.f;
			pos_vtx.emplace_back(x_slices, y_stacks);
		}
	}

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


/**
@brief Generates a model consisting of lines forming a grid pattern.
This function procedurally generates a model containing lines forming a grid pattern.
The grid is created by drawing horizontal lines between -1 and 1 on the y-axis, and vertical lines
between -1 and 1 on the x-axis. The number of horizontal lines is determined by the stacks parameter,
and the number of vertical lines is determined by the slices parameter.

@param slices		The number of vertical lines (slices) to generate.
@param stacks		The number of horizontal lines (stacks) to generate.
@param vtx_shdr		The vertex shader code for the model.
@param frg_shdr		The fragment shader code for the model.
@return				A GLModel object representing the generated grid model.
*/
GLApp::GLModel GLApp::lines_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr)
{
	int const count{ (slices + 1) * 2 + (stacks + 1) * 2 };
	std::vector<glm::vec2> pos_vtx(count);
	float const u{ 2.f / static_cast<float>(slices) };
	float const p{ 2.f / static_cast<float>(stacks) };
	int index{ 0 };

	//Stores position of vertical lines
	for (int col{ 0 }; col <= slices; ++col) {
		float x{ u * static_cast<float>(col) - 1.f };
		pos_vtx[index++] = glm::vec2(x, -1.f);
		pos_vtx[index++] = glm::vec2(x, 1.f);
	}
	//Stores position of horizontal linesw
	for (int row{ 0 }; row <= stacks; ++row) {
		float y{ p * static_cast<float>(row) - 1.f };
		pos_vtx[index++] = glm::vec2(-1.f,y);
		pos_vtx[index++] = glm::vec2(1.f,y);
	}

	// compute and store endpoints for (slices+1) set of lines
	// for each x from -1 to 1
	// start endpoint is (x, -1) and end endpoint is (x, 1)
	// set up VAO as in GLApp::points_model
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
	mdl.vaoid = vaoid; // set up VAO same as in GLApp::points_model
	mdl.primitive_type = GL_LINES;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = 2 * (slices + 1) + 2 * (stacks + 1); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt / 2; // number of primitives (not used)
	return mdl;

}

/**
 * @brief Generate a triangle fan model of a circle with random vertex colors.
 * @param slices	The number of slices in the circle.
 * @param vtx_shdr	The vertex shader source code.
 * @param frg_shdr	The fragment shader source code.
 * @return			A GLApp::GLModel object representing the triangle fan model.
 */
GLApp::GLModel GLApp::trifans_model(int slices, std::string vtx_shdr, std::string frg_shdr)
{
// Step 1: Generate the (slices+2) count of vertices required to
// render a triangle fan parameterization of a circle with unit
// radius and centered at (0, 0)
	int count = slices + 2;
	std::vector<glm::vec2> pos_vtx(count);
	float angle_increment = 2 * 3.1415f / static_cast<float>(slices);
	//center vertex
	pos_vtx[0] = glm::vec2(0.0f, 0.0f);

	for (int i=0; i <= slices; ++i) {
		float angle = (i) * angle_increment;
		float x = glm::cos(angle);
		float y = glm::sin(angle);
		pos_vtx[i+1] = glm::vec2(x, y);
	}

// Step 2: In addition to vertex position coordinates, compute
// (slices+2) count of vertex color coordinates.
// Each RGB component must be randomly computed.
	//Generate random r, g and b values.
	std::vector<glm::vec3> col_vtx;
	col_vtx.reserve(count * 3);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	for (int i = 0; i < slices + 2; ++i) {
		float r = dis(gen);
		float g = dis(gen);
		float b = dis(gen);
		col_vtx.emplace_back(r, g, b);
	}

// Step 3: Generate a VAO handle to encapsulate the VBO(s) and
// state of this triangle mesh
	GLuint vao_hdl;
	glCreateVertexArrays(1, &vao_hdl);

	GLuint vbo_pos_hdl;
	glCreateBuffers(1, &vbo_pos_hdl);

	// Bind VAO
	glBindVertexArray(vao_hdl);

	// Bind and initialize position data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_hdl);
	glNamedBufferData(vbo_pos_hdl, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Bind and initialize color data
	GLuint vbo_col_hdl;
	glCreateBuffers(1, &vbo_col_hdl);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_col_hdl);
	glNamedBufferData(vbo_col_hdl, sizeof(glm::vec3) * col_vtx.size(), col_vtx.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Unbind VAO and buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create and initialize the GLModel object
	GLApp::GLModel mdl;
	mdl.vaoid = vao_hdl;
	mdl.primitive_type = GL_TRIANGLE_FAN;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = slices + 2; // number of vertices
	mdl.primitive_cnt = slices; // number of primitives (triangle fan)


// Step 4: Return an appropriately initialized instance of GLApp::GLModel

	return mdl;
}


/**
 * @brief Create a triangle strip model with the specified number of slices and stacks.
 *
 * @param slices    The number of slices.
 * @param stacks    The number of stacks.
 * @param vtx_shdr  The filename of the vertex shader.
 * @param frg_shdr  The filename of the fragment shader.
 * @return          The created GLModel object.
 */
GLApp::GLModel GLApp::tristrip_model(int slices, int stacks, std::string vtx_shdr, std::string frg_shdr) {

	//Step 1: Generate the (slices+2) count of vertices for a triangle fan
	std::vector<glm::vec2> pos_vtx;

	float x_slice = 2.0f / slices;
	float y_stack = 2.0f / stacks;

	for (int y = 0; y <= stacks; ++y) {
		float posY = -1.0f + y * y_stack;

		for (int x = 0; x <= slices; ++x) {
			float posX = -1.0f + x * x_slice;
			pos_vtx.emplace_back(posX, posY);
		}
	}

	// Step 2: Generate the index buffer for triangle strip
	std::vector<GLushort> vtx_idx;
	vtx_idx.clear();
	int offset = slices + 1;

	//first layer
	for (int i = 0; i < slices + 1; i++)
	{
		vtx_idx.emplace_back(static_cast<GLushort>(offset + i));
		vtx_idx.emplace_back(static_cast<GLushort>(i));
	}

	for (int y = 2; y < stacks + 1; y++)
	{
		std::vector<GLushort>temp;
		temp.clear();
		for (int x = 0; x < slices + 1; x++)
		{
			temp.emplace_back(static_cast<GLushort>(y * offset + x));
			temp.emplace_back(static_cast<GLushort>((y - 1) * offset + x));

		}
		vtx_idx.emplace_back(vtx_idx.back());
		vtx_idx.emplace_back(temp[0]);

		//append temp to vtx_idx
		for (const auto& element : temp) {
			vtx_idx.emplace_back(element);
		}
	}

	std::vector<glm::vec3> colors((slices + 1) * (stacks + 1));
	// Generate random colors
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	for (int i = 0; i < (slices + 1) * (stacks + 1); ++i) {
		colors[i] = glm::vec3(dis(gen), dis(gen), dis(gen));
	}
	// Step 3: Generate VAO handle
	GLuint vao_hdl;
	glCreateVertexArrays(1, &vao_hdl);

	GLuint vbo_pos_hdl, ibo_hdl;
	glCreateBuffers(1, &vbo_pos_hdl);
	glCreateBuffers(1, &ibo_hdl);

	// Bind VAO
	glBindVertexArray(vao_hdl);

	// Bind and initialize position data
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos_hdl);
	glNamedBufferData(vbo_pos_hdl, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Bind and initialize color data
	GLuint vbo_col_hdl;
	glCreateBuffers(1, &vbo_col_hdl);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_col_hdl);
	glNamedBufferData(vbo_col_hdl, sizeof(glm::vec3) * colors.size(), colors.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Bind and initialize index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_hdl);
	glNamedBufferData(ibo_hdl, sizeof(GLuint) * vtx_idx.size(), vtx_idx.data(), GL_DYNAMIC_DRAW);

	// Unbind VAO and buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create and initialize the GLModel object
	GLApp::GLModel mdl;
	mdl.vaoid = vao_hdl;
	mdl.primitive_type = GL_TRIANGLE_STRIP;
	mdl.setup_shdrpgm(vtx_shdr, frg_shdr);
	mdl.draw_cnt = vtx_idx.size(); // number of vertices
	mdl.primitive_cnt = slices * stacks * 2; // number of primitives (triangles)

	return mdl;
}


/*	GLApp::GLModel::draw
* 
*	Renders model, issues draw calls to graphics hardware
*/
void GLApp::GLModel::draw() {
	shdr_pgm.Use();
	glBindVertexArray(vaoid);
	switch (primitive_type) {
	case GL_POINTS:
		glPointSize(10.f);
		glVertexAttrib3f(1, 1.f, 0.0f, 0.f); // red color for points
		glDrawArrays(primitive_type, 0, draw_cnt);
		glPointSize(1.f);
		break;
	case GL_LINES:
		glLineWidth(3.f);
		glVertexAttrib3f(1, 0.f, 0.f, 1.f); // blue color for lines
		glDrawArrays(primitive_type, 0, draw_cnt);
		glLineWidth(1.f);
		break;
	case GL_TRIANGLE_FAN:
		glDrawArrays(primitive_type, 0, draw_cnt);
		break;
	case GL_TRIANGLE_STRIP:
		glDrawElements(primitive_type, draw_cnt, GL_UNSIGNED_SHORT,nullptr);
		break;
	}
	glBindVertexArray(0);
	shdr_pgm.UnUse();

}



