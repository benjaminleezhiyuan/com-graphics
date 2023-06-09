/* !
@file    glhelper.h
@author  pghali@digipen.edu
@date    10/11/2016

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		26/05/2023
@ammendent	added keystate for V,Z,H,K and U

This file contains the declaration of namespace Helper that encapsulates the
functionality required to create an OpenGL context using GLFW; use GLEW
to load OpenGL extensions; initialize OpenGL state; and finally initialize
the OpenGL application by calling initalization functions associated with
objects participating in the application.

*//*__________________________________________________________________________*/

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLHELPER_H
#define GLHELPER_H

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <GL/glew.h> // for access to OpenGL API declarations 
#include <GLFW/glfw3.h>
#include <string>

/*  _________________________________________________________________________ */
struct GLHelper
  /*! GLHelper structure to encapsulate initialization stuff ...
  */
{
  static bool init(GLint w, GLint h, std::string t);
  static void cleanup();

  // callbacks ...
  static void error_cb(int error, char const* description) noexcept;
  static void fbsize_cb(GLFWwindow *ptr_win, int width, int height)noexcept;
  // I/O callbacks ...
// I/O callbacks ...
  static void key_cb(GLFWwindow*, int key, int scancode,
	  int action, int mod)noexcept;
  static void mousebutton_cb(GLFWwindow*, int button,
	  int action, int mod)noexcept;
  static void mousescroll_cb(GLFWwindow*,
	  double xoffset, double yoffset)noexcept;
  static void mousepos_cb(GLFWwindow*, double xpos, double ypos)noexcept;
  static GLboolean keystateP;
  static GLboolean keystateV;
  static GLboolean keystateZ;
  static GLboolean keystateH;
  static GLboolean keystateK;
  static GLboolean keystateU;
  static GLboolean keystateM;
  static GLboolean keystateT;
  static GLboolean keystateA;
  static GLboolean mousestateLeft;
  static void update_time(double fpsCalcInt = 1.0);

  static GLint width, height;
  static GLdouble fps;
  static GLdouble delta_time; // time taken to complete most recent game loop
  static std::string title;
  static GLFWwindow *ptr_window;

  static void print_specs();
};

#endif /* GLHELPER_H */
