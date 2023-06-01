/*!
@file    glapp.cpp
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		26/05/2023
@ammendent	edited init update and draw functions. Added object init draw and update functions.
			

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <iostream>
#include <iomanip>
#include <random>
#include <glapp.h>
#include <glhelper.h>
#include <glslshader.h>
#include <array>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

// define singleton containers
std::map<std::string, GLSLShader> GLApp::shdrpgms;
std::map<std::string, GLApp::GLModel> GLApp::models;
std::map<std::string, GLApp::GLObject> GLApp::objects;
GLApp::Camera2D GLApp::cam;

//enumaration for raster mode
enum mode
{
	fill,
	line,
	point
};

//store current raster mode
static mode raster = fill;

/*	
*	@brief	Initialises neccesary variables and functions at the start of game loop.

*/
void GLApp::init() {
	// Part 1: Initialize OpenGL state ...
	glClearColor(1.f, 1.f, 1.f, 1.f);
	// Part 2: Use the entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);
	// Part 3: parse scene file $(SolutionDir)scenes/tutorial-4.scn
	// and store repositories of models of type GLModel in container
	// GLApp::models, store shader programs of type GLSLShader in
	// container GLApp::shdrpgms, and store repositories of objects of
	// type GLObject in container GLApp::objects
	GLApp::init_scene("../scenes/tutorial-4.scn");
	// Part 4: initialize camera
	// explained in a later section ...
	cam.init(GLHelper::ptr_window, &objects.at("Camera"));
}

/*	update
* 
*	Updates variables every game loop
*/
void GLApp::update() 
{
	cam.update(GLHelper::ptr_window);

	for (auto& [key, val] : objects) {
		if (key == "Camera") continue;
		val.update(GLHelper::delta_time);
	}

	if (GLHelper::keystateP == GL_TRUE)
	{
		switch (raster)
		{
		case fill:
			raster = line;
			break;
		case line:
			raster = point;
			break;
		case point:
			raster = fill;
			break;
		}
		GLHelper::keystateP = GL_FALSE;
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
	// Print to window title bar
	std::stringstream cam_pos{};
	cam_pos << std::fixed << std::setprecision(2) << "(" << objects["Camera"].position.x << ", " << objects["Camera"].position.y << ")";
	std::stringstream cam_orientation{};
	cam_orientation << "Orientation: " << objects["Camera"].orientation.x << " degrees";
	std::stringstream window_height{};
	window_height << "Window height: " << cam.height;
	std::stringstream fps{};
	fps << std::fixed << std::setprecision(2) << "FPS: " << GLHelper::fps;
	std::stringstream space{};
	space << " | ";
	std::stringstream title{};
	title << "Tutorial 4 | Benjamin Lee | Camera Position " << cam_pos.str() << space.str() <<
																cam_orientation.str() << space.str() <<
																window_height.str() << space.str() <<
																fps.str();
	glfwSetWindowTitle(GLHelper::ptr_window, title.str().c_str());

	// Part 2: Clear back buffer of color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Part 3: Change raster mode
	switch (raster)
	{
	case fill:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case line:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case point:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	}

	// Part 4: Render each object in container GLApp::objects
	for (auto const& [key,val] : GLApp::objects) 
	{	
		if (key == "Camera")
		{
			continue;
		}
		val.draw(); // call member function GLObject::draw()
	}
	//render camera
	objects["Camera"].draw();
}

void GLApp::cleanup() 
{
	// empty for now
}

void GLApp::init_scene(std::string scene_filename)
{
	std::ifstream ifs{ scene_filename, std::ios::in };
	if (!ifs) {
		std::cout << "ERROR: Unable to open scene file: "
			<< scene_filename << "\n";
		exit(EXIT_FAILURE);
	}
	ifs.seekg(0, std::ios::beg);

	std::string line;
	getline(ifs, line);

	std::istringstream line_sstm{ line };
	int obj_cnt{};
	line_sstm >> obj_cnt;
	while (obj_cnt--)
	{
		//get model name and init model if not found
		getline(ifs, line);
		std::istringstream line_modelname{ line };
		std::string model_name;
		line_modelname >> model_name;

		if (models.find(model_name) == models.end()) {
			GLModel mdl{};
			mdl.init(model_name);
			models[model_name] = mdl;
		}

		//get model name
		getline(ifs, line);
		GLObject obj{};
		std::istringstream line_objname{ line };
		line_objname >> obj.name;

		//get shader program name and init shader program if not found
		getline(ifs, line);
		std::istringstream line_shdrpgmname{ line };
		std::string shdrpgm_name;
		line_shdrpgmname >> shdrpgm_name;

		if (shdrpgms.find(shdrpgm_name) == shdrpgms.end()) {
			std::string vtx_shdr{}, frg_shdr{};
			line_shdrpgmname >> vtx_shdr >> frg_shdr;
			init_shdrpgms(shdrpgm_name, vtx_shdr, frg_shdr);
		}

		//get color of object
		getline(ifs, line);
		std::istringstream line_color{ line };
		line_color >> obj.color[0] >> obj.color[1] >> obj.color[2];

		//get scale of object
		getline(ifs, line);
		std::istringstream line_scale{ line };
		line_scale >> obj.scaling.x >> obj.scaling.y;

		//get angular disp and angular speed of object
		getline(ifs, line);
		std::istringstream line_orientation{ line };
		line_orientation >> obj.orientation.x >> obj.orientation.y;

		//get position of object
		getline(ifs, line);
		std::istringstream line_position{ line };
		line_position >> obj.position.x >> obj.position.y;

		//set mdl_ref and shd_ref
		obj.mdl_ref = models.find(model_name);
		obj.shd_ref = shdrpgms.find(shdrpgm_name);

		objects[obj.name] = obj;
	}
}

void GLApp::init_shdrpgms(std::string shdr_pgm_name, std::string vtx_shdr, std::string frg_shdr) 
{
	std::vector<std::pair<GLenum, std::string>> shdr_files{
	std::make_pair(GL_VERTEX_SHADER, vtx_shdr),
	std::make_pair(GL_FRAGMENT_SHADER, frg_shdr)
	};
	GLSLShader shdr_pgm;
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked()) {
		std::cout << "Unable to compile/link/validate shader programs\n";
		std::cout << shdr_pgm.GetLog() << "\n";
		std::exit(EXIT_FAILURE);
	}
	// add compiled, linked, and validated shader program to
	// std::map container GLApp::shdrpgms
	GLApp::shdrpgms[shdr_pgm_name] = shdr_pgm;
}

