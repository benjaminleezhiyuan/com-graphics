/* !
@file    glapp.h
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		10/05/2023
@ammendent	Added declarations for shader functions and GLModel srtuct.

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
  // previous existing declarations
  static void init();
  static void update();
  static void draw();
  static void cleanup();
// encapsulates state required to render a geometrical model
  struct GLModel {
	  GLenum primitive_type = GL_TRIANGLES; // which OpenGL primitive to be rendered?
	  GLSLShader shdr_pgm; // which shader program?
	  GLuint vaoid{ 0 }; // handle to VAO
	  GLuint idx_elem_cnt{ 0 }; // how many elements of primitive of type
	  // primitive_type are to be rendered
	  // member functions defined in glapp.cpp
	  void setup_vao();
	  void setup_shdrpgm();
	  void draw();
  };
  // data member to represent geometric model to be rendered
  // C++ requires this object to have a definition in glapp.cpp!!!
  static GLModel mdl;

};

#endif /* GLAPP_H */
