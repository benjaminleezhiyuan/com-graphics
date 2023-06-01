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
#include <GLFW/glfw3.h>

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

  struct GLModel {
	  GLenum primitive_type;
	  GLuint primitive_cnt;
	  GLuint vaoid;
	  GLuint draw_cnt;
	  // you could add member functions for convenience if you so wish ...
	  void init(std::string model_file_name); // read mesh data from file ...
	  void release(); // return buffers back to GPU ...
  };

  struct GLObject {
	  std::string name{};
	  glm::vec2 scaling{ };
	  glm::vec2 orientation{};
	  glm::vec2 position{};
	  glm::vec3 color{};
	  glm::mat3 mdl_xform{}; // model (model-to-world) transform
	  glm::mat3 mdl_to_ndc_xform{}; // model-to-NDC transform
	  std::map<std::string, GLApp::GLModel>::iterator mdl_ref;
	  std::map<std::string, GLSLShader>::iterator shd_ref;
	  // you can implement them as in tutorial 3 ...
	  void init();
	  void draw() const;
	  //void update();
	  void update(GLdouble delta_time);
  };

  static std::map<std::string, GLSLShader> shdrpgms; // singleton
  static std::map<std::string, GLModel> models; // singleton
  static std::map<std::string, GLObject> objects; // singleton

  // function to insert shader program into container GLApp::shdrpgms ...
  static void init_shdrpgms(std::string, std::string, std::string);
  // function to parse scene file ...
  static void init_scene(std::string);


  struct Camera2D {
	  GLObject* pgo{}; // pointer to game object that embeds camera
	  glm::vec2 right{}, up{};
	  glm::mat3 view_xform{}, camwin_to_ndc_xform{}, world_to_ndc_xform{};
	  // window parameters ...
	  GLint height{ 1000 };
	  GLfloat ar{};
	  // window change parameters ...
	  GLint min_height{ 500 }, max_height{ 2000 };
	  // height is increasing if 1 and decreasing if -1
	  GLint height_chg_dir{ 1 };
	  // increments by which window height is changed per Z key press
	  GLint height_chg_val{ 5 };
	  // camera's speed when button U is pressed
	  GLfloat linear_speed{ 2.f };
	  // keyboard button press flags
	  GLboolean camtype_flag{ GL_FALSE }; // button V
	  GLboolean zoom_flag{ GL_FALSE }; // button Z
	  GLboolean left_turn_flag{ GL_FALSE }; // button H
	  GLboolean right_turn_flag{ GL_FALSE }; // button K
	  GLboolean move_flag{ GL_FALSE }; // button U
	  // you can implement these functions as you wish ...
	  void init(GLFWwindow* win, GLObject* ptr);
	  void update(GLFWwindow*);
  };
  // define object of type Camera2D ...
  static Camera2D cam;

};

#endif /* GLAPP_H */
