/*!
@file		glpbo.cpp
@author		pghali@digipen.edu
@co-author	benjaminzhiyuan.lee@digipen.edu
@date		19/06/2023

Added emulate, draw_fullwindow_quad, init, setup_quad_vao, setup_shdrpgm, cleanup, set_clear_color overload, clear_color_buffer functions implementations.

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include "glpbo.h"
#include <algorithm> // for std::fill
#include <array>
#include <glhelper.h>
#include <iomanip> // setprecision
#include <dpml.h>


/*                                                   objects with file scope
----------------------------------------------------------------------------- */
// Initialize static members
GLsizei GLPbo::width = 0;
GLsizei GLPbo::height = 0;
GLsizei GLPbo::pixel_cnt = 0;
GLsizei GLPbo::byte_cnt = 0;
GLPbo::Color* GLPbo::ptr_to_pbo = nullptr;
GLuint GLPbo::vaoid = 0;
GLuint GLPbo::elem_cnt = 0;
GLuint GLPbo::pboid = 0;
GLuint GLPbo::texid = 0;
GLSLShader GLPbo::shdr_pgm;
GLPbo::Color GLPbo::clear_clr;

static const float color_transition_time = 2.0f;    //color transit per seconds
static float elapsed_time = 0.f;                    //application time passed


/**
 * @brief Emulates the PBO.
 */
void GLPbo::emulate() 
{

	ptr_to_pbo = reinterpret_cast<GLPbo::Color*>(glMapNamedBuffer(pboid, GL_WRITE_ONLY));

		
	GLPbo::clear_color_buffer();


	glUnmapNamedBuffer(pboid);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboid);
	// copy image data from client memory to GPU texture buffer memory
	glTextureSubImage2D(texid, 0, 0, 0, width, height,
		GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	static GLubyte start_color[4] = { 255, 255, 255, 255 };
	static GLubyte end_color[4] = { 0, 0, 0, 255 };
	static GLubyte current_color[4] = { 0, 0, 0, 255 };

	// Time passed for color transition counter
	elapsed_time += static_cast<float>(GLHelper::delta_time);

	// Transition color for every elapsed_time greater than color_transition_time
	if (elapsed_time > color_transition_time) 
	{
		elapsed_time = 0.f;

		// Set the new start color to the previous end color
		for (int i = 0; i < 4; ++i) 
		{
			start_color[i] = end_color[i];
		}

		// Generate a new random end color
		for (int i = 0; i < 3; ++i) 
		{
			end_color[i] = static_cast<GLubyte>(std::rand() % 256);
		}
		end_color[3] = 255;
	}

	// Interpolate between start color and end color
	for (int i = 0; i < 4; ++i) 
	{
		current_color[i] = start_color[i] + static_cast<GLubyte>((elapsed_time / color_transition_time) * (end_color[i] - start_color[i]));
	}


	std::stringstream fps;
	fps << std::fixed << std::setprecision(2) << GLHelper::fps;
	std::string fps_string = fps.str();

	// Print to window title bar
	std::string title = "Tutorial 6 | Benjamin Lee | PBO size: " + std::to_string(GLHelper::width) + " x " + std::to_string(GLHelper::height) + " | FPS: " + fps_string;
	glfwSetWindowTitle(GLHelper::ptr_window, title.c_str());

	
	set_clear_color(current_color[0], current_color[1], current_color[2], current_color[3]);

	clear_color_buffer();
}


/**
 * @brief Draws a full-window quad using the current texture and shader program.
 */
void GLPbo::draw_fullwindow_quad() 
{
	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texid);

	// Use the shader program
	shdr_pgm.Use();

	glUniform1i(glGetUniformLocation(shdr_pgm.GetHandle(), "tex"), 0);  // Set the texture unit to 0
	glActiveTexture(GL_TEXTURE0);  // Activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, texid);

	// Bind the VAO and draw the quad
	glBindVertexArray(vaoid);
	// here, we're saying what primitive is to be rendered and how many
   // such primitives exist.
   // the graphics driver knows where to get the indices because the VAO
   // containing this state information has been made current ...
	glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(elem_cnt), GL_UNSIGNED_SHORT, nullptr);

	// Unbind the VAO and the shader program
	glBindVertexArray(0);
	glUseProgram(0);

	

	// Unbind the texture object
	glBindTexture(GL_TEXTURE_2D, 0);
}
/**
 * @brief Initializes the GLPbo.
 * @param w The width of the PBO.
 * @param h The height of the PBO.
 */
