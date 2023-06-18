/*!
@file    main-pbo.cpp
@author  pghali@digipen.edu
@date    10/11/2016

This file uses functionality defined in types GLHelper and GLPbo to
initialize an OpenGL context and implement a game loop.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <glhelper.h>
#include <glpbo.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
// Don't include glapp.h - we've nothing more to do with that file anymore!!!

/*                                                         type declarations
----------------------------------------------------------------------------- */

/*                                                      function declarations
----------------------------------------------------------------------------- */
static void draw();
static void update();
static void init();
static void cleanup();

/*                                                      function definitions
----------------------------------------------------------------------------- */
/*  _________________________________________________________________________ */
/*! main

@param none

@return int

Indicates how the program existed. Normal exit is signaled by a return value of
0. Abnormal termination is signaled by a non-zero return value.
Note that the C++ compiler will insert a return 0 statement if one is missing.
*/
int main() {
    // Part 1
    init();

    // Part 2
    while (!glfwWindowShouldClose(GLHelper::ptr_window)) {
        // Part 2a
        update();
        // Part 2b
        draw();
    }

    // Part 3
    cleanup();
}

/*  _________________________________________________________________________ */
/*! update
@param none
@return none

Uses GLHelper::GLFWWindow* to get handle to OpenGL context.
For now, there are no objects to animate nor keyboard, mouse button click,
mouse movement, and mouse scroller events to be processed.
*/
static void update() {
    // Part 1
    glfwPollEvents();

    // Part 2
    GLHelper::update_time(1.0);

    // Part 3
    GLPbo::emulate();
}

/*  _________________________________________________________________________ */
/*! draw
@param none
@return none

Uses GLHelper::GLFWWindow* to get handle to OpenGL context.
For now, there's nothing to draw - just paint color buffer with constant color
*/
static void draw() {
    // Part 1
    GLPbo::draw_fullwindow_quad();

    // Part 2: swap buffers: front <-> back
    glfwSwapBuffers(GLHelper::ptr_window);
}

/*  _________________________________________________________________________ */
/*! init
@param none
@return none

Get handle to OpenGL context through GLHelper::GLFWwindow*.
*/
static void init() {
    // Part 1
    if (!GLHelper::init(1800, 1800, "Tutorial 6")) {
        std::cout << "Unable to create OpenGL context" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Part 2
    GLPbo::init(GLHelper::width, GLHelper::height);
}

/*  _________________________________________________________________________ */
/*! cleanup
@param none
@return none

Return allocated resources for window and OpenGL context thro GLFW back
to system.
Return graphics memory claimed through
*/
void cleanup() {
    // Part 1
    GLPbo::cleanup();

    // Part 2
    GLHelper::cleanup();
}
