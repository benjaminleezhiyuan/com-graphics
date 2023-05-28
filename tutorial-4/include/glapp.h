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

struct GLApp {
  
  static void init();
  static void update();
  static void draw();
  static void cleanup();
 
  // container for shader programs and helper function(s) ...
  static std::vector<GLSLShader> shdrpgms; // singleton
  using VPSS = std::vector<std::pair<std::string, std::string>>;
  static void init_shdrpgms_cont(VPSS const&); // initialize singleton
  // encapsulates geometry - we call this abstraction a model
  struct GLModel {
	  GLenum primitive_type;
	  GLuint primitive_cnt;
	  GLuint vaoid;
	  GLuint vboid;
	  GLuint cboid;
	  GLuint draw_cnt;
  };
  // container for models and helper function(s) ...
  static std::vector<GLApp::GLModel> models; // singleton
  static GLApp::GLModel box_model();
  static GLApp::GLModel mystery_model();
  static void init_models_cont(); // initialize singleton

  // encapsulates rendered object
  struct GLObject {
	  glm::vec2 scaling; // scaling
	  GLfloat angle_speed, angle_disp; // orientation
	  glm::vec2 position; // translation
	  glm::mat3 mdl_to_ndc_xform;
	  GLuint mdl_ref, shd_ref;
	  // set up initial state
	  void init();
	  void draw() const;
	  void update(GLdouble delta_time);
  };
  // container for objects ...
  static std::list<GLApp::GLObject> objects; // singleton
};

#endif /* GLAPP_H */