void GLApp::GLModel::init(std::string model_file_name)
{
	std::string mesh = "../meshes/" + model_file_name + ".msh";
	std::ifstream ifs{ mesh, std::ios::in };
	if (!ifs) {
		std::cout << "ERROR: Unable to open mesh file: "
			<< mesh << "\n";
		exit(EXIT_FAILURE);
	}
	ifs.seekg(0, std::ios::beg);

	std::string lines, name;
	std::vector<glm::vec2> pos_vtx;
	std::vector<GLushort> idx_vtx;
	pos_vtx.clear();
	idx_vtx.clear();
	while (getline(ifs, lines)) {
		std::istringstream line{ lines };
		std::string prefix;
		line >> prefix;
		// read name
		if (prefix == "n") {
			line >> name;
		}
		// read vertex position
		else if (prefix == "v") {
			glm::vec2 pos{};
			line >> pos.x >> pos.y;
			pos_vtx.emplace_back(pos);
		}
		// read triangle primitives rendered as GL_TRIANGLES
		else if (prefix == "t") {
			primitive_type = GL_TRIANGLES;
			GLushort idx{};
			while (line >> idx) {
				idx_vtx.emplace_back(idx);
			}
		}
		// read triangle primitives rendered as GL_TRIANGLE_FAN
		else if (prefix == "f") {
			primitive_type = GL_TRIANGLE_FAN;
			GLushort idx{};
			while (line >> idx) {
				idx_vtx.emplace_back(idx);
			}
		}
	}
	//transfer vertex position attributes to VBO
	// creating a buffer object
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	// allocating and filling data store
	glNamedBufferStorage(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(),
		nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(vbo_hdl, 0, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data());

	// creating vertex array object
	glCreateVertexArrays(1, &vaoid);
	// VAO setup for position attributes
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 3, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 3);

	// transfer topo information to GPU
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl,
		sizeof(GLushort) * idx_vtx.size(), reinterpret_cast<GLvoid*>(idx_vtx.data()),
		GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	glBindVertexArray(0);

	draw_cnt = idx_vtx.size();		// number of vertices
	primitive_cnt = idx_vtx.size();		// number of primitives unused
}

