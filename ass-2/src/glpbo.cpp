/*!
@file		glpbo.cpp
@author		pghali@digipen.edu

@co-author	benjaminzhiyuan.lee@digipen.edu
@date		19/06/2023

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.

*//*__________________________________________________________________________*/
#include <glpbo.h>
#define UNREFERENCED_PARAMETER(P) (P)  
#define int_only static_cast<int>

namespace CORE10
{
    std::vector<std::string> objectName{ "ogre","cube" };

    glm::vec3 cam_pos = glm::vec3(0.f, 0.f, 10.f);
    glm::vec3 target = glm::vec3(0.f, 0.f, 0.f);

    float near_plane = 8.f;
    float far_plane = 12.f;
    float top_plane = 1.5f;
    float bottom_plane = -1.5f;
    float left_plane;
    float right_plane;

    glm::dvec3 intensity{1.0,1.0,1.0}; // we choose to not store the alpha component
    glm::dvec3 light_pos{ 0.0,0.0,10.0 };
   
}

// Definitions for all static data members.
GLsizei GLPbo::pixel_cnt;
GLsizei GLPbo::byte_cnt;
GLsizei GLPbo::width;
GLsizei GLPbo::height;
GLPbo::Color* GLPbo::ptr_to_pbo{ nullptr };
GLuint GLPbo::vaoid;
GLuint GLPbo::elem_cnt;
GLuint GLPbo::pboid;
GLuint GLPbo::texid;
GLSLShader GLPbo::shdr_pgm;
GLPbo::Color GLPbo::clear_clr;
glm::mat4 view_chain;
double* depthBuffer;
GLPbo::Model ModelTrans{};
GLPbo::Model GLPbo::mdl;
std::unordered_map<std::string, GLPbo::Model> mdl_map;
std::unordered_map<std::string, GLPbo::Model>::iterator current_mdl_iterator;

bool previous_keystateM = false;
bool rotating = false;
float  angle{};
float radians{};
bool  cull = true;
std::string mode{};

int cull_counter{};
int vtx_counter{};
int tri_counter{};
float timesSpeed = 1.0f;

float normalizeDegrees(float degrees);