void GLPbo::init(GLsizei w, GLsizei h) 
{
	std::string filename = "../meshes/cube.obj";
	bool load_nml_coord_flag = true;
	bool load_tex_coord_flag = false; // Set to true if you want to load texture coordinates
	bool model_centered_flag = true;

	// Create an instance of the Model struct
	GLPbo::Model cube;

	// Call the parse_obj_mesh() function
	bool success = DPML::parse_obj_mesh(filename, cube.pm, cube.nml, cube.tex, cube.tri,
		load_nml_coord_flag, load_tex_coord_flag, model_centered_flag);

	if (success)
	{
		std::cout << "obj parsed successfully";
	}
	else std::cout << "obj failed to parse";

	width = w;
	height = h;
	pixel_cnt = width * height;
	byte_cnt = pixel_cnt * sizeof(Color);

	// Set the PBO fill color
	set_clear_color(Color(255, 255, 255, 255));

	// Create a texture object
	glCreateTextures(GL_TEXTURE_2D, 1, &texid);
	glTextureStorage2D(texid, 1, GL_RGBA8, width, height);

	// Create a PBO
	glCreateBuffers(1, &pboid);
	glNamedBufferStorage(pboid, byte_cnt, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	// Setup the VAO for the full-window quad
	setup_quad_vao();

	// Setup the shader program
	setup_shdrpgm();
}

/**
 * @brief Sets up the vertex array object (VAO) for the full-window quad.
 */
void GLPbo::setup_quad_vao() 
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
	 Vertex{ glm::vec2(1.0f, -1.0f), glm::vec3(1.f, 0.f, 0.f),glm::vec2(1.f, 0.f)},
	 Vertex{ glm::vec2(1.0f, 1.0f), glm::vec3(0.f, 1.f, 0.f),glm::vec2(1.f, 1.f)},
	 Vertex{ glm::vec2(-1.0f, 1.0f), glm::vec3(0.f, 0.f, 1.f),glm::vec2(0.f, 1.f)},
	 Vertex{ glm::vec2(-1.0f, -1.0f), glm::vec3(0.5f, 0.2f, 1.f),glm::vec2(0.f, 0.f)}
	};


	// Create a single vertex buffer object (VBO) to store the vertex data
	GLuint vbo_hdl;
	glCreateBuffers(1, &vbo_hdl);
	glNamedBufferStorage(vbo_hdl, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_STORAGE_BIT);

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

	// Define the indices of the vertices
	std::array<GLushort, 6> indices = {
	 0,1,2,2,3,0
	};
	elem_cnt = indices.size();

	// Create an element buffer object (EBO) to store the indices
	GLuint ebo_hdl;
	glCreateBuffers(1, &ebo_hdl);
	glNamedBufferStorage(ebo_hdl, sizeof(GLushort)* indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vaoid, ebo_hdl);

	// Cleanup
	glBindVertexArray(0);
}
/**
 * @brief Sets up the shader program for rendering.
 */
void GLPbo::setup_shdrpgm() {
	// Define the vertex shader source code
	std::string vertexShaderSource = R"(
        #version 450 core
        layout (location=0) in vec2 VertexPosition;
        layout (location=1) in vec3 VertexColor;
        layout (location=0) out vec3 vColor;
		layout (location=2) in vec2 TexCoord;
		layout (location=1) out vec2 vTexCoord;  // Add this line to declare the output variable

        void main() {
            gl_Position = vec4(VertexPosition, 0.0, 1.0);
            vColor = VertexColor;
			vTexCoord = TexCoord;  // Pass the texture coordinates as an output
        }
    )";
	// Define the fragment shader source code
	std::string fragmentShaderSource = R"(
    #version 450 core
    layout (location=0) in vec3 vInterpColor;
    layout(location=1) in vec2 vTexCoord;
    layout (location=0) out vec4 fFragColor;
    uniform sampler2D tex;  // Add this line to declare the texture sampler
    void main () {
        fFragColor = texture(tex, vTexCoord);  // Sample the texture with the texture coordinates
    }
	)";

	//start linking
	shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, vertexShaderSource);
	shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, fragmentShaderSource);

	if (GL_FALSE == shdr_pgm.Link()) {
		std::cout << "Unable to compile/link/validate shader programs" << "\n";
		std::cout << shdr_pgm.GetLog() << std::endl;
		std::exit(EXIT_FAILURE);
	}

}

/**
 * @brief Cleans up and deletes the PBO, texture, and VAO.
 */
void GLPbo::cleanup() {
	glInvalidateBufferData(vaoid);
	glInvalidateBufferData(texid);
	glInvalidateBufferData(pboid);
	// Delete the PBO
	glDeleteBuffers(1, &pboid);

	// Delete the texture
	glDeleteTextures(1, &texid);

	// Delete the VAO
	glDeleteVertexArrays(1, &vaoid);
}

/**
 * @brief Sets the clear color for the PBO.
 * @param color The color to set.
 */
void GLPbo::set_clear_color(GLPbo::Color color) {
	clear_clr = color;
}
/**
 * @brief Sets the clear color for the PBO.
 * @param r The red component of the color.
 * @param g The green component of the color.
 * @param b The blue component of the color.
 * @param a The alpha component of the color.
 */
void GLPbo::set_clear_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
	clear_clr.rgba.r = r;
	clear_clr.rgba.g = g;
	clear_clr.rgba.b = b;
	clear_clr.rgba.a = a;
}

/**
 * @brief Clears the color buffer of the PBO with the current clear color.
 */
void GLPbo::clear_color_buffer() {
	if (ptr_to_pbo) {
		std::fill(ptr_to_pbo, ptr_to_pbo + pixel_cnt, clear_clr);
	}
}
