/* !
@file    glapp.h
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		15/05/2023
@ammendent	Added declarations for points_model, lines_model,trifans_model and
			tristrip_model.

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

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLAPP_H
#define GLAPP_H

/*                                                                   includes
----------------------------------------------------------------------------- */

struct GLApp {
  
  static void init();
  static void update();
  static void draw();
  static void cleanup();
//encapsulates state required to render a geometrical model
  struct GLModel {
	GLenum primitive_type{ 0 }; // same as tutorial 1
	GLuint primitive_cnt{ 0 }; // added for tutorial 2
	GLuint vaoid{ 0 }; // same as tutorial 1
	GLuint draw_cnt{ 0 }; // added for tutorial 2
	GLSLShader shdr_pgm; // same as tutorial 1
	void setup_shdrpgm(std::string vtx_shdr, // added to tutorial 2
	std::string frg_shdr);
	void draw(); // same as tutorial 1
};
//static GLModel mdl; // removed for tutorial 2
// container for different types of geometries required in tutorial 2
static std::vector<GLModel> models;
// tutorial 2's replacement for setup_vao for GL_POINT primitives
static GLApp::GLModel points_model(int slices,int stacks, std::string vtx_shdr,
std::string frg_shdr);

static GLApp::GLModel lines_model(int slices, int stacks,
	std::string vtx_shdr, std::string frg_shdr);

static GLApp::GLModel trifans_model(int slices, std::string vtx_shdr,
	std::string frg_shdr);

static GLApp::GLModel tristrip_model(int slices, int stacks,
	std::string vtx_shdr, std::string frg_shdr);

  struct GLViewport {
	  GLint x, y;
	  GLsizei width, height;
  };
  static std::vector<GLViewport> vps; // container for viewports
  // data member to represent geometric model to be rendered
  // C++ requires this object to have a definition in glapp.cpp!!!
};

#endif /* GLAPP_H */