/*!***********************************************************************
\brief Emulates the graphics pipeline by generating images with changing colors.

\param None

\details This function emulates the graphics pipeline by generating images with changing colors. It uses the current time to calculate the color values based on sine and cosine functions.
The resulting color values are mapped to the range [0, 255] and assigned to the clear_clr variable. The set_clear_color function is called to update the clear color of the PBO.
The PBO is then mapped to the application side using glMapNamedBuffer, and the clear_color_buffer function is called to fill the PBO with the new color.
Finally, the PBO is unmapepd and the texture image is updated using glTextureSubImage2D.

\note This function assumes that the necessary variables and objects (timesSpeed, clear_clr, pboid, ptr_to_pbo, texid, width, and height) have been properly initialized.
*************************************************************************/
void GLPbo::emulate() {
    if (GLHelper::keystateM)
    {
        // The 'M' key has just been released.
        ++current_mdl_iterator;
        if (current_mdl_iterator == mdl_map.end())
        {
            current_mdl_iterator = mdl_map.begin();
        }
        current_mdl_iterator->second.rotating = false;
        cull = true;
        cull_counter = 0;
        GLHelper::keystateM = GL_FALSE;
    }

    GLPbo::Model& current_mdl = current_mdl_iterator->second;

    if (GLHelper::keystateW)
    {
        current_mdl.Tasking = static_cast<GLPbo::Model::task>(static_cast<int>(current_mdl.Tasking) + 1);
        if (static_cast<int>(current_mdl.Tasking) == 4)
        {
            current_mdl.Tasking = GLPbo::Model::task::wireframe;
        }
        GLHelper::keystateW = GL_FALSE;
    }

    if (GLHelper::keystateR)
    {
        current_mdl_iterator->second.rotating = !current_mdl_iterator->second.rotating;
        GLHelper::keystateR = GL_FALSE;
    }

    if (current_mdl_iterator->second.rotating)
    {
        //current_mdl_iterator->second.angle += 1;
        current_mdl_iterator->second.angle = normalizeDegrees(current_mdl_iterator->second.angle + 1);
    }

    //set_clear_color(clear_clr);
    // Mapping pboid to client address ptr_to_pbo
    ptr_to_pbo = static_cast<GLPbo::Color*>(glMapNamedBuffer(pboid, GL_WRITE_ONLY));
    clear_color_buffer();
    clear_depth_buffer();
    viewport_xform(current_mdl);

    for (size_t i = 0; i < current_mdl.tri.size(); i += 3)
    {
        // Get the indices of the three vertices that form the triangle.
        int idx1 = current_mdl.tri[i];
        int idx2 = current_mdl.tri[i + 1];
        int idx3 = current_mdl.tri[i + 2];

        // Calculate two edges of the triangle.
        glm::vec3 edge1 = current_mdl.pd[idx2] - current_mdl.pd[idx1];
        glm::vec3 edge2 = current_mdl.pd[idx3] - current_mdl.pd[idx1];

        // Calculate the normal of the triangle.
        glm::vec3 normal = glm::cross(edge1, edge2);
        // Check if the triangle is back-facing.
        if (normal.z >= 0) {
            switch (current_mdl.Tasking)
            {
            case GLPbo::Model::task::wireframe:
                render_linebresenham(int_only(current_mdl.pd[idx1].x), int_only(current_mdl.pd[idx1].y), int_only(current_mdl.pd[idx2].x), int_only(current_mdl.pd[idx2].y), { 0, 0, 255 ,255 });
                render_linebresenham(int_only(current_mdl.pd[idx2].x), int_only(current_mdl.pd[idx2].y), int_only(current_mdl.pd[idx3].x), int_only(current_mdl.pd[idx3].y), { 0, 0, 255 ,255 });
                render_linebresenham(int_only(current_mdl.pd[idx3].x), int_only(current_mdl.pd[idx3].y), int_only(current_mdl.pd[idx1].x), int_only(current_mdl.pd[idx1].y), { 0, 0, 255 ,255 });
                mode = "Wireframe";
                break;
            case GLPbo::Model::task::depth:
                render_shadow_map(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3]);
                mode = "Depth Buffer";
                break;
            case GLPbo::Model::task::faceted:
                render_faceted_shading(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3], current_mdl.pm[idx1], current_mdl.pm[idx2], current_mdl.pm[idx3]);
                mode = "Faceted";
                break;
            case GLPbo::Model::task::shaded:
                render_smooth_shading(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3], 
                    current_mdl.pm[idx1], current_mdl.pm[idx2], current_mdl.pm[idx3], 
                    current_mdl.nml[idx1], current_mdl.nml[idx2], current_mdl.nml[idx3]);
                mode = "Shaded";
                break;
            }
        }
        else if (cull)
        {
            cull_counter++;
        }
    }
    cull = false;
    // BIND A NAMED BUFFER OBJECT
    // GL_PIXEL_UNPACK_BUFFER - "target" - purpose is for Texture data source
    // pboid - "buffer" - name of the sourced buffer object
    glUnmapNamedBuffer(pboid);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboid);
    // Associate the PBO with the texture image - texid read (unpack) their data from the buffer object into texid image store 
    glTextureSubImage2D(texid, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Unbind the PBO
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

/*!***********************************************************************
\brief Draws a full-window quad with the current PBO texture.

\param None

\details This function binds the PBO texture to texture unit 0 using glBindTextureUnit. It then uses the shader program specified by shdr_pgm by calling shdr_pgm.Use().
The texture location is obtained using glGetUniformLocation and stored in texture_location. The texture unit is set using glUniform1i with the texture location and value 0.
The vertex array object (VAO) specified by vaoid is bound using glBindVertexArray. The quad is drawn using glDrawArrays with the parameters GL_TRIANGLE_STRIP, 0, and elem_cnt.
Finally, the shader program is un-used with shdr_pgm.UnUse().

\note This function assumes that the necessary variables and objects (texid, shdr_pgm, vaoid, and elem_cnt) have been properly initialized.

*************************************************************************/
void GLPbo::draw_fullwindow_quad() {
    std::stringstream sstr;
    // Bind texture unit
    glBindTextureUnit(0, texid);

    // Use program
    shdr_pgm.Use();

    // Get texture location for fragment shader use
    GLuint texture_location = glGetUniformLocation(shdr_pgm.GetHandle(), "uTex2d");
    glUniform1i(texture_location, 0);

    // Bind the VAO
    glBindVertexArray(vaoid);

    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, elem_cnt);
    shdr_pgm.UnUse();
    std::string modelName = current_mdl_iterator->first;

    sstr << std::fixed << std::setprecision(2) << "A1 | Benjamin Lee | Model: " << modelName << " | Mode: " << mode << " | Vtx: " << current_mdl_iterator->second.pm.size() << " | Tri: " << current_mdl_iterator->second.tri.size() / 3 << " | Culled: " << cull_counter << " | FPS: " << GLHelper::fps;
    glfwSetWindowTitle(GLHelper::ptr_window, sstr.str().c_str());
}

