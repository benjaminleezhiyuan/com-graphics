/* !
@file    glapp.h
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		26/05/2023
@ammendent	Updated the GLApp Struct to do instanced rendering

This file contains the declaration of namespace GLApp that encapsulates the
functionality required to implement an OpenGL application including 
compiling, linking, and validating shader programs
setting up geometry and index buffers, 
configuring VAO to present the buffered geometry and index data to
vertex shaders,
configuring textures (in later labs),
configuring cameras (in later labs), 
and transformations (in later labs).
*//*__________________________________________________________________________*/

#include <glslshader.h>
#include <list>

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLAPP_H
#define GLAPP_H

/*                                                                   includes
----------------------------------------------------------------------------- */

struct GLApp 
{
  static void init();
  static void update();
  static void draw();
  static void cleanup();
 
	struct GLModel 
	{
	GLenum primitive_type=GL_TRIANGLES;
	GLuint primitive_cnt=0;
	GLuint vaoid=0;
	GLuint draw_cnt=0;
	std::vector<glm::vec2> vtx_pos;
	std::vector<glm::vec3> index;
	// you could add member functions for convenience if you so wish ...
	void init(std::string model_name);	// read mesh data from file ...
	void release(); // return buffers back to GPU ...
	};

	struct GLObject {
	glm::vec2 scaling;
	glm::vec2 orientation;
	glm::vec2 position;
	glm::vec3 color;
	glm::mat3 mdl_xform; // model (model-to-world) transform
	glm::mat3 mdl_to_ndc_xform; // model-to-NDC transform
	GLObject() : scaling(1.0f, 1.0f), orientation(0.0f, 0.0f), position(0.0f, 0.0f), color(0.0f, 0.0f, 0.0f), mdl_xform(glm::mat3(1.0f)), mdl_to_ndc_xform(glm::mat3(1.0f)) {}
	std::map<std::string, GLApp::GLModel>::iterator mdl_ref;
	std::map<std::string, GLSLShader>::iterator shd_ref;
	// you can implement them as in tutorial 3 ...
	void init();
	void draw() const;
	//void update();
	void update(GLdouble time_per_frame);
	};
static std::map<std::string, GLSLShader> shdrpgms; // singleton
static std::map<std::string, GLModel> models; // singleton
static std::map<std::string, GLObject> objects; // singleton


// function to insert shader program into container GLApp::shdrpgms ...
static void init_shdrpgms(std::string shdr_pgm_name, std::string vtx_shdr, std::string frg_shdr);
// function to parse scene file ...
static void init_scene(std::string);

};

#endif /* GLAPP_H */