void GLApp::Camera2D::init(GLFWwindow* win, GLApp::GLObject* ptr) 
{
	pgo = ptr;

	GLsizei fb_width{}, fb_height{};
	glfwGetFramebufferSize(win, &fb_width, &fb_height);
	ar = static_cast<GLfloat>(fb_width) / fb_height;

	GLfloat rad = glm::radians(pgo->orientation.x);
	up = glm::vec2{ -sin(rad), cos(rad) };
	right = glm::vec2{ cos(rad), sin(rad) };

	view_xform = { glm::vec3{1, 0, 0},
				   glm::vec3{0, 1, 0},
				   glm::vec3{-pgo->position.x, -pgo->position.y, 1} };

	camwin_to_ndc_xform = { glm::vec3{2.f / static_cast<float>(fb_width), 0, 0},
							glm::vec3{0, 2.f / static_cast<float>(fb_height), 0},
							glm::vec3{0, 0, 1} };

	world_to_ndc_xform = camwin_to_ndc_xform * view_xform;
}

void GLApp::Camera2D::update(GLFWwindow*) {
	left_turn_flag = GLHelper::keystateH;
	right_turn_flag = GLHelper::keystateK;
	move_flag = GLHelper::keystateU;
	zoom_flag = GLHelper::keystateZ;

	if (GLHelper::keystateV) {
		camtype_flag = camtype_flag ? GL_FALSE : GL_TRUE;
		GLHelper::keystateV = GL_FALSE;
	}

	//update camera orientation
	if (left_turn_flag || right_turn_flag) {
		
		if (left_turn_flag) {
			pgo->orientation.x += pgo->orientation.y;
			if (pgo->orientation.x > 360.f) pgo->orientation.x = 0.f;
		}
		else {
			pgo->orientation.x -= pgo->orientation.y;
			if (pgo->orientation.x < -360.f) pgo->orientation.x = 0.f;
		}

		GLfloat rad = glm::radians(pgo->orientation.x);
		up = glm::vec2{ -sin(rad), cos(rad) };
		right = glm::vec2{ cos(rad), sin(rad) };
	}

	if (move_flag) {
		pgo->position = pgo->position + (linear_speed * up);
	}

	if (zoom_flag) {
		if (height >= max_height) {
			height_chg_dir = -1;
		}
		else if (height <= min_height) {
			height_chg_dir = 1;
		}

		height += height_chg_dir * height_chg_val;
		zoom_flag = GL_FALSE;
	}

	if (camtype_flag) {
		view_xform = { glm::vec3{1, 0, 0},
					   glm::vec3{0, 1, 0},
					   glm::vec3{-pgo->position.x, -pgo->position.y, 1} };
	}
	else {
		view_xform = { glm::vec3{right.x, up.x, 0},
					   glm::vec3{right.y, up.y, 0},
					   glm::vec3{-(glm::dot(right,pgo->position)), -(glm::dot(up,pgo->position)), 1} };
	}

	GLfloat angle_rad = glm::radians(pgo->orientation.x);
	glm::mat3 scale_mtx = { glm::vec3{pgo->scaling.x, 0, 0},
							glm::vec3{0, pgo->scaling.y, 0},
							glm::vec3{0, 0, 1} };

	glm::mat3 rot_mtx = { glm::vec3{cos(angle_rad), sin(angle_rad), 0},
						  glm::vec3{-sin(angle_rad), cos(angle_rad), 0},
						  glm::vec3{0, 0, 1} };

	glm::mat3 trans_mtx = { glm::vec3{1, 0, 0},
							glm::vec3{0, 1, 0},
							glm::vec3{pgo->position.x, pgo->position.y, 1} };

	pgo->mdl_xform = trans_mtx * rot_mtx * scale_mtx;

	camwin_to_ndc_xform = { glm::vec3{2.f / static_cast<GLfloat>(height * ar), 0, 0},
							glm::vec3{0, 2.f / static_cast<GLfloat>(height), 0},
							glm::vec3{0, 0, 1} };

	world_to_ndc_xform = camwin_to_ndc_xform * view_xform;

	pgo->mdl_to_ndc_xform = world_to_ndc_xform * pgo->mdl_xform;
}

void GLApp::GLModel::release()
{

}

