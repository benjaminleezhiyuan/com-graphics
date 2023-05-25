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
#include <array>
#include <glm/gtc/type_ptr.hpp>

// define singleton containers
std::vector<GLSLShader> GLApp::shdrpgms;
std::vector<GLApp::GLModel> GLApp::models;
std::list<GLApp::GLObject> GLApp::objects;

//static variables
static int box_count = 0,	 //number of boxes
mystery_count = 0,			//number of mystery objects
increase_count = 0,		   //number of objects to spawn
decrease_count = 0;		  //number of objects to kill

static GLboolean spawn_mode = true; //spawn_mode true = spawn objects, false = kill objects

//enumaration for raster mode
enum mode
{
	fill,
	line,
	point
};

//store current raster mode
static mode raster = fill;

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
	mdl_ref = dis(mystery);

	//assign shd_ref value of 0
	shd_ref = 0;

	//init the objects parameters
	std::uniform_real_distribution<> dis_float(-1.f, 1.f);

	//set random position between -1.f and 1.f
	std::uniform_real_distribution<> dis_coordinate(-1.f, 1.f);
	position = glm::vec2(dis_coordinate(mystery), dis_coordinate(mystery));

	//Initialise objects scale, rotation
	std::uniform_real_distribution<> dis_scale(0.05, 0.15);		//random scale
	scaling = glm::vec2(dis_scale(mystery), dis_scale(mystery));
	angle_speed = static_cast<float>(dis_float(mystery));			//random rotation speed
	angle_disp = static_cast<float>(dis_float(mystery)) * 360.f;	//current orientation
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
	//install shader prgm
	shdrpgms[shd_ref].Use();

	//bind object vao handle
	glBindVertexArray(models[mdl_ref].vaoid);

	//copy object 3x3 model to ndc matrix to vtx shader
	GLint uniform_var_loc1 =
		glGetUniformLocation(GLApp::shdrpgms[shd_ref].GetHandle(),
			"uModel_to_NDC");
	if (uniform_var_loc1 >= 0) {
		// Print the matrix elements
		const glm::mat3& matrix = GLApp::GLObject::mdl_to_ndc_xform;

		glUniformMatrix3fv(uniform_var_loc1, 1, GL_FALSE,
			glm::value_ptr(matrix));
	}
	else {
		std::cout << "Uniform variable doesn't exist!!!\n";
		std::exit(EXIT_FAILURE);
	}

	//render model
	switch (raster)
	{
	case line:
		glLineWidth(10.f);
		glDrawArrays(models[mdl_ref].primitive_type, 0, models[mdl_ref].draw_cnt);
		glLineWidth(1.f);
		break;
	case fill:
		glDrawArrays(models[mdl_ref].primitive_type, 0, models[mdl_ref].draw_cnt);
		break;
	case point:
		glPointSize(10.f);
		glDrawArrays(models[mdl_ref].primitive_type, 0, models[mdl_ref].draw_cnt);
		glPointSize(1.f);
		break;
	}
	
	//cleanup
	glBindVertexArray(0);
	shdrpgms[shd_ref].UnUse();
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
	angle_disp += static_cast<float>(angle_speed * delta_time);

	//translation matrix
	glm::mat3 transmtx = glm::mat3(1.f);
	transmtx[2] = glm::vec3(position.x, position.y, 1.f);

	//rotation matrix
	glm::mat3 rotmtx = glm::mat3(1.f);
	rotmtx[0] = glm::vec3(std::cos(angle_disp), -std::sin(angle_disp), 0.f);
	rotmtx[1] = glm::vec3(std::sin(angle_disp), std::cos(angle_disp), 0.f);

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
// Part 1: initialize OpenGL state ...
	// clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	if (GLHelper::keystateP==GL_TRUE)
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

	// Part 2: Spawn or kill objects ...
	// Check if left mouse button is pressed

	if (GLHelper::mousestateLeft == GLFW_TRUE)
	{
		GLint max_size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size); //maximum spawned objects
		// If maximum object limit is not reached, spawn new object(s)
		
		if (static_cast<GLint>(objects.size() * 2) <= max_size && spawn_mode)
		{
			increase_count = objects.size(); //increase amount = objects size

			for (int i = 0; i < increase_count || objects.size() == 0; ++i)
			{
				//Add objects
				GLObject newobj{};
				newobj.init();
				
				if (newobj.mdl_ref == 0) //if its a box, ++count of box
				{
					box_count++;
				}
				else if (newobj.mdl_ref == 1) //else ++ count of mystery
				{
					mystery_count++;
				}
				objects.emplace_back(newobj);
			}
		}
		// Otherwise, kill oldest objects
		else
		{
			spawn_mode = false;						
			decrease_count = objects.size() / 2;	//delete half of total objects
			
			for (int i = 0; i < decrease_count; ++i)
			{
				if (objects.front().mdl_ref == 0)	//if its a box, decrease box count
				{
					box_count--;
				}
				else								//if its not a box, decrease mystery count
				{
					mystery_count--;
				}
				objects.pop_front();				//erase object from front of object list
			}
		}

		if (objects.size() == 1) //if only 1 object remains, change back to spawn objects
		{
			spawn_mode = true;
		}

		//ensure count does not go to negative
		if (mystery_count < 0) 
		{
			mystery_count = 0;
		}
		if (box_count < 0)
		{
			box_count = 0;
		}
		GLHelper::mousestateLeft = GLFW_RELEASE;
	}

	// Part 3:
	// for each object in container GLApp::objects
	// Update object's orientation
	// A more elaborate implementation would animate the object's movement
	// A much more elaborate implementation would animate the object's size
	// Using updated attributes, compute world-to-ndc transformation matrix
	for (auto& object : objects) 
	{
		object.update(GLHelper::delta_time);
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
	std::string title = "Tutorial 3 | Benjamin Lee | Obj: " + std::to_string(objects.size()) + " | Box: " + std::to_string(box_count) +
		" | Mystery: " + std::to_string(mystery_count) +
		" | " + std::to_string(GLHelper::fps).substr(0, 5);//fps count
	glfwSetWindowTitle(GLHelper::ptr_window, title.c_str());

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
	for (auto const& x : GLApp::objects) {
		x.draw(); // call member function GLObject::draw()
	}
}