/*!***********************************************************************
\brief Initializes the GLPbo object with the specified width and height.

\param w The width of the GLPbo object.
\param h The height of the GLPbo object.

\details This function initializes the GLPbo object by setting the width and height variables to the provided values.
It calculates the total number of pixels (pixel_cnt) and bytes (byte_cnt) based on the width and height. The clear color is set to white (255, 255, 255).
The texture object is created using glCreateTextures, with the target set to GL_TEXTURE_2D, and the storage is allocated using glTextureStorage2D with the format GL_RGBA8, and the width and height of the GLPbo object.
The pixel buffer object (PBO) is created using glCreateBuffers, and the storage is allocated using glNamedBufferStorage.
The storage size is determined by the byte count, and the storage flags include GL_DYNAMIC_STORAGE_BIT and GL_MAP_WRITE_BIT.
The quad vertex array object (VAO) and the shader program are set up using the setup_quad_vao and setup_shdrpgm functions, respectively.

\note This function assumes that the necessary variables and objects (texid, pboid, setup_quad_vao, and setup_shdrpgm) have been properly defined and implemented.

*************************************************************************/
void GLPbo::init(GLsizei w, GLsizei h) {
    width = w;
    height = h;

    pixel_cnt = w * h;
    byte_cnt = pixel_cnt * 4;

    
    depthBuffer = new double[pixel_cnt];

    glm::mat4 view_port{
    GLHelper::width * 0.5, 0                   , 0, 0,
    0                  , GLHelper::height * 0.5, 0, 0,
    0                  , 0                   , 1, 0,
     GLHelper::width * 0.5, GLHelper::height * 0.5, 0, 1
    };

    glm::vec3 eye = CORE10::cam_pos;
    glm::vec3 target = CORE10::target;
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);

    glm::mat4 view{};
    view = glm::lookAt(eye, target, up);

    glm::mat4 ortho{};
    float ar = (float)GLPbo::width / (float)GLPbo::height;
    float aspect_ratio = (float)GLPbo::width / (float)GLPbo::height;
    CORE10::left_plane = aspect_ratio * CORE10::bottom_plane;
    CORE10::right_plane = aspect_ratio * CORE10::top_plane;
    ortho = glm::ortho(CORE10::left_plane, CORE10::right_plane, CORE10::bottom_plane, CORE10::top_plane, CORE10::near_plane, CORE10::far_plane);

    view_chain = view_port * ortho * view;

    set_clear_color(0,0,0);

    glCreateTextures(GL_TEXTURE_2D, 1, &texid);

    glTextureStorage2D(texid, 1, GL_RGBA8, width, height);

    glCreateBuffers(1, &pboid);

    glNamedBufferStorage(pboid,
        byte_cnt,
        nullptr,
        GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

    for (const auto& x : CORE10::objectName)
    {
        GLPbo::mdl.pm.clear();
        GLPbo::mdl.nml.clear();
        GLPbo::mdl.tex.clear();
        GLPbo::mdl.tri.clear();
        if (DPML::parse_obj_mesh("../meshes/" + x + ".obj", mdl.pm, mdl.nml, mdl.tex, mdl.tri, true, true, true))
        {
            mdl_map[x] = mdl;
        }
    }

    current_mdl_iterator = mdl_map.begin();

    viewport_xform(mdl);
    setup_quad_vao();
    setup_shdrpgm();
}

/*!***********************************************************************
\brief Sets up the vertex array object (VAO) for rendering a full-window quad.

\details This function creates a VBO to store the position and texture coordinate data of the quad. It uses the provided pos_vtx and tex_coord vectors to define the vertex attributes.
A VBO is created using glCreateBuffers, and the storage is allocated using glNamedBufferStorage. The size of the storage is calculated based on the size of the pos_vtx and tex_coord vectors.
The position data is uploaded to the VBO using glNamedBufferSubData with the offset set to 0 and the size based on the size of the pos_vtx vector.
The texture coordinate data is uploaded to the VBO using glNamedBufferSubData with the offset set to the size of the pos_vtx data and the size based on the size of the tex_coord vector.
A VAO is created using glCreateVertexArrays.
The position attribute is enabled using glEnableVertexArrayAttrib, and its vertex buffer is set using glVertexArrayVertexBuffer.
The attribute format is defined using glVertexArrayAttribFormat, and the attribute binding is set using glVertexArrayAttribBinding.
The texture coordinate attribute is enabled using glEnableVertexArrayAttrib, and its vertex buffer is set using glVertexArrayVertexBuffer.
The attribute format is defined using glVertexArrayAttribFormat, and the attribute binding is set using glVertexArrayAttribBinding.
The element count is set to the size of the pos_vtx vector.
The VBO is deleted using glDeleteBuffers, and the VAO is unbound using glBindVertexArray.

\note This function assumes that the necessary variables and objects (vaoid) have been properly defined and implemented.

*************************************************************************/
void GLPbo::setup_quad_vao() {
    std::vector<glm::vec2> pos_vtx{
        glm::vec2{-1.f, -1.f}, glm::vec2{-1.f,1.f},
        glm::vec2{1.f,-1.f}, glm::vec2{1.f,1.f}
    };
    std::vector<glm::vec2> tex_coord{
        glm::vec2{0.0f, 0.0f}, glm::vec2{0.0f, 1.f},
        glm::vec2{1.0f, 0.0f}, glm::vec2{1.0f, 1.0f}
    };

    // Creating VBO
    GLuint vbo_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glNamedBufferStorage(vbo_hdl, pos_vtx.size() * sizeof(glm::vec2) + tex_coord.size() * sizeof(glm::vec2), nullptr, GL_DYNAMIC_STORAGE_BIT);

    // VBO - Position
    glNamedBufferSubData(vbo_hdl, 0, pos_vtx.size() * sizeof(glm::vec2), pos_vtx.data());
    // VBO - Texture Coordinates
    glNamedBufferSubData(vbo_hdl, pos_vtx.size() * sizeof(glm::vec2), tex_coord.size() * sizeof(glm::vec2), tex_coord.data());

    // Creating VAO
    glCreateVertexArrays(1, &vaoid);

    // VAO - Position
    glEnableVertexArrayAttrib(vaoid, 0);
    glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(glm::vec2));
    glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaoid, 0, 0);
    // VAO - Texture Coordinates
    glEnableVertexArrayAttrib(vaoid, 1);
    glVertexArrayVertexBuffer(vaoid, 1, vbo_hdl, sizeof(glm::vec2) * pos_vtx.size(), sizeof(glm::vec2));
    glVertexArrayAttribFormat(vaoid, 1, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaoid, 1, 1);

    // Element Count
    elem_cnt = pos_vtx.size();
    glDeleteBuffers(1, &vbo_hdl);
    glBindVertexArray(0);
}

