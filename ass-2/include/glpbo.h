/* !
@file       glpbo.h
@author     pghali@digipen.edu

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		19/07/2023

This file contains the declaration of structure GLPbo that encapsulates the
functionality required to stream images generated by a 3D graphics pipe 
emulator executed on the CPU for display by the GPU.

See the assignment specs for details ...
*//*__________________________________________________________________________*/

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLPBO_H
#define GLPBO_H

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <GL/glew.h> // for access to OpenGL API declarations 
#include <glhelper.h>
#include <glslshader.h> // GLSLShader class definition
#include <GLFW/glfw3.h> 
#include <dpml.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <cmath>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <random>
#include <chrono>
#include <cctype>
#include <unordered_map>

/*  _________________________________________________________________________ */
struct GLPbo
  /*! GLPbo structure to encapsulate 3D graphics pipe emulator which will write
  an image composed of 32-bit RGBA values to PBO, copy the image from PBO to image 
  store of a texture object, and finally use this texture image to render a 
  full-window quad that will display the emulator's output (phew).
  */
{
  // forward declaration
  union Color;

  //-------------- static member function declarations here -----------------
  
  // this is where the emulator does its work of emulating the graphics pipe
  // it generates images using set_pixel to write to the PBO
  static void emulate();
  
  // render quad using the texture image generated by emulate()
  // also prints appropriate information to window's title bar ...
  static void draw_fullwindow_quad();

  // initialization and cleanup stuff ...
  static void init(GLsizei w, GLsizei h);
  static void setup_quad_vao();
  static void setup_shdrpgm();
  static void cleanup();

  // ---------------static data members are declared here ----------------
  
  // Storage requirements common to emulator, PBO and texture object
  static GLsizei width, height; // dimensions of 
  
  // rather than computing these values many times, compute once in
  // GLPbo::init() and then forget ...
  static GLsizei pixel_cnt, byte_cnt; // how many pixels and bytes
  // pointer to PBO's data store - must be set every frame with  
  // the value returned by glMapNamedBuffer()
  static Color *ptr_to_pbo;

  // geometry and material information ...
  static GLuint vaoid;        // with GL 4.5, VBO & EBO are not required
  static GLuint elem_cnt;     // how many indices in element buffer
  static GLuint pboid;        // id for PBO
  static GLuint texid;        // id for texture object
  static GLSLShader shdr_pgm; // object that abstracts away nitty-gritty
                              // details of shader management

// --- here we're trying to emulate GL's functions for clearing colorbuffer ---

  // overloaded functions akin to glClearcolor that set GLPbo::clear_clr
  // with user-supplied parameters ...
  static void set_clear_color(GLPbo::Color);
  static void set_clear_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a = 255);

  // akin to glClear(GL_COLOR_BUFFER_BIT) - uses the pointer ptr_to_pbo
  // to get access to the PBO's data store ...
  static void clear_color_buffer();
  static void clear_depth_buffer();

  // what value to use for clearing color buffer?
  static Color clear_clr;

  union Color
    /*! glm doesn't have type unsigned char [4] - therefore we declare our
    own type.
    Since Color is such a common type name in graphics applications, make
    sure to encapsulate the name in scope GLPbo!!!
    */
  {
    struct 
    {
      GLubyte r, g, b, a; // a, b, g, r
    }rgba;
    GLubyte val[4]; // treat <r,g,b,a> as array of 8-bit unsigned values
    GLuint raw;     // treat <r,g,b,a> as 32-bit unsigned value

    Color(GLubyte re = 0, GLubyte gr = 0, GLubyte bl = 0, GLubyte al = 255) :
        rgba{ re, gr, bl, al }
    {
    }
  };

  // Data structure to store vertex position and triangle index arrays