void GLApp::cleanup() {
	// empty for now
}

//rng for colours
std::random_device rd;
std::mt19937 box(rd());
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f);

/**

@brief Creates a box model.

This function creates a box model by defining the vertex position and color attributes, generating

Vertex Buffer Objects (VBOs) and allocating storage for the position and color data, generating a

Vertex Array Object (VAO) and binding the attributes, generating an Index Buffer Object (IBO) and

allocating storage for the index data. Finally, it creates and initializes a GLModel object with the

necessary information and returns it.

@return The created GLModel object representing a box.
*/
GLApp::GLModel GLApp::box_model()
{
	// Step 1: Define vertex pos and colour attributes
	std::vector<glm::vec2> pos_vtx
	{
	{ -0.5f, -0.5f },   // Bottom-left vertex
	{ 0.5f, -0.5f },    // Bottom-right vertex
	{ 0.5f, 0.5f },     // Top-right vertex
	{ -0.5f, 0.5f },    // Top-left vertex
	{ -0.5f, -0.5f },   // Bottom-left vertex of the second triangle
	{ 0.5f, 0.5f }      // Top-right vertex of the second triangle
	};

	std::vector<glm::vec3> clr_vtx
	{
		{ dis_color(box), dis_color(box), dis_color(box) },
		{ dis_color(box), dis_color(box), dis_color(box) },
		{ dis_color(box), dis_color(box), dis_color(box) },
		{ dis_color(box), dis_color(box), dis_color(box) },
		{ dis_color(box), dis_color(box), dis_color(box) },
		{ dis_color(box), dis_color(box), dis_color(box) },
	};

	// Step 2: mysteryerate Vertex Buffer Objects (VBOs) and allocate storage for position and color data
	GLuint vbo_hdl, color_vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferData(vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_DRAW);

	glCreateBuffers(1, &color_vbo_hdl);
	glNamedBufferData(color_vbo_hdl, sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data(), GL_DYNAMIC_DRAW);

	// Step 3: mysteryerate Vertex Array Object (VAO) and bind attributes
	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 0);

	// Bind color attribute
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayVertexBuffer(vaoid, 1, color_vbo_hdl, 0, sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 1, 1);

	// Step 4: mysteryerate Index Buffer Object (IBO) and allocate storage for index data
	GLuint ibo_hdl;
	std::vector<GLuint> indices;
	for (int i = 0; i < static_cast<int>(pos_vtx.size()); ++i)
		indices.push_back(i);
	glCreateBuffers(1, &ibo_hdl);
	glNamedBufferData(ibo_hdl, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);

	// Create and initialize the GLModel object
	GLApp::GLModel mdl{ 0 };
	mdl.vaoid = vaoid;
	mdl.primitive_type = GL_TRIANGLE_FAN; // Use GL_TRIANGLE_FAN to render a box
	mdl.draw_cnt = indices.size(); // Number of indices
	mdl.primitive_cnt = mdl.draw_cnt / 3; // Number of triangles (primitives)

	return mdl;
}