/*!***********************************************************************
\brief Sets up the shader program for rendering.

\details This function creates the vertex and fragment shaders using the provided shader source code.
The vertex shader takes in the vertex position and texture coordinate attributes and outputs the texture coordinate as a varying variable.
The fragment shader takes in the texture coordinate and outputs the final fragment color based on the texture lookup.
The vertex shader source code is defined as a string vert_Shader and contains the necessary shader code for the vertex stage.
The fragment shader source code is defined as a string frag_Shader and contains the necessary shader code for the fragment stage.
The shader source code strings are stored in a vector of pairs, shdr_sources, where each pair contains the shader type (e.g., GL_VERTEX_SHADER) and the corresponding source code.
The shader program object (shdr_pgm) is iterated over the shdr_sources vector, and each shader source is compiled using shdr_pgm.CompileShaderFromString. If compilation fails, the shader program log is printed.
After compiling all shaders, the shader program is linked using shdr_pgm.Link() and validated using shdr_pgm.Validate(). If either linking or validation fails, an error message is printed.

\note This function assumes that the necessary variables and objects (shdr_pgm) have been properly defined and implemented.

*************************************************************************/
void GLPbo::setup_shdrpgm() {
    const std::string vert_Shader{
      "#version 450 core\n"
      "layout (location = 0) in vec2 aVertexPosition;\n"
      "layout (location = 1) in vec2 aTextureCoord;\n"
      "layout (location = 1) out vec2 vTextureCoord;\n"
      "void main(void){\n"
      "gl_Position = vec4(aVertexPosition, 0.0, 1.0);\n"
      "vTextureCoord = aTextureCoord;\n"
      "}"
    };

    const std::string frag_Shader{
        "#version 450 core\n"
        "layout (location = 1) in vec2 vTextureCoord;\n"
        "layout (location = 0) out vec4 fFragColor;\n"
        "uniform sampler2D uTex2d;\n"
        "void main() {\n"
        //"vec4 textureColor = texture(uTex2d, vTextureCoord);\n"
        "fFragColor = texture(uTex2d, vTextureCoord);\n"
        "}"
    };

    shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, vert_Shader);
    shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, frag_Shader);

    if (!shdr_pgm.Link() || !shdr_pgm.Validate()) {
        std::cout << "Shaders is not linked and/or not validated!" << std::endl;
        std::cout << shdr_pgm.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    //shdr_pgm.PrintActiveAttribs();
    //shdr_pgm.PrintActiveUniforms();
}

/*!***********************************************************************
\brief Cleans up the allocated resources.

\details This function deletes the vertex array object (vaoid), pixel buffer object (pboid), and texture object (texid) using the corresponding OpenGL delete functions: glDeleteVertexArrays, glDeleteBuffers, and glDeleteTextures, respectively.

\note This function assumes that the necessary variables (vaoid, pboid, texid) have been properly defined and implemented.

*************************************************************************/
void GLPbo::cleanup() {
    glInvalidateBufferData(texid);
    glDeleteTextures(1, &texid);

    glInvalidateBufferData(pboid);
    glDeleteBuffers(1, &pboid);

    glInvalidateBufferData(vaoid);
    glDeleteBuffers(1, &vaoid);
}

/*!***********************************************************************
\brief Sets the clear color of the GLPbo.

\param clr The color to set as the clear color.

\details This function sets the clear color of the GLPbo by assigning the provided clr to the clear_clr member variable.

\note This function assumes that the clear_clr member variable has been properly defined and implemented.

*************************************************************************/
void GLPbo::set_clear_color(GLPbo::Color clr) {
    clear_clr = clr;
}

/*!***********************************************************************
\brief Sets the clear color of the GLPbo.

\param r The red component of the clear color.
\param g The green component of the clear color.
\param b The blue component of the clear color.
\param a The alpha component of the clear color.

\details This function sets the clear color of the GLPbo by assigning the provided r, g, b, and a values to the corresponding components of the clear_clr member variable.

\note This function assumes that the clear_clr member variable has been properly defined and implemented.

*************************************************************************/
void GLPbo::set_clear_color(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
    clear_clr.rgba.r = r;
    clear_clr.rgba.g = g;
    clear_clr.rgba.b = b;
    clear_clr.rgba.a = a;
}

/*!***********************************************************************
\brief Clears the color buffer of the GLPbo.

\details This function fills the color buffer of the GLPbo with the clear color (clear_clr) using the std::fill algorithm.

\note This function assumes that ptr_to_pbo and pixel_cnt have been properly defined and implemented, and that clear_clr represents the desired clear color.

*************************************************************************/
void GLPbo::clear_color_buffer() {
    // FPS: 510
    std::fill(ptr_to_pbo, ptr_to_pbo + pixel_cnt, clear_clr);
}

void GLPbo::clear_depth_buffer()
{
    std::fill(depthBuffer, depthBuffer + pixel_cnt, 1.0);
}