/**

@brief Initializes the GLObject.

This function initializes the GLObject by setting various parameters such as the model reference,

shader reference, position, scaling, rotation speed, and current orientation.

The model reference is randomly chosen between 0 and 1.

The shader reference is set to 0.

The position is randomly set between -1 and 1 on both x and y axes.

The scaling is randomly set between 0.05 and 0.15 on both x and y axes.

The rotation speed is randomly set between -1 and 1.

The current orientation is calculated by multiplying a random value between -1 and 1 with 360.
*/
void GLApp::GLObject::init()
{
	/*double const range_x = GLHelper::width;
	double const range_y = GLHelper::height;*/

	//randomly choose 0 or 1 to decide which model data is assigned to mdl_ref
	std::random_device ran;
	std::mt19937 mystery(ran());
	std::uniform_int_distribution<> dis(0, 1);
	//mdl_ref = dis(mystery);

	//assign shd_ref value of 0
	//shd_ref = 0;

	//init the objects parameters
	std::uniform_real_distribution<> dis_float(-1.f, 1.f);

	//set random position between -1.f and 1.f
	std::uniform_real_distribution<> dis_coordinate(-1.f, 1.f);
	position = glm::vec2(dis_coordinate(mystery), dis_coordinate(mystery));

	//Initialise objects scale, rotation
	std::uniform_real_distribution<> dis_scale(0.05, 0.15);		//random scale
	scaling = glm::vec2(dis_scale(mystery), dis_scale(mystery));
	orientation.y = static_cast<float>(dis_float(mystery));			//random rotation speed
	orientation.x = static_cast<float>(dis_float(mystery)) * 360.f;	//current orientation
}

/**

@brief Draws the GLObject.

This function performs the necessary steps to draw the GLObject on the screen.

It installs the shader program, binds the object's vertex array object (VAO), and copies the object's

transformation matrix to the vertex shader. Then, based on the specified raster mode, it renders the model

using the appropriate OpenGL draw function (glDrawArrays). Finally, it cleans up by unbinding the VAO and

unusing the shader program.
*/
void GLApp::GLObject::draw() const
{	
	shd_ref->second.Use();

	glBindVertexArray(mdl_ref->second.vaoid);

	GLint uniform_var_loc1 = glGetUniformLocation(shd_ref->second.GetHandle(), "uModel_to_NDC");

	if (uniform_var_loc1 >= 0) {
		glUniformMatrix3fv(uniform_var_loc1, 1, GL_FALSE, glm::value_ptr(mdl_to_ndc_xform));
	}
	else {
		std::cout << "Uniform variable 1 doesn't exist\n";
		std::exit(EXIT_FAILURE);
	}

	GLint uniform_var_loc2 = glGetUniformLocation(shd_ref->second.GetHandle(), "uColor");

	if (uniform_var_loc2 >= 0) {
		glUniform3fv(uniform_var_loc2, 1, glm::value_ptr(color));
	}
	else {
		std::cout << "Uniform variable 2 doesn't exist\n";
		std::exit(EXIT_FAILURE);
	}

	//glVertexAttrib3f(1, color.r, color.g, color.b);
	glDrawElements(mdl_ref->second.primitive_type, mdl_ref->second.draw_cnt, GL_UNSIGNED_SHORT, NULL);
	//glDrawArrays(mdl_ref->second.primitive_type, 0, mdl_ref->second.draw_cnt);

	glBindVertexArray(0);

	shd_ref->second.UnUse();
}

/**

@brief Updates the GLObject.

This function updates the GLObject by modifying its orientation based on the provided delta time.

It calculates the new angle displacement by adding the product of angle speed and delta time to the

current angle displacement. Then, it calculates the translation matrix, rotation matrix, and scale matrix

based on the object's position, angle displacement, and scaling values. Finally, it combines these matrices

to calculate the model-to-NDC transformation matrix (mdl_to_ndc_xform).

@param delta_time The time difference between the current frame and the previous frame.
*/
void GLApp::GLObject::update(GLdouble delta_time)
{
	glm::mat3 scale_mtx{ scaling.x,			0,	0,
									 0,	scaling.y,	0,
									 0,			0,	1 };

	glm::mat3 trans_mtx{ 1,			0,0,
						 0,			1,0,
				position.x,position.y,1 };

	
	orientation.x += static_cast<GLfloat>(orientation.y * delta_time);
	float rad = glm::radians(orientation.x);

	glm::mat3 rot_mtx{ glm::cos(rad), glm::sin(rad), 0,
					  -glm::sin(rad), glm::cos(rad), 0,
								   0, 			  0, 1 };

	mdl_xform = trans_mtx * rot_mtx * scale_mtx;
	mdl_to_ndc_xform = cam.world_to_ndc_xform * mdl_xform;
}