/**

@brief Creates a mystery model.

This function generates a mystery model by defining the vertex positions and colors for a lighting shape,

generating Vertex Buffer Objects (VBOs) and allocating storage for the position and color data, generating

an Index Buffer Object (IBO) and allocating storage for the index data, generating a Vertex Array Object (VAO),

and binding the attributes for position, color, and indices. Finally, it creates and initializes a GLModel

object with the necessary information and returns it.

@return The created GLModel object representing a mystery shape.
*/
GLApp::GLModel GLApp::mystery_model()
{
	// Step 1: mysteryerate vertex positions for a lighting shape
	std::vector<glm::vec2> pos_vtx
	{
		{0.f,0.f},{0.f,0.5f},{-0.25f,0.f},
		{0.f,0.f},{0.f,0.15f},{0.3f,0.25f},
		{0.f,0.f},{0.f,-0.5f},{0.3f,0.25f}
	};

	std::vector<glm::vec3> clr_vtx;
	for (size_t i = 0; i < pos_vtx.size(); ++i)
	{
		// mysteryerate random color for each vertex
		glm::vec3 random_color(dis_color(box), dis_color(box), dis_color(box));
		clr_vtx.push_back(random_color);
	}
	
	// Step 2: mysteryerate Vertex Buffer Objects (VBOs) and allocate storage for position and color data
	GLuint pos_vbo_hdl, color_vbo_hdl;
	glCreateBuffers(1, &pos_vbo_hdl);
	glNamedBufferData(pos_vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_DRAW);

	glCreateBuffers(1, &color_vbo_hdl);
	glNamedBufferData(color_vbo_hdl, sizeof(glm::vec3) * clr_vtx.size(), clr_vtx.data(), GL_DYNAMIC_DRAW);

	// Step 3: mysteryerate Index Buffer Object (IBO) and allocate storage for index data
	GLuint ibo_hdl;
	std::vector<GLuint> indices;
	for (int i = 0; i < static_cast<int>(pos_vtx.size()); ++i)
		indices.push_back(i);
	glCreateBuffers(1, &ibo_hdl);
	glNamedBufferData(ibo_hdl, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

	// Step 4: mysteryerate Vertex Array Object (VAO) and bind attributes for position, color, and indices
	GLuint vaoid;
	glCreateVertexArrays(1, &vaoid);

	// Bind position attribute
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayVertexBuffer(vaoid, 0, pos_vbo_hdl, 0, sizeof(glm::vec2));
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 0, 0);

	// Bind color attribute
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayVertexBuffer(vaoid, 1, color_vbo_hdl, 0, sizeof(glm::vec3));
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vaoid, 1, 1);

	// Bind index buffer
	glVertexArrayElementBuffer(vaoid, ibo_hdl);

	// Unbind VAO
	glBindVertexArray(0);

	// Create and initialize the GLModel object
	GLApp::GLModel mdl{};
	mdl.vaoid = vaoid;
	mdl.primitive_type = GL_TRIANGLE_STRIP; // Use GL_TRIANGLE_FAN to render the shape
	mdl.draw_cnt = pos_vtx.size(); // number of vertices
	mdl.primitive_cnt = mdl.draw_cnt / 3; // number of triangles

	return mdl;
}

/**

@brief Initializes additional models for the GLApp.

This function adds the models generated by the box_model() and mystery_model() functions to the models

vector of the GLApp. These models can then be used for rendering in the application.
*/
void GLApp::init_models_cont() {
	GLApp::models.emplace_back(GLApp::box_model());
	GLApp::models.emplace_back(GLApp::mystery_model());
}

/**

@brief Initializes the shader programs container for the GLApp.

This function initializes the shader programs container (shdrpgms) for the GLApp by compiling, linking, and

validating the shader programs specified in the vpss parameter.

@param vpss A vector of pairs representing the vertex and fragment shader file paths for each shader program.
*/
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



