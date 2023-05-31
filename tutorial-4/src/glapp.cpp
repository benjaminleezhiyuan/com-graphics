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

std::vector<glm::vec2> vtx_pos;
std::vector<glm::vec3> index;

void GLApp::GLModel::init(std::string model_file_name)
{
	std::string file_name = "../meshes/" + model_file_name + ".msh";
	// Read data from file
	std::ifstream file(file_name);
	if (!file.is_open())
	{
		std::cout << "Failed to open square.msh" << std::endl;
		EXIT_FAILURE;
	}

	// Process mesh data
	std::string line;
	while (std::getline(file, line))
	{
		std::istringstream iss(line);

		char type;
		iss >> type;
		if (type == 'n')
		{
			std::string model_name;
			iss >> model_name;
		}
		else if (type == 'v')
		{
			float x, y;
			iss >> x >> y;
			vtx_pos.emplace_back(x, y);
		}
		else if (type == 't')
		{
			int v1, v2, v3;
			iss >> v1 >> v2 >> v3;
			index.emplace_back(v1, v2, v3);
		}
		else if (type == 'f')
		{

		}
	}
	file.close();
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
	getline(ifs, line); // first line is count of objects in scene
	std::istringstream line_sstm{ line };
	int obj_cnt;
	line_sstm >> obj_cnt; // read count of objects in scene
	while (obj_cnt--) // read each object's parameters
	{
		getline(ifs, line); // 1st parameter: model's name
		std::istringstream line_modelname{ line };
		std::string model_name;
		line_modelname >> model_name;
		/*
		add code to do this:
		if model with name model_name is not present in std::map container
		called models, then add this model to the container
		*/
		// Check if model with name model_name is already present in the models map
		if (models.find(model_name) == models.end())
		{
			// Model doesn't exist in the map, create and add it
			GLModel model;
			// Initialize the model based on the file data
			models[model_name].init(model_name);
		}
		
		/*
		add code to do this:
		if shader program listed in the scene file is not present in
		std::map container called shdrpgms, then add this model to the
		container
		*/
		getline(ifs, line); //name of object
		std::istringstream line_objectname{ line };
		std::string object_name;
		line_objectname >> object_name;

		getline(ifs, line); //shader program name
		std::string shaderP, vtx_shd, frg_shd;
		std::istringstream shader_prg{ line };
		shader_prg >> shaderP >> vtx_shd >> frg_shd;

		//init shader program
		init_shdrpgms(shaderP, vtx_shd, frg_shd);

		/*
		add code to do this:
		read remaining parameters of object from file:
		object's name
		RGB parameters for rendering object's model geometry
		scaling factors to be applied on object's model
		orientation factors: initial angular orientation and angular speed
		object's position in game world
		set data member GLApp::GLObject::mdl_ref to iterator that points to
		model instantiated by this object
		set data member GLApp::GLObject::shd_ref to iterator that points to
		shader program used by this object
		insert this object to std::map container objects
		*/
		getline(ifs, line); //rgb values
		std::istringstream color{ line };
		float r, g, b;
		color >> r >> g >> b;

		getline(ifs, line); // Scaling factors to be applied on object's model
		std::istringstream line_scaling{ line };
		float scaleX, scaleY;
		line_scaling >> scaleX >> scaleY;

		getline(ifs, line); // Orientation factors: initial angular orientation and angular speed
		std::istringstream line_orientation{ line };
		float angle, angularSpeed;
		line_orientation >> angle >> angularSpeed;

		getline(ifs, line); // Object's position in the game world
		std::istringstream line_position{ line };
		float posX, posY;
		line_position >> posX >> posY;

		// Set the references to the model and shader program
		auto modelRef = models.find(model_name);
		auto shaderRef = shdrpgms.find(shaderP);

		std::cout << model_name << std::endl;
		std::cout << object_name << std::endl;
		std::cout << shaderP << std::endl;
		std::cout << r << " " << g << " " << b << std::endl;
		std::cout << scaleX << " " << scaleY << std::endl;
		std::cout << angle << " " << angularSpeed << std::endl;
		std::cout << posX << " " << posY << std::endl;
		// Create a GLObject and populate its properties
		GLObject object;
		object.mdl_ref = modelRef;
		object.shd_ref = shaderRef;
		object.color = glm::vec3(r, g, b);
		object.scaling = glm::vec2(scaleX, scaleY);
		object.orientation = glm::vec2(angle, angularSpeed);
		object.position = glm::vec2(posX, posY);

		objects.insert(std::make_pair(object_name, object));
	}
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
	// Part 1: Get the shader program in use by this object
	const GLSLShader& shaderProgram = shd_ref->second;

	// Part 2: Bind object's VAO handle
	glBindVertexArray(mdl_ref->second.vaoid);

	// Part 3: Copy object's color to fragment shader uniform variable uColor
	GLint uniform_var_loc2 = glGetUniformLocation(shaderProgram.GetHandle(), "uColor");
	if (uniform_var_loc2 >= 0) {
		const glm::vec3& color2 = this->color;
		glUniform3fv(uniform_var_loc2, 1, glm::value_ptr(color2));
	}
	else {
		std::cout << "Uniform variable doesn't exist!!!" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Part 4: Copy object's model-to-NDC matrix to vertex shader's uniform variable uModelToNDC
	GLint uniform_var_loc1 = glGetUniformLocation(shaderProgram.GetHandle(), "uModelToNDC");
	if (uniform_var_loc1 >= 0) {
		const glm::mat3& matrix = this->mdl_to_ndc_xform;
		glUniformMatrix3fv(uniform_var_loc1, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	else {
		std::cout << "Uniform variable doesn't exist!!!" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Part 5: Call glDrawArrays with appropriate arguments
	const GLModel& model = mdl_ref->second;
	glDrawArrays(model.primitive_type, 0, model.primitive_cnt);

	// Part 6: Unbind VAO and unload shader program
	glBindVertexArray(0);
	glUseProgram(0);
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
	//update object orientation
	orientation.x += static_cast<float>(orientation.y * delta_time);

	//translation matrix
	glm::mat3 transmtx = glm::mat3(1.f);
	transmtx[2] = glm::vec3(position.x, position.y, 1.f);

	//rotation matrix
	glm::mat3 rotmtx = glm::mat3(1.f);
	rotmtx[0] = glm::vec3(std::cos(orientation.x), -std::sin(orientation.x), 0.f);
	rotmtx[1] = glm::vec3(std::sin(orientation.x), std::cos(orientation.x), 0.f);

	//scale matrix
	glm::mat3 scalmtx = glm::mat3(1.f);
	scalmtx[0][0] = scaling.x;
	scalmtx[1][1] = scaling.y;

	//perform transform calculations
	mdl_to_ndc_xform = transmtx * rotmtx * scalmtx;
}

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
}

/*	update
* 
*	Updates variables every game loop
*/
void GLApp::update() 
{

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
	std::string title = "Tutorial 4 | Benjamin Lee | Obj: " + std::to_string(objects.size()) + " | Box: " +
		" | Mystery: " +
		" | " + std::to_string(GLHelper::fps).substr(0, 5);//fps count
	glfwSetWindowTitle(GLHelper::ptr_window, title.c_str());

	// Part 2: Clear back buffer of color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Part 4: Render each object in container GLApp::objects
	for (auto const& x : GLApp::objects) {
		x.second.draw(); // call member function GLObject::draw()
	}
}

void GLApp::cleanup() {
	// empty for now
}