/**
@brief Applies viewport transformation to the given model.
This function applies a viewport transformation to the specified model.
It clears the model's point data and then performs rotation and viewport transformations on each point.
The rotated points are converted to a vec4 and multiplied by the viewport matrix to obtain the transformed points.
The resulting transformed points are stored in the model's point data vector.

@param mdl The model to apply the viewport transformation to.
*/
void GLPbo::viewport_xform(Model& model) {

    model.pd.clear();
    radians = glm::radians(model.angle);
    glm::mat3 m_rotation{ cos(radians),  0, -sin(radians),
                          0,  1,  0,
                          sin(radians), 0, cos(radians)};

    double scale_val; 

    if (current_mdl_iterator->first == "cube")
    {
        scale_val = 1.5;
    }
    else
    {
        scale_val = 2;
    }
    
    glm::mat3 scale
    {
            scale_val,  0,          0,
            0,          scale_val,  0,
            0,          0,          scale_val
    };
    model.ModelTrans = m_rotation;
    glm::mat3 model_trans = scale * m_rotation;

    for (size_t i = 0; i < model.pm.size(); i++)
    {   
        //Apply the scale and rotation
        glm::vec3 scale_rot = model_trans * model.pm[i];

        // Convert the rotated vec3 to a vec4
        glm::vec4 rotated4(scale_rot, 1.0f);

        // Apply the viewport transformation
        glm::vec4 transformed = view_chain * rotated4;
       // model.pm[i].z = (model.pm[i].z + 1) / 2;
        model.pd.push_back(glm::vec3(transformed));
    }
}

/**
@brief Sets the color of a pixel at the specified coordinates.
This function sets the color of a pixel at the given (x, y) coordinates.
It uses the glScissor function to define the scissor box to cover the entire viewport.
The position of the pixel in the PBO (Pixel Buffer Object) is calculated based on the width and the provided (x, y) coordinates.
The color value is then assigned to the corresponding position in the PBO.

@param x The x-coordinate of the pixel.
@param y The y-coordinate of the pixel.
@param clr The color to set for the pixel.
*/
void GLPbo::set_pixel(int x, int y, Color clr)
{
   glScissor(0, 0, GLHelper::width, GLHelper::height);
    int position = (GLPbo::width * y) + x;
    ptr_to_pbo[position] = clr;
}

/**
@brief Renders a line using the Bresenham's line algorithm.
This function renders a line using the Bresenham's line algorithm between the specified (x0, y0) and (x1, y1) coordinates.
The draw_clr parameter defines the color to be used for drawing the line.

@param x0 The x-coordinate of the starting point of the line.
@param y0 The y-coordinate of the starting point of the line.
@param x1 The x-coordinate of the ending point of the line.
@param y1 The y-coordinate of the ending point of the line.
@param draw_clr The color to be used for drawing the line.
*/
void GLPbo::render_linebresenham(GLint x0, GLint y0,
    GLint x1, GLint y1, GLPbo::Color draw_clr) {

    int dx = x1 - x0, dy = y1 - y0, xNext{}, yNext{};
    // Calculating for x coordinate
    if (dx < 0) {
        dx = -dx;
        xNext = -1;
    }
    else {
        dx = dx;
        xNext = 1;
    }
    // Calculating for y coordinate
    if (dy < 0) {
        dy = -dy;
        yNext = -1;
    }
    else {
        dy = dy;
        yNext = 1;
    }

    if (abs(dy) >= abs(dx)) {
        int d = 2 * dx - dy, dSame = 2 * dx, dNext = 2 * dx - 2 * dy;
        int x = x0, y = y0;

        //std::cout << "Drawing point at (" << x1 << "," << y1 << "), decision parameter = " << d << "\n";
        set_pixel(x, y, draw_clr);
        while (--dy > 0) {
            x += (d > 0) ? xNext : 0;
            d += (d > 0) ? dNext : dSame;
            y += yNext;
            //std::cout << "Drawing point at (" << x1 << "," << y1 << "), decision parameter = " << d << "\n";
            set_pixel(x, y, draw_clr);
        }
    }
    else {
        int d = 2 * dy - dx, dSame = 2 * dy, dNext = 2 * dy - 2 * dx;
        int x = x0, y = y0;

        //std::cout << "Drawing point at (" << x1 << "," << y1 << "), decision parameter = " << d << "\n";
        set_pixel(x, y, draw_clr);

        while (--dx > 0) {
            y += (d > 0) ? yNext : 0;
            d += (d > 0) ? dNext : dSame;
            x += xNext;
            //std::cout << "Drawing point at (" << x1 << "," << y1 << "), decision parameter = " << d << "\n";
            set_pixel(x, y, draw_clr);
        }
    }
}

/**
@brief Normalizes an angle in degrees to the range [0, 360).
This function normalizes an angle in degrees to the range [0, 360) by applying modulo operation.
If the input angle is positive, it is modulo divided by 360.
If the input angle is negative, its absolute value is modulo divided by -360.
The resulting normalized angle is returned.

@param degrees The angle in degrees to be normalized.
@return The normalized angle in the range [0, 360).
*/
float normalizeDegrees(float degrees) {
    if (degrees >= 0) {
        degrees = static_cast<float>(fmod(degrees, 360));
    }
    else {
        degrees = static_cast<float>(fmod(degrees, -360));
    }
    return degrees;
}

struct EdgeEqn {
    double a{};
    double b{};
    double c{};

    bool topLeft{};
};

/**
@brief Computes the edge equation for a line segment defined by two points.
This function computes the edge equation for a line segment defined by two points, p0 and p1.
The edge equation is represented by the coefficients (a, b, c) in the equation ax + by + c = 0.
The function assigns the computed coefficients to the provided EdgeEqn structure, E.

@param p0 The first point of the line segment.
@param p1 The second point of the line segment.
@param E The EdgeEqn structure to store the computed coefficients.
*/
void computeEdgeEqn(glm::dvec3 const& p0, glm::dvec3 const& p1, EdgeEqn& E) {

    E.a = p0.y - p1.y;
    E.b = p1.x - p0.x;
    E.c = p0.x * p1.y - p1.x * p0.y;

    // Check for Left edge         // Check for Top Edge
    E.topLeft = (E.a != 0.0) ? (E.a > 0.0 ? true : false) : (E.b < 0.0 ? true : false);

}