// returned by DPML::parse_obj_mesh()
  struct Model {
      std::vector<glm::vec3> pm; // invariant
      // DPML::parse_obj_mesh() will return per-vertex normal coordinates
      // that you must map to color coordinates in range [0, 1]
      std::vector<glm::vec3> nml; // will contain per-vertex normal
      // coordinates which must then be
      // converted by you to RGB values
      std::vector<glm::vec2> tex; // not used in this submission
      std::vector<unsigned short> tri; // triangle indices
      // window coordinates in array pd are obtained after NDC coordinates in
      // array pm are transformed by rotation transform followed by
      // viewport transformation matrix
      std::vector<glm::vec3> pd;

      bool rotating = false;
      float angle = 0.0f;
      enum class task {
          wireframe = 0,
          depth,
          faceted,
          shaded,
          textured,
          faceted_tex,
          smooth_tex,
      } Tasking = GLPbo::Model::task::wireframe;

      glm::mat3 ModelTrans{};
      
     
  };
  static Model mdl;
     static void viewport_xform(Model& model);
     static void set_pixel(int x, int y, GLPbo::Color draw_clr);

    // set all pixels with same color draw_clr on line segment starting
    // at point P1(x1, y1) and ending at point P2(x2, y2)
    // Note: points are in window coordinates
        static void render_linebresenham(GLint x1, GLint y1,
      GLint x2, GLint y2, GLPbo::Color draw_clr);

    // Implements a flat shaded triangle rasterizer using edge equations
    // and top-left tie-breaking rule.
    // Whether fragment (x, y) is rasterized or not is based on the results of
    // point sampling the fragment at its center (x+0.5, y+0.5).
    // Top left tie-breaking rule must be implemented.
    // Parameters p0, p1, and p2 represent vertices of triangle whose x- and y
    // values are specified in window coordinates.
    // Front-facing triangles are assumed to have counterclockwise winding.
    // Back-facing triangles must be culled.
    // The clr parameter is specified by the caller with randomly generated
    // color coordinates in range [0, 1].
    // The function returns false if the triangle is back-facing; otherwise
    // the function returns true.
        static bool render_faceted_shading(glm::dvec3 const& p0, glm::dvec3 const& p1,
            glm::dvec3 const& p2, glm::dvec3 const& m0, glm::dvec3 const& m1, glm::dvec3 const& m2);

    // Implements a smooth shaded triangle rasterizer using edge equations,
    // top-left tie-breaking rule, and Barycentric interpolation.
    // Parameters p0, p1, and p2 represent vertices of triangle whose x- and y-
    // values are specified in window coordinates.
    // Front-facing triangles are assumed to have counterclockwise winding.
    // Back-facing triangles must be culled.
    // Parameters c0, c1, and c2 represent RGB color coordinates for vertices
    // p0, p1, and p2, respectively. Color coordinates are specified in
    // range [0, 1].
    // The function returns false if the triangle is back-facing; otherwise
    // the function returns true.
        static bool render_shadow_map(glm::dvec3 const& p0, glm::dvec3 const& p1,
            glm::dvec3 const& p2);

        static bool render_smooth_shading(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2, 
            glm::vec3 const& m0, glm::vec3 const& m1, glm::vec3 const& m2,
            glm::dvec3 const& n0, glm::dvec3 const& n1, glm::dvec3 const& n2);

        static void setup_texobj(std::string pathname);

        static bool render_texture_map(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2
            , glm::dvec2 const& tx0, glm::dvec2 const& tx1, glm::dvec2 const& tx2);

        static bool render_faceted_texture(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2
            , glm::dvec3 const& m0, glm::dvec3 const& m1, glm::dvec3 const& m2,
            glm::dvec2 const& tx0, glm::dvec2 const& tx1, glm::dvec2 const& tx2);

        static bool render_smooth_texture(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2
            , glm::vec3 const& m0, glm::vec3 const& m1, glm::vec3 const& m2,
            glm::dvec3 const& n0, glm::dvec3 const& n1, glm::dvec3 const& n2,
            glm::dvec2 const& tx0, glm::dvec2 const& tx1, glm::dvec2 const& tx2);
};

#endif /* GLPBO_H */
