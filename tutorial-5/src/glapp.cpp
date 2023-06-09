/*!
@file    glapp.cpp
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		05/06/2023

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <iostream>
#include <cstddef>
#include <fstream>
#include <glapp.h>
#include <glhelper.h>
#include <array>


struct GLApp::GLModel GLApp::mdl;
static GLboolean modulation = false;
static GLboolean alphablend = false;
static float elapsed_time = 0.f;

enum tasks
{
	zero,
	one,
	two,
	three,
	four,
	five,
	six
};
static tasks task = zero;

GLuint setup_texobj(std::string pathname) {
	GLuint width = 256;
	GLuint height = 256;
	GLuint bytes_per_texel = 4;

	// Open the image file
	std::ifstream file(pathname, std::ios::binary);
	if (!file) {
		// Handle error when the file cannot be opened
		std::cerr << "Failed to open image file: " << pathname << std::endl;
		return 0;  // Return an invalid texture handle
	}

	// Calculate the total size of the image data
	size_t image_size = width * height * bytes_per_texel;

	// Allocate memory to store the image data
	GLubyte* ptr_texels = new GLubyte[image_size];

	// Read the image data from the file
	file.read(reinterpret_cast<char*>(ptr_texels), image_size);

	// Close the file
	file.close();

	GLuint texture_obj;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture_obj);


	glTextureStorage2D(texture_obj, 1, GL_RGBA8, width, height);
	glTextureSubImage2D(texture_obj, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, ptr_texels);

	delete[] ptr_texels;

	return texture_obj;
}

void GLApp::init() {
	// Part 1: clear colorbuffer with RGBA value in glClearColor ...
	glClearColor(1.f, 0.f, 0.f, 1.f);
	// Part 2: use the entire window as viewport ...
	glViewport(0, 0, GLHelper::width, GLHelper::height);
	mdl.texture_obj = setup_texobj("../images/duck-rgba-256.tex");
	// Part 3: initialize VAO and create shader program
	mdl.setup_vao();
	mdl.setup_shdrpgm();
}

void GLApp::update() 
{
	if (GLHelper::keystateM) {
		modulation ^= 1;
		GLHelper::keystateM = GL_FALSE;
	}
	else if (GLHelper::keystateA) {
		alphablend ^= 1;
		GLHelper::keystateA = GL_FALSE;
	}
	if (alphablend) {
		// Enable blending
		glEnable(GL_BLEND);

		// Set the blend function
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else {
		glDisable(GL_BLEND);
	}
	//cycle the task with Button T
	if (GLHelper::keystateT) {
		// Cycle through tasks
		switch (task) {
		case zero:
			task = one;
			break;
		case one:
			task = two;
			break;
		case two:
			task = three;
			break;
		case three:
			task = four;
			break;
		case four:
			task = five;
			break;
		case five:
			task = six;
			break;
		case six:
			task = zero;
			break;
		}
		GLHelper::keystateT = GL_FALSE;
	}
}
 
void GLApp::draw() {
	// clear back buffer as before
	glClear(GL_COLOR_BUFFER_BIT);
	// now, render rectangular model from NDC coordinates to viewport
	mdl.draw();

	std::string task_name = "";
	if (task == zero)
		task_name = "Task 0: Paint Color ";
	else if (task == one)
		task_name = "Task 1: Fixed-Size Checkerboard ";
	else if (task == two)
		task_name = "Task 2: Animated Checkerboard ";
	else if (task == three)
		task_name = "Task 3: Texture Mapping ";
	else if (task == four)
		task_name = "Task 4: Repeating ";
	else if (task == five)
		task_name = "Task 5: Mirroring ";
	else if (task == six)
		task_name = "Task 6: Clamping ";

	std::string alpha_blend_string = "";
	if (alphablend)
		alpha_blend_string = "ON ";
	else  alpha_blend_string = "OFF ";

	std::string modulate_string = "";
	if (modulation)
		modulate_string = "ON";
	else  modulate_string = "OFF";

	// Print to window title bar
	std::string title = "Tutorial 5 | Benjamin Lee | " + task_name + "| Alpha Blend: " + alpha_blend_string + "| Modulate: " + modulate_string;
	glfwSetWindowTitle(GLHelper::ptr_window, title.c_str());
}

void GLApp::cleanup() {
  // empty for now
}

void GLApp::GLModel::setup_vao() 
{
	// Define the vertex structure
	struct Vertex 
	{
		glm::vec2 position;
		glm::vec3 color;
		glm::vec2 texture;
	};

	// Define the vertices using the array of structure format
	std::array<Vertex, 4> vertices = 
	{
		Vertex{ glm::vec2(1.f, -1.f), glm::vec3(1.f, 0.f, 0.f),glm::vec2(1.f, 0.f)},
		Vertex{ glm::vec2(1.f, 1.f), glm::vec3(0.f, 1.f, 0.f),glm::vec2(1.f, 1.f)},
		Vertex{ glm::vec2(-1.f, 1.f), glm::vec3(0.f, 0.f, 1.f),glm::vec2(0.f, 1.f)},
		Vertex{ glm::vec2(-1.f, -1.f), glm::vec3(0.5f, 0.2f, 1.f),glm::vec2(0.f, 0.f)}
	};

	// Create a single vertex buffer object (VBO) to store the vertex data
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(Vertex)* vertices.size(), vertices.data(), GL_DYNAMIC_STORAGE_BIT);

	// Encapsulate information about the contents of VBO and VBO handle into a VAO
	glCreateVertexArrays(1, &vaoid);

	// Specify the vertex position attribute
	glEnableVertexArrayAttrib(vaoid, 0);
	glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
	glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, offsetof(Vertex, position), sizeof(Vertex));
	glVertexArrayAttribBinding(vaoid, 0, 0);

	// Specify the vertex color attribute
	glEnableVertexArrayAttrib(vaoid, 1);
	glVertexArrayAttribFormat(vaoid, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
	glVertexArrayVertexBuffer(vaoid, 1, vbo_hdl, offsetof(Vertex, color), sizeof(Vertex));
	glVertexArrayAttribBinding(vaoid, 1, 0);

	// Specify the texture attribute
	glEnableVertexArrayAttrib(vaoid, 2);
	glVertexArrayAttribFormat(vaoid, 2, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, texture));
	glVertexArrayVertexBuffer(vaoid, 2, vbo_hdl, offsetof(Vertex, texture), sizeof(Vertex));
	glVertexArrayAttribBinding(vaoid, 2, 0);

	primitive_type = GL_TRIANGLE_STRIP;

	// Define the indices of the vertices
	std::array<GLushort, 6> indices = {
		0,1,2,2,3,0
	};
	idx_elem_cnt = indices.size();

	// Create an element buffer object (EBO) to store the indices
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort)* indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	// Cleanup
	glBindVertexArray(0);
}

void GLApp::GLModel::setup_shdrpgm() {
	std::vector<std::pair<GLenum, std::string>> shdr_files;
	shdr_files.emplace_back(std::make_pair(
		GL_VERTEX_SHADER,
		"../shaders/my-tutorial-5.vert"));
	shdr_files.emplace_back(std::make_pair(
		GL_FRAGMENT_SHADER,
		"../shaders/my-tutorial-5.frag"));
	shdr_pgm.CompileLinkValidate(shdr_files);
	if (GL_FALSE == shdr_pgm.IsLinked()) {
		std::cout << "Unable to compile/link/validate shader programs" << "\n";
		std::cout << shdr_pgm.GetLog() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

float ease(float delta_time) {
	// Calculate the animation parameters
	static const float animation_duration = 30.0f;  // Total duration of the animation in Firsts
	static const float size_min = 16.0f;  // Minimum tile size
	static const float size_max = 256.0f;  // Maximum tile size

	// Calculate the current time in the animation from 0.0 to 1.0
	float t = glm::clamp(elapsed_time / animation_duration, 0.0f, 1.0f);

	// Apply ease-in/ease-out function to t
	float t_prime = t * t * (3.0f - 2.0f * t);

	// Calculate the current tile size using the interpolated value
	float current_size;
	if (t < 0.5f) {
		// Ease in phase
		current_size = size_min + t_prime * (size_max - size_min);
	}
	else {
		// Ease out phase
		float reverse_t = 1.0f - t;
		float reverse_t_prime = reverse_t * reverse_t * (3.0f - 2.0f * reverse_t);
		current_size = size_min + reverse_t_prime * (size_max - size_min);
	}

	// Increase the elapsed time for the animation
	elapsed_time += delta_time;

	// Check if the animation has reached the end
	if (elapsed_time >= animation_duration) {
		// Reset the elapsed time and adjust the current size to ensure a smooth transition
		elapsed_time -= animation_duration;
		current_size = size_min;
	}

	return current_size;
}

void GLApp::GLModel::draw() {
	// suppose texture object is to use texture image unit 0
	glBindTextureUnit(0, mdl.texture_obj);

	shdr_pgm.Use();

	// there are many models, each with their own initialized VAO object
	// here, we're saying which VAO's state should be used to set up pipe
	glBindVertexArray(vaoid);

	// tell fragment shader sampler uTex2d will use texture image unit 6
	GLuint tex_loc = glGetUniformLocation(shdr_pgm.GetHandle(), "uTex2d");
	glUniform1i(tex_loc, 0);


	// Set the modulationEnabled uniform in the shader
	GLboolean modulationEnabledLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "modulation") ? GL_TRUE : GL_FALSE;
	glUniform1i(modulationEnabledLoc, modulation);

	// Set the size uniform in the shader
	GLuint sizeLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "size");


	// Set the size uniform in the shader
	GLuint taskLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "task");

	// Set the texture scaling uniform in the shader
	GLuint textureScaleLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "textureScale");

	GLfloat texture_scale = 0.0f;


	// Calculate the tile current size based on the task
	float current_size = 0.0f;

	//cycle the task with Button T
	switch (task) {
	case zero:
		elapsed_time = 0.0f;//reset animation for Task Second
		//Rectangle
		break;
	case one:
		//Checkered Pattern
		current_size = 30.0f;

		break;
	case two:

		//Checkered Pattern with ease in and ease out
		current_size = ease((float)GLHelper::delta_time);

		break;
	case three:
		//Single Duck
		texture_scale = 1.0f;


		elapsed_time = 0.0f;//reset animation for Task Second
		break;


	case four:
		//4x4 Duck image
		// Set the texture wrapping parameters to GL_REPEAT mode
		glTextureParameteri(mdl.texture_obj, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(mdl.texture_obj, GL_TEXTURE_WRAP_T, GL_REPEAT);
		texture_scale = 4.0f;
		break;

		break;
	case five:
		//mirror 4x4
		// Set the texture wrapping parameters to GL_MIRRORED_REPEAT mode
		glTextureParameteri(mdl.texture_obj, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTextureParameteri(mdl.texture_obj, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		texture_scale = 4.0f;
		break;
	case six:
		//clamp duck image to bottom left
		// Set the texture wrapping parameters to GL_CLAMP_TO_EDGE mode
		glTextureParameteri(mdl.texture_obj, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(mdl.texture_obj, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		texture_scale = 4.0f;
		break;
	}

	glUniform1f(textureScaleLoc, texture_scale); // Set the texture scale value

	glUniform1f(sizeLoc, current_size);

	glUniform1i(modulationEnabledLoc, modulation);

	glUniform1i(taskLoc, task);

	// here, we're saying what primitive is to be rendered and how many
	// such primitives exist.
	// the graphics driver knows where to get the indices because the VAO
	// containing this state information has been made current ...
	glDrawElements(primitive_type, static_cast<GLsizei>(idx_elem_cnt), GL_UNSIGNED_SHORT, nullptr);
	// after completing the rendering, we tell the driver that VAO
	// vaoid and current shader program are no longer current
	glBindVertexArray(0);

	// Unbind the texture object
	glBindTexture(GL_TEXTURE_2D, 0);

	shdr_pgm.UnUse();

}