/**
@brief Checks if a point is in the interior of an edge, considering the TopLeft tie breaker.
This function determines if a point is in the interior of an edge, taking into account the TopLeft tie breaker.
The function takes an evaluation value (eval), which indicates the result of evaluating the edge equation with the point coordinates.
If the eval value is greater than 0, it means the point is on the interior of the edge.
If the eval value is equal to 0 and the topLeft flag is true, indicating the edge passes the TopLeft tie breaker,
the point is also considered in the interior.
The function returns true if the point is in the interior, and false otherwise.

@param eval The evaluation value of the edge equation with the point coordinates.
@param p The coordinates of the point to be checked.
@param topLeft The flag indicating if the edge passes the TopLeft tie breaker.
@return True if the point is in the interior of the edge, false otherwise.
*/
bool PointInEdgeTopLeftOptimized(double const& eval, glm::dvec2 p, bool& topLeft) noexcept{
    UNREFERENCED_PARAMETER(p);
    // if eval is more than 0 means its on the interior of the line.
    // also if, eval == 0 (meaning on the edge line itself) and if the line pass the TopLeft tie breaker, -
    // it is considered in the interior as well and thus should be considered in the interior
    return (eval > 0 || (eval == 0 && topLeft == true)) ? true : false;
}

/**
@brief Checks if a point is inside a triangle, considering the TopLeft tie breaker for each edge.
This function determines if a point is inside a triangle, taking into account the TopLeft tie breaker for each edge.
The function takes three evaluation values (eval0, eval1, eval2), which indicate the results of evaluating the edge equations
of the triangle's edges with the point coordinates.
For each edge, the function calls the PointInEdgeTopLeftOptimized function, which considers the TopLeft tie breaker.
If the point is inside all three edges (passes the TopLeft tie breaker for each edge), the function returns true.
Otherwise, it returns false.

@param eval0 The evaluation value of the first edge equation with the point coordinates.
@param eval1 The evaluation value of the second edge equation with the point coordinates.
@param eval2 The evaluation value of the third edge equation with the point coordinates.
@param p The coordinates of the point to be checked.
@param topLeft0 The flag indicating if the first edge passes the TopLeft tie breaker.
@param topLeft1 The flag indicating if the second edge passes the TopLeft tie breaker.
@param topLeft2 The flag indicating if the third edge passes the TopLeft tie breaker.
@return True if the point is inside the triangle, false otherwise.
*/
bool PointInTriangleOptimized(double const& eval0, double const& eval1, double const& eval2, glm::dvec2 p, bool& topLeft0, bool& topLeft1, bool& topLeft2) {
    if (PointInEdgeTopLeftOptimized(eval0, p, topLeft0) && PointInEdgeTopLeftOptimized(eval1, p, topLeft1) && PointInEdgeTopLeftOptimized(eval2, p, topLeft2)) {
        return true;
    }
    else {
        return false;
    }
}

/**
@brief Calculates the evaluation value of an edge equation with the TopLeft tie breaker.
This function calculates the evaluation value of an edge equation with the TopLeft tie breaker
for a given random point. The edge equation is represented by the coefficients (a, b, c) in the equation ax + by + c = 0,
stored in the provided EdgeEqn structure, E.
The function computes the evaluation value by substituting the coordinates of the random point into the edge equation.

@param E The EdgeEqn structure containing the coefficients of the edge equation.
@param random_point The coordinates of the random point.
@return The evaluation value of the edge equation with the TopLeft tie breaker.
*/
double calculateEdgeEqn_TopLeft(EdgeEqn& E, glm::dvec3 random_point) {
    // this is eval of the EdgeEqn
    return (E.a * random_point.x + E.b * random_point.y + E.c);
}

/**
@brief Renders a filled triangle using the scanline algorithm.
This function renders a filled triangle using the scanline algorithm.
It takes three vertices of the triangle (p0, p1, p2) and a color (clr) as input.
The function computes the edge equations of the triangle's three sides using the computeEdgeEqn function.
It then determines the bounding box of the triangle to define the scanline range.
For each scanline within the bounding box, the function calculates the evaluation values of the edge equations
with the TopLeft tie breaker at the midpoint of each pixel on the scanline.
The PointInTriangleOptimized function is used to check if each pixel falls within the triangle.
If a pixel is inside the triangle, the set_pixel function is called to set the corresponding pixel color.
The function returns true upon successful rendering.

@param p0 The first vertex of the triangle.
@param p1 The second vertex of the triangle.
@param p2 The third vertex of the triangle.
@param clr The color of the triangle.
@return True if the triangle was rendered successfully, false otherwise.
*/
bool GLPbo::render_faceted_shading(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2, glm::dvec3 const& m0, glm::dvec3 const& m1, glm::dvec3 const& m2) {
    EdgeEqn e0, e1, e2;

    // Edge equation of the 3 lines
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area_full_triangle = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));

    // Bounding box of the triangle
    GLdouble minX = floor(std::min({ p0.x, p1.x, p2.x }));
    GLdouble maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    GLdouble minY = floor(std::min({ p0.y, p1.y, p2.y }));
    GLdouble maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eVal0 = calculateEdgeEqn_TopLeft(e0, { minX + 0.5f, minY + 0.5f, 0 });
    double eVal1 = calculateEdgeEqn_TopLeft(e1, { minX + 0.5f, minY + 0.5f, 0 });
    double eVal2 = calculateEdgeEqn_TopLeft(e2, { minX + 0.5f, minY + 0.5f, 0 });

    double barycentric_a = eVal0 / area_full_triangle;
    double barycentric_b = eVal1 / area_full_triangle;
    double barycentric_c = eVal2 / area_full_triangle;

    double incrementX_a = e0.a / area_full_triangle;
    double incrementX_b = e1.a / area_full_triangle;
    double incrementX_c = e2.a / area_full_triangle;

    double incrementY_a = e0.b / area_full_triangle;
    double incrementY_b = e1.b / area_full_triangle;
    double incrementY_c = e2.b / area_full_triangle;

    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;

    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);

    glm::mat3 inverse_transform = glm::inverse(current_mdl_iterator->second.ModelTrans);

    double Cx = (m0.x + m1.x + m2.x) * 0.333;
    double Cy = (m0.y + m1.y + m2.y) * 0.333;
    double Cz = (m0.z + m1.z + m2.z) * 0.333;

    glm::dvec3 centroid = glm::dvec3(Cx, Cy, Cz);

    glm::dvec3 AB = m1 - m0;
    glm::dvec3 AC = m2 - m0;
    glm::dvec3 outwardNormal = glm::normalize(glm::cross(AB, AC));

    glm::dvec3 lightpos = CORE10::light_pos;
    lightpos = inverse_transform * lightpos;
    glm::dvec3 vectorToLight = lightpos - centroid;
    glm::dvec3 normalisedvectorToLight = glm::normalize(vectorToLight);
  
    double dotProduct = glm::dot(outwardNormal, normalisedvectorToLight);
   
    double reflectance{1.0};
    double incomingLight = glm::max(0.0, dotProduct) * reflectance;
    
    GLubyte clr = static_cast<GLubyte>(incomingLight*255);
    
    for (int y = intMinY; y < intMaxY; ++y)
    {
        double HEVal0 = eVal0;
        double HEVal1 = eVal1;
        double HEVal2 = eVal2;

        double HEa = barycentric_a;
        double HEb = barycentric_b;
        double HEc = barycentric_c;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangleOptimized(HEVal0, HEVal1, HEVal2, { x + 0.5, y + 0.5 }, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                z = (z + 1) / 2;
                int buffer_idx = y * GLPbo::width + x;
                if (z < depthBuffer[buffer_idx])
                {
                    depthBuffer[buffer_idx] = z;
                    set_pixel(x, y, { clr,clr,clr });
                }
                
            }
            // plus 'a' is increment it horizontally aka increment x-axis
            HEVal0 += e0a;
            HEVal1 += e1a;
            HEVal2 += e2a;

            HEa += incrementX_a;
            HEb += incrementX_b;
            HEc += incrementX_c;
        }
        // plus 'b' is increment it vertically aka increment y-axis
        eVal0 += e0b;
        eVal1 += e1b;
        eVal2 += e2b;

        barycentric_a += incrementY_a;
        barycentric_b += incrementY_b;
        barycentric_c += incrementY_c;
    }

    return true;
}

/**
@brief Renders a filled triangle using the scanline algorithm with interpolated colors.
This function renders a filled triangle using the scanline algorithm with interpolated colors.
It takes three vertices of the triangle (p0, p1, p2) and the corresponding vertex colors (c0, c1, c2) as input.
The function computes the edge equations of the triangle's three sides using the computeEdgeEqn function.
It then determines the bounding box of the triangle to define the scanline range.
For each scanline within the bounding box, the function calculates the evaluation values of the edge equations
with the TopLeft tie breaker at the midpoint of each pixel on the scanline.
The PointInTriangleOptimized function is used to check if each pixel falls within the triangle.
If a pixel is inside the triangle, the color is interpolated using the barycentric coordinates of the pixel
and the vertex colors. The set_pixel function is called to set the corresponding pixel color.
The function returns true upon successful rendering.

@param p0 The first vertex of the triangle.
@param p1 The second vertex of the triangle.
@param p2 The third vertex of the triangle.
@param c0 The color at the first vertex.
@param c1 The color at the second vertex.
@param c2 The color at the third vertex.
@return True if the triangle was rendered successfully, false otherwise.
*/
bool GLPbo::render_shadow_map(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2) {

    EdgeEqn e0, e1, e2;

    // E1dge equation of the 3 lines0
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area_full_triangle = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));

    // Bounding box of the triangle
    GLdouble minX = floor(std::min({ p0.x, p1.x, p2.x }));
    GLdouble maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    GLdouble minY = floor(std::min({ p0.y, p1.y, p2.y }));
    GLdouble maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eVal0 = calculateEdgeEqn_TopLeft(e0, { minX + 0.5f, minY + 0.5f, 0 });
    double eVal1 = calculateEdgeEqn_TopLeft(e1, { minX + 0.5f, minY + 0.5f, 0 });
    double eVal2 = calculateEdgeEqn_TopLeft(e2, { minX + 0.5f, minY + 0.5f, 0 });

    double barycentric_a = eVal0 / area_full_triangle;
    double barycentric_b = eVal1 / area_full_triangle;
    double barycentric_c = eVal2 / area_full_triangle;

    double incrementX_a = e0.a / area_full_triangle;
    double incrementX_b = e1.a / area_full_triangle;
    double incrementX_c = e2.a / area_full_triangle;

    double incrementY_a = e0.b / area_full_triangle;
    double incrementY_b = e1.b / area_full_triangle;
    double incrementY_c = e2.b / area_full_triangle;


    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;

    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);


    for (int y = intMinY; y < intMaxY; ++y) {
        double HEVal0 = eVal0;
        double HEVal1 = eVal1;
        double HEVal2 = eVal2;

        double HEa = barycentric_a;
        double HEb = barycentric_b;
        double HEc = barycentric_c;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangleOptimized(HEVal0, HEVal1, HEVal2, { x + 0.5, y + 0.5 }, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                z = (z + 1) / 2;
                int buffer_idx = y * GLPbo::width + x;
                if (z < depthBuffer[buffer_idx])
                {
                    depthBuffer[buffer_idx] = z;
                    set_pixel(x, y, { static_cast<GLubyte>(z * 255.0),static_cast<GLubyte>(z * 255.0),static_cast<GLubyte>(z * 255.0) });
                }
            }
            HEVal0 += e0a;
            HEVal1 += e1a;
            HEVal2 += e2a;

            HEa += incrementX_a;
            HEb += incrementX_b;
            HEc += incrementX_c;
        }
        eVal0 += e0b;
        eVal1 += e1b;
        eVal2 += e2b;

        barycentric_a += incrementY_a;
        barycentric_b += incrementY_b;
        barycentric_c += incrementY_c;

    }
    return true;
}

bool GLPbo::render_smooth_shading(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2, glm::vec3 const& m0, glm::vec3 const& m1, glm::vec3 const& m2, 
    glm::dvec3 const& n0, glm::dvec3 const& n1, glm::dvec3 const& n2) {
 
    //pm0 to lightsource pos
    glm::dvec3 lightsource = CORE10::light_pos;
    glm::dvec3 pm0 = glm::inverse(current_mdl_iterator->second.ModelTrans) * lightsource - m0;
    glm::dvec3 pm1 = glm::inverse(current_mdl_iterator->second.ModelTrans) * lightsource - m1;
    glm::dvec3 pm2 = glm::inverse(current_mdl_iterator->second.ModelTrans) * lightsource - m2;
    pm0 = glm::normalize(pm0);
    pm1 = glm::normalize(pm1);
    pm2 = glm::normalize(pm2);
    glm::dvec3 intensity = CORE10::intensity;
    glm::dvec3 c0 = intensity * glm::max(0.0, glm::dot(n0, pm0));
    glm::dvec3 c1 = intensity * glm::max(0.0, glm::dot(n1, pm1));
    glm::dvec3 c2 = intensity * glm::max(0.0, glm::dot(n2, pm2));

    EdgeEqn e0, e1, e2;

    // E1dge equation of the 3 lines0
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area_full_triangle = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));

    // Bounding box of the triangle
    GLdouble minX = floor(std::min({ p0.x, p1.x, p2.x }));
    GLdouble maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    GLdouble minY = floor(std::min({ p0.y, p1.y, p2.y }));
    GLdouble maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eVal0 = calculateEdgeEqn_TopLeft(e0, { minX + 0.5f, minY + 0.5f, 0 });
    double eVal1 = calculateEdgeEqn_TopLeft(e1, { minX + 0.5f, minY + 0.5f, 0 });
    double eVal2 = calculateEdgeEqn_TopLeft(e2, { minX + 0.5f, minY + 0.5f, 0 });

    double barycentric_a = eVal0 / area_full_triangle;
    double barycentric_b = eVal1 / area_full_triangle;
    double barycentric_c = eVal2 / area_full_triangle;

    double incrementX_a = e0.a / area_full_triangle;
    double incrementX_b = e1.a / area_full_triangle;
    double incrementX_c = e2.a / area_full_triangle;

    double incrementY_a = e0.b / area_full_triangle;
    double incrementY_b = e1.b / area_full_triangle;
    double incrementY_c = e2.b / area_full_triangle;


    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;

    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);


    for (int y = intMinY; y < intMaxY; ++y) {
        double HEVal0 = eVal0;
        double HEVal1 = eVal1;
        double HEVal2 = eVal2;

        double HEa = barycentric_a;
        double HEb = barycentric_b;
        double HEc = barycentric_c;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangleOptimized(HEVal0, HEVal1, HEVal2, { x + 0.5, y + 0.5 }, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                z = (z + 1) / 2;
                int buffer_idx = y * GLPbo::width + x;
               
                if (z < depthBuffer[buffer_idx])
                {
                glm::dvec3 clr = HEa * c0 + HEb * c1 + HEc * c2;
                depthBuffer[buffer_idx] = z;
                set_pixel(x, y, { static_cast<GLubyte>(clr.x*255),static_cast<GLubyte>(clr.y*255),static_cast<GLubyte>(clr.z*255) });
                }
            }
            HEVal0 += e0a;
            HEVal1 += e1a;
            HEVal2 += e2a;

            HEa += incrementX_a;
            HEb += incrementX_b;
            HEc += incrementX_c;
        }
        eVal0 += e0b;
        eVal1 += e1b;
        eVal2 += e2b;

        barycentric_a += incrementY_a;
        barycentric_b += incrementY_b;
        barycentric_c += incrementY_c;

    }
    return true;
}


