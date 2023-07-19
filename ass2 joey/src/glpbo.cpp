/*!
@file    glpbo.cpp
@author  pghali@digipen.edu
@date    10/11/2016
@co-author joeyjunyu.c@digipen.edu
@date    30/06/2023
@ammendmants added additional features
void GLPbo::emulate()
void GLPbo::draw_fullwindow_quad()
void GLPbo::init(GLsizei w, GLsizei h)
void GLPbo::setup_quad_vao()
void GLPbo::setup_shdrpgm()

This file provides the implementation of the GLPbo struct, provides functionalities for managing and rendering an OpenGL application.
It includes functions for managing texture objects, application states, vertex array objects, shaders, and animations.


*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <glpbo.h>
/*                                                   objects with file scope
----------------------------------------------------------------------------- */
namespace CORE10
{
    std::string textureFile{ "ogre.tex" };
    std::array<std::string, 2> fileName {"cube", "ogre"};
    GLPbo::PointLight lightSource{glm::vec3( 1,1,1 ), glm::vec3(0,0,10) };
    glm::vec3 eye{ 0,0,10 };
    glm::vec3 target{ 0,0,0 };
    GLuint textureWidth{}, textureHeight{}, bytes_per_texel{};

    //near plane : 8, far plane : 12, top plane : 1.5, bottom plane : -1.5
//    left plane : aspect - ratio * bottom plane
//    right plane : aspect - ratio * top plane
    double aspectRatio;
    double nearPlane = 8, 
        farPlane = 12, 
        topPlane = 1.5, 
        bottomPlane = -1.5;
    double rightPlane = aspectRatio * topPlane,
        leftPlane = aspectRatio * bottomPlane;
}
// Definitions for all static data members.
GLsizei GLPbo::width;
GLsizei GLPbo::height;
GLsizei GLPbo::pixel_cnt;
GLsizei GLPbo::byte_cnt;
GLPbo::Color* GLPbo::ptr_to_pbo{ nullptr };
GLuint GLPbo::vaoid;
GLuint GLPbo::elem_cnt;
GLuint GLPbo::pboid;
GLuint GLPbo::texid;
GLSLShader GLPbo::shdr_pgm;
GLPbo::Color GLPbo::clear_clr;
std::string selectModel{};
std::vector<std::string> objectName;
GLPbo::Model GLPbo::mdl;
std::map<std::string, GLPbo::Model> models;
std::map<std::string, GLPbo::Model>::iterator currModel;
bool previous_keystateM = false;
bool isRotating = false;
float  angle{};
float radians{};
bool  cullFlag = true;
std::string mode{};
GLubyte red{}, green{}, blue{};
int cullCount{};
int currentIndex{ 0 };
float limitDegrees(float degrees); //foward declaration
glm::mat4 viewChain{};
glm::mat3 modelTransform{};

double* GLPbo::depthBuffer = nullptr;    
glm::vec3 vectorL{};
GLuint tex_hdl{};
std::vector<glm::vec3>textureArray{};
/*
    @brief emulate the behaviors of vertex processor, rasterizer engine in graphics hardware, and fragment processor.
    @param none
    @return none
*/
void GLPbo::emulate() 
{
     // Create a rnadom number generator
    std::default_random_engine dre;
    dre.seed(30);
    std::uniform_real_distribution<float> urdf(0.f, 1.f);

    if (GLHelper::keystateM)
    {
        ++currentIndex;
        if (currentIndex == 2) { currentIndex = 0; }
        currModel->second.isRotating = false;
        cullFlag = true;
        cullCount = 0;
        GLHelper::keystateM = GL_FALSE;
    }
    selectModel = objectName[currentIndex];
        currModel = models.find(selectModel);
    GLPbo::Model& current_mdl = currModel->second;

    if (GLHelper::keystateW)
    {
        current_mdl.Tasking = static_cast<GLPbo::Model::modelType>(static_cast<int>(current_mdl.Tasking) + 1);
        if (static_cast<int>(current_mdl.Tasking) == 5)
        {
            current_mdl.Tasking = GLPbo::Model::modelType::wireframe;
        }
        GLHelper::keystateW = GL_FALSE;
    }

    if (GLHelper::keystateR)
    {
        currModel->second.isRotating = !currModel->second.isRotating;
        GLHelper::keystateR = GL_FALSE;
    }

    if (currModel->second.isRotating)
    {
        //currModel->second.angle = limitDegrees(currModel->second.angle + 1.f);
        ++currModel->second.angle;
    }

    ptr_to_pbo = static_cast<GLPbo::Color*>(glMapNamedBuffer(pboid, GL_WRITE_ONLY));
    clear_color_buffer();
    viewport_xform(current_mdl);
    // transform light pos using viewport trf
    //compute inverse of point to light

    //lightSource.position = current_mdl.modelTransform * lightSource.position;
    
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
            red = static_cast<GLubyte>(urdf(dre) * 255);
            blue = static_cast<GLubyte>(urdf(dre) * 255);
            green = static_cast<GLubyte>(urdf(dre) * 255);
            switch (current_mdl.Tasking)
            {
            case GLPbo::Model::modelType::wireframe:
                // The triangle is front-facing, so draw it.
                render_linebresenham(static_cast<int>(current_mdl.pd[idx1].x), static_cast<int>(current_mdl.pd[idx1].y), static_cast<int>(current_mdl.pd[idx2].x), static_cast<int>(current_mdl.pd[idx2].y), { 0, 0, 255 ,255 });
                render_linebresenham(static_cast<int>(current_mdl.pd[idx2].x), static_cast<int>(current_mdl.pd[idx2].y), static_cast<int>(current_mdl.pd[idx3].x), static_cast<int>(current_mdl.pd[idx3].y), { 0, 0, 255 ,255 });
                render_linebresenham(static_cast<int>(current_mdl.pd[idx3].x), static_cast<int>(current_mdl.pd[idx3].y), static_cast<int>(current_mdl.pd[idx1].x), static_cast<int>(current_mdl.pd[idx1].y), { 0, 0, 255 ,255 });
                mode = "Wireframe";
                break;
            //case GLPbo::Model::modelType::wireframeColor:
            //    render_linebresenham(static_cast<int>(current_mdl.pd[idx1].x), static_cast<int>(current_mdl.pd[idx1].y), static_cast<int>(current_mdl.pd[idx2].x), static_cast<int>(current_mdl.pd[idx2].y), { static_cast<GLubyte>(red * current_mdl.pd[idx1].x)    ,static_cast<GLubyte>(green * current_mdl.pd[idx1].x),static_cast<GLubyte>(blue * current_mdl.pd[idx1].y),255 });
            //    render_linebresenham(static_cast<int>(current_mdl.pd[idx2].x), static_cast<int>(current_mdl.pd[idx2].y), static_cast<int>(current_mdl.pd[idx3].x), static_cast<int>(current_mdl.pd[idx3].y), { static_cast<GLubyte>(red * current_mdl.pd[idx2].y),static_cast<GLubyte>(green * current_mdl.pd[idx2].x),static_cast<GLubyte>(blue * current_mdl.pd[idx2].y),255 });
            //    render_linebresenham(static_cast<int>(current_mdl.pd[idx3].x), static_cast<int>(current_mdl.pd[idx3].y), static_cast<int>(current_mdl.pd[idx1].x), static_cast<int>(current_mdl.pd[idx1].y), { static_cast<GLubyte>(red * current_mdl.pd[idx2].y)   ,static_cast<GLubyte>(green * current_mdl.pd[idx2].x),static_cast<GLubyte>(blue * current_mdl.pd[idx3].y),255 });
            //    mode = "Wireframe Color";
            //    break;
            case GLPbo::Model::modelType::faceted:
                render_triangle_faceted(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3], current_mdl.pm[idx1], current_mdl.pm[idx2], current_mdl.pm[idx3]);
                mode = "Faceted";
                break;
            case GLPbo::Model::modelType::depthBuffer:
                render_shadow_map(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3]);
                mode = "Depth Buffer";
                break;
            case GLPbo::Model::modelType::shaded:
                render_shadow_map_light(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3], 
                    current_mdl.nml[idx1] , current_mdl.nml[idx2] , current_mdl.nml[idx3], 
                    current_mdl.pm[idx1], current_mdl.pm[idx2], current_mdl.pm[idx3]);
                mode = "Shaded";
                break;
            case GLPbo::Model::modelType::textured:
                render_texture_map(current_mdl.pd[idx1], current_mdl.pd[idx2], current_mdl.pd[idx3],
                    current_mdl.tex[idx1], current_mdl.tex[idx2], current_mdl.tex[idx3]);
                mode = "Textured";
                break;
            }
        }
        else if (cullFlag)
        {
            ++cullCount;
        }
    }
    cullFlag = false;
    clear_depth_buffer();

    glUnmapNamedBuffer(pboid);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboid);
    glTextureSubImage2D(texid, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}
/*
    @brief draw the object using the shader program and the vertex array object
            also prints appropriate information to window's title bar
    @param none
    @return none
*/
void GLPbo::draw_fullwindow_quad() {
    std::stringstream sstr;
    // Bind texture unit
    glBindTextureUnit(0, texid);

    // Use program
    shdr_pgm.Use();

    // Get texture location for fragment shader use
    GLuint textureLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "uTex2d");
    glUniform1i(textureLoc, 0);

    // Bind the VAO
    glBindVertexArray(vaoid);

    // draw
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    shdr_pgm.UnUse();
    std::string modelName = currModel->first;

    sstr << std::fixed << std::setprecision(2) 
        << "A1 | Joey Chua Jun Yu | Model: " 
        << modelName << " | Mode: " << mode 
        << " | Vtx: " << currModel->second.pm.size() 
        << " | Tri: " << currModel->second.tri.size() / 3 
        << " | Culled: " << cullCount << " | FPS: " << GLHelper::fps;
    glfwSetWindowTitle(GLHelper::ptr_window, sstr.str().c_str());
}
/*
    @brief initialises parameters
    @param w - width of screen, h - height of screen
    @return none
*/
void GLPbo::init(GLsizei w, GLsizei h) {

    setup_texobj("../images/ogre.tex");

    CORE10::aspectRatio = GLHelper::width / GLHelper::height;
    CORE10::rightPlane = CORE10::aspectRatio * CORE10::topPlane,
    CORE10::leftPlane =  CORE10::aspectRatio * CORE10::bottomPlane;
    glm::vec3 up{ 0,1,0 };
    glm::mat4 view{};
    view = glm::lookAt(CORE10::eye, CORE10::target, up); 
    glm::mat4 ortho{};
    ortho = glm::ortho(CORE10::leftPlane, CORE10::rightPlane, CORE10::bottomPlane, CORE10::topPlane, CORE10::nearPlane, CORE10::farPlane);
    float arWidth = static_cast<float>(GLHelper::width * 0.5f);
    float arHeight = static_cast<float>(GLHelper::height * 0.5f);
    glm::mat4 viewport{
     arWidth,        0,   0, 0,
     0      , arHeight,   0, 0,
     0      ,        0, 0.5, 0,
     arWidth, arHeight, 0.5, 1.0
    };
    viewChain = viewport * ortho * view;
    width = w;
    height = h;
    pixel_cnt = w * h;
    byte_cnt = pixel_cnt * 4;
    set_clear_color(0, 0, 0);
    glCreateTextures(GL_TEXTURE_2D, 1, &texid);
    glTextureStorage2D(texid, 1, GL_RGBA8, width, height);
    glCreateBuffers(1, &pboid);
    glNamedBufferStorage(pboid,byte_cnt,nullptr,GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

    depthBuffer = new double[pixel_cnt];
   //set_depth_buffer(depthBuffer);
    objectName.push_back("cube");
    objectName.push_back("ogre");

    for (const auto& meshName : objectName)
    {   

        mdl.pm.clear();
        mdl.nml.clear();
        mdl.tex.clear();
        mdl.tri.clear();
        if (DPML::parse_obj_mesh("../meshes/" + meshName + ".obj", mdl.pm, mdl.nml, mdl.tex, mdl.tri, true, true, true))
        {            
            mdl.pd.resize(mdl.pm.size(), glm::vec3());
            viewport_xform(mdl);
            models[meshName] = mdl;
        }
    }

    currModel = models.begin();

    setup_quad_vao();
    setup_shdrpgm();
}

/*!
    @brief setup vao with the vertex pos and teture coordinates
    @param none
    @return none
*/
void GLPbo::setup_quad_vao()
{
    std::array<Vertex, 4> vtx{
    Vertex{{-1.f, -1.f}, {0,0}},
    Vertex{{ 1.f, -1.f}, {1,0}},
    Vertex{{-1.f,  1.f}, {0,1}},
    Vertex{{ 1.f,  1.f}, {1,1}}
    };

    //// transfer vertex position and color attributes to VBO
    GLuint vbo_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glNamedBufferStorage(vbo_hdl, sizeof(vtx), &vtx, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &vaoid);
    glVertexArrayVertexBuffer(vaoid, 0, vbo_hdl, 0, sizeof(Vertex));

    // for vertex position array, we use vertex attribute index 0
    // and vertex buffer binding point 3
    //pos
    glEnableVertexArrayAttrib(vaoid, 0);
    glVertexArrayAttribFormat(vaoid, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(vaoid, 0, 0);
    //texture
    glEnableVertexArrayAttrib(vaoid, 1);
    glVertexArrayAttribFormat(vaoid, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textCord));
    glVertexArrayAttribBinding(vaoid, 1, 0);

    std::array<GLushort, 4> idx_vtx{ 2, 0, 3, 1 };
    GLuint ebo_hdl;
    glCreateBuffers(1, &ebo_hdl);
    glNamedBufferStorage(ebo_hdl, sizeof(GLushort) * idx_vtx.size(),
        reinterpret_cast<GLvoid*>(idx_vtx.data()),
        GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayElementBuffer(vaoid, ebo_hdl);
    glBindVertexArray(0);
}

/*!
    @brief setup shader program and compiles them
    @param none
    @return none
*/
void GLPbo::setup_shdrpgm()
{
    std::string vtx_shdr =
        R"(
		#version 450 core
		layout (location=0) in vec2 aVertexPosition;
		layout (location=1) in vec2 textCord;
		layout (location=0) out vec2 vColor;
		void main() {
		gl_Position = vec4(aVertexPosition, 0.0, 1.0);
		vColor = textCord;
		}
	)";

    std::string frg_shdr =
        R"(
		#version 450 core

		layout(location=0) in vec2 vTexCoord;

		layout (location=0) out vec4 fFragColor;

		uniform sampler2D uTex2d;
		void main () {
			fFragColor = texture(uTex2d,vTexCoord);
		}
	)";

    shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, vtx_shdr);
    shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, frg_shdr);

    if (GL_FALSE == shdr_pgm.Link()) {
        std::cout << "Unable to compile/link/validate shader programs\n";
        std::cout << shdr_pgm.GetLog() << "\n";
        std::exit(EXIT_FAILURE);
    }
}
/*!
    @brief returns buffers back to GPU
    @param none
    @return none
*/
void GLPbo::cleanup() {
    glInvalidateBufferData(texid);
    glDeleteTextures(1, &texid);

    glInvalidateBufferData(pboid);
    glDeleteBuffers(1, &pboid);

    glInvalidateBufferData(vaoid);
    glDeleteBuffers(1, &vaoid);

    delete depthBuffer;
}

/*
    @brief sets color to specify
    @param GLPbo::Color color to set
    @return none
*/
void GLPbo::set_clear_color(GLPbo::Color clr) {
    clear_clr = clr;
}
//overload function
void GLPbo::set_clear_color(GLubyte _r, GLubyte _g, GLubyte _b, GLubyte _a) {
    clear_clr.rgba.r = _r;
    clear_clr.rgba.g = _g;
    clear_clr.rgba.b = _b;
    clear_clr.rgba.a = _a;
}
/*
    @brief emulate the behavior of GL command glClearColor()
    @param none
    @return none
*/
void GLPbo::clear_color_buffer() 
{
    std::fill(ptr_to_pbo, ptr_to_pbo + pixel_cnt, clear_clr);
}

/*
@brief Applies the viewport transformation to the model's points.
@param currentModel The Model object containing the points to transform.
@return None.
*/
void GLPbo::viewport_xform(Model& currentModel) {

    radians = glm::radians(currentModel.angle);
    double scaleVal;
   // std::cout << currModel->first;
    if (objectName[currentIndex] == "cube")
    {
        scaleVal = 1;
    }
    else
    {
        scaleVal = 2;
    }

    glm::mat3 scale
    {
            scaleVal,  0,          0,
            0,          scaleVal,  0,
            0,          0,          scaleVal
    };

    glm::mat3 rotation{
                        cos(radians),   0, -sin(radians),
                                   0,   1,    0,
                        sin(radians),   0,cos(radians)
    };

    currentModel.modelTransform = rotation;
    for (size_t i = 0; i < currentModel.pm.size(); i++)
    {
        // Apply the rotation
        glm::vec3 rotated = rotation * scale * currentModel.pm[i];
        // Convert the rotated vec3 to a vec4
        glm::vec4 rotated4(rotated, 1.0f);
        // Apply the viewport transformation
        glm::vec4 transformed = viewChain * rotated4;

        currentModel.pd[i] = glm::vec3(transformed);
        currentModel.pd[i].z = (currentModel.pd[i].z + 1) / 2;
    }
}
/*
@brief Sets the color of a pixel at the specified coordinates.
@param x The x-coordinate of the pixel.
       y The y-coordinate of the pixel.
       clr The color to set for the pixel.
@return None.
*/
void GLPbo::set_pixel(int x, int y, Color clr)
{
    if (x < 0 || x >= GLPbo::width || y < 0 || y >= GLPbo::height)
    {
        return;
    }
    else {
        int position = (GLPbo::width * y) + x;
        ptr_to_pbo[position] = clr;
    }

}
/*
@brief Renders a line using the Bresenham's line algorithm.
@param px0 The x-coordinate of the starting point.
       py0 The y-coordinate of the starting point.
       px1 The x-coordinate of the ending point.
       py1 The y-coordinate of the ending point.
       draw_clr The color to use for drawing the line.
@return None.
*/
void GLPbo::render_linebresenham(GLint px0, GLint py0,
    GLint px1, GLint py1, GLPbo::Color draw_clr) {
    int dx = px1 - px0, dy = py1 - py0;
    int xstep = (dx < 0) ? -1 : 1;
    int ystep = (dy < 0) ? -1 : 1;
    dx = (dx < 0) ? -dx : dx;
    dy = (dy < 0) ? -dy : dy;
    if (dx == 0)
    {
        // Vertical line
        for (int y = py0; y != py1; y += ystep)
        {
            set_pixel(px0, y, draw_clr);
        }
        set_pixel(px1, py1, draw_clr);
        return;
    }
    else if (dy == 0)
    {
        // Horizontal line
        for (int x = px0; x != px1; x += xstep) {
            set_pixel(x, py0, draw_clr);
        }
        set_pixel(px1, py1, draw_clr);
        return;
    }
    else if (dy > dx)
    {
        int d = 2 * dx - dy, dmin = 2 * dx, dmaj = 2 * dx - 2 * dy;
        set_pixel(px0, py0, draw_clr);
        while (--dy)
        {
            px0 += (d > 0) ? xstep : 0;
            d += (d > 0) ? dmaj : dmin;
            py0 += ystep;
            set_pixel(px0, py0, draw_clr);
        }
    }
    else if (dx > dy)
    {
        int d = 2 * dy - dx, dmin = 2 * dy, dmaj = 2 * dy - 2 * dx;
        set_pixel(px0, py0, draw_clr);
        while (--dx)
        {
            py0 += (d > 0) ? ystep : 0;
            d += (d > 0) ? dmaj : dmin;
            px0 += xstep;
            set_pixel(px0, py0, draw_clr);
        }
    }
}
/*
@brief Limits the input angle to the range of [0, 360) degrees.
@param degrees The angle in degrees.
@return Returns the angle limited to the range [0, 360) degrees.
*/
float limitDegrees(float degrees) {
    if (degrees >= 0) {
        degrees = static_cast<float>(fmod(degrees, 360.f));
    }
    else {
        degrees = static_cast<float>(fmod(degrees, -360.f));
    }
    return degrees;
}
/*
@brief Represents an edge equation and its top-left flag.
@param a The coefficient 'a' of the edge equation.
       b The coefficient 'b' of the edge equation.
       c The coefficient 'c' of the edge equation.
       topLeft The top-left flag indicating the position of the edge.
*/
struct Edge {
    double a{};
    double b{};
    double c{};

    bool topLeft{};
};

/*
@brief Computes the edge equation coefficients and top-left flag based on two given points.
@param p0 The first point of the edge.
       p1 The second point of the edge.
       E The Edge object to store the computed coefficients and top-left flag.
@return None.
*/
void computeEdgeEqn(glm::dvec3 const& p0, glm::dvec3 const& p1, Edge& E) 
{
    E.a = p0.y - p1.y;
    E.b = p1.x - p0.x;
    E.c = p0.x * p1.y - p1.x * p0.y;
    E.topLeft = (E.a != 0.0) ? (E.a > 0.0 ? true : false) : (E.b < 0.0 ? true : false);
}
/*
@brief Checks if a point is inside an edge based on the evaluation value and top-left flag.
@param eval The evaluation value of the edge equation.
       topLeft The top-left flag of the edge.
@return Returns a boolean value indicating whether the point is inside the edge.
*/
bool PointInEdgeTopLeft(double const& eval, bool& topLeft)
{
    return (eval > 0 || (eval == 0 && topLeft == true)) ? true : false;
}

/*
@brief Checks if a point is inside a triangle based on the evaluation values and top-left flags of the triangle edges.
@param eval0 The evaluation value of the first edge equation.
       eval1 The evaluation value of the second edge equation.
       eval2 The evaluation value of the third edge equation.
       topLeft0 The top-left flag of the first edge.
       topLeft1 The top-left flag of the second edge.
       topLeft2 The top-left flag of the third edge.
@return Returns a boolean value indicating whether the point is inside the triangle.
*/
bool PointInTriangle(double const& eval0, double const& eval1, double const& eval2, bool& topLeft0, bool& topLeft1, bool& topLeft2) {
    if (PointInEdgeTopLeft(eval0, topLeft0) && PointInEdgeTopLeft(eval1, topLeft1) && PointInEdgeTopLeft(eval2, topLeft2))
    {
        return true;
    }
    else 
    {
        return false;
    }
}
/*
@brief Computes the evaluation of the EdgeEqn at the top-left corner of the bounding box.
@param edgeObject The Edge object representing the edge equation.
       random_point The 3D coordinates of a random point used for evaluation.
@return Returns the evaluation result of the EdgeEqn at the top-left corner.
*/
double EdgeEqnTopLeft(Edge& edgeObject, glm::dvec3 randomPoint) {
    // this is eval of the EdgeEqn
    return (edgeObject.a * randomPoint.x + edgeObject.b * randomPoint.y + edgeObject.c);
}

/*
@brief Renders a filled triangle on the screen using the provided points and colors.
@param p0 The first vertex of the triangle.
       p1 The second vertex of the triangle.
       p2 The third vertex of the triangle.
       clr color of vertex
@return Returns a boolean value indicating the success of the rendering operation.
*/
bool GLPbo::render_triangle_faceted(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2, 
    glm::vec3 const& vert1, glm::vec3 const& vert2, glm::vec3 const& vert3) 
{
    
    glm::dvec3 P0P1 = vert2 - vert1;
    glm::dvec3 P0P2 = vert3 - vert1;
    glm::vec3 outwardNormal = glm::cross(P0P1, P0P2);
    //NORMALISED N
    outwardNormal = glm::normalize(outwardNormal);
    glm::vec3 centroid = static_cast<glm::vec3>((vert1 + vert3 + vert2) * 0.3333f);
    //lightSource.position = glm::inverse(models[currModel->second]);
        GLPbo::Model & current_mdl = currModel->second;
        //glm::mat4 rotato = glm::mat4(current_mdl.modelTransform,1.0);
    vectorL = glm::inverse(current_mdl.modelTransform) * CORE10::lightSource.position - centroid;
    //NORMLAIZED vectorL
    vectorL = glm::normalize(vectorL);
    glm::vec3 intensity{ 1,1,1 };
    glm::vec3 incomingLight = intensity * glm::max(0.f, glm::dot(outwardNormal, vectorL));
    Edge e0, e1, e2;
    // Edge equation of the 3 lines0
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
    double inverseArea = 1.0 / area;

    // Bounding box of the triangle
    double minX = floor(std::min({ p0.x, p1.x, p2.x }));
    double maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    double minY = floor(std::min({ p0.y, p1.y, p2.y }));
    double maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eval0 = EdgeEqnTopLeft(e0, { minX + 0.5, minY + 0.5, 0 });
    double eval1 = EdgeEqnTopLeft(e1, { minX + 0.5, minY + 0.5, 0 });
    double eval2 = EdgeEqnTopLeft(e2, { minX + 0.5, minY + 0.5, 0 });

    double barycentricA = eval0 * inverseArea;
    double barycentricB = eval1 * inverseArea;
    double barycentricC = 1 - barycentricA - barycentricB;//eval2 * inverseArea;

    double incXA = e0.a * inverseArea;
    double incXB = e1.a * inverseArea;
    double incXC = e2.a * inverseArea;

    double incYA = e0.b * inverseArea;
    double incYB = e1.b * inverseArea;
    double incYC = e2.b * inverseArea;


    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;
    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);


    for (int y = intMinY; y < intMaxY; ++y) {
        double Heval0 = eval0;
        double Heval1 = eval1;
        double Heval2 = eval2;

        double HEa = barycentricA;
        double HEb = barycentricB;
        double HEc = barycentricC;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangle(Heval0, Heval1, Heval2, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                int bufferIndex = y * GLPbo::width + x;  // Adjusted buffer index calculation
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                if (z <= depthBuffer[bufferIndex])
                {
                    depthBuffer[bufferIndex] = z;
                    Color clr = { static_cast<GLubyte>(incomingLight.x * 255),
                        static_cast<GLubyte>(incomingLight.y * 255) ,
                        static_cast<GLubyte>(incomingLight.z* 255)  };
                    set_pixel(x, y, clr );
                }
            }
            Heval0 += e0a;
            Heval1 += e1a;
            Heval2 += e2a;

            HEa += incXA;
            HEb += incXB;
            HEc += incXC;
        }
        eval0 += e0b;
        eval1 += e1b;
        eval2 += e2b;

        barycentricA += incYA;
        barycentricB += incYB;
        barycentricC += incYC;
    }

    return true;
}

/*
    @brief emulate the behavior of GL command glClearColor()
    @param none
    @return none
*/
void GLPbo::clear_depth_buffer()
{
    std::fill(depthBuffer, depthBuffer + pixel_cnt, 1.0);
}

/*
@brief Renders a filled triangle on the screen using the provided points and colors.
@param p0 The first vertex of the triangle.
       p1 The second vertex of the triangle.
       p2 The third vertex of the triangle.
       c0 The color at the first vertex.
       c1 The color at the second vertex.
       c2 The color at the third vertex.
@return Returns a boolean value indicating the success of the rendering operation.
*/
bool GLPbo::render_shadow_map(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2) {

    Edge e0, e1, e2;
    // Edge equation of the 3 lines0
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
    double inverseArea = 1.0 / area;

    // Bounding box of the triangle
    double minX = floor(std::min({ p0.x, p1.x, p2.x }));
    double maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    double minY = floor(std::min({ p0.y, p1.y, p2.y }));
    double maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eval0 = EdgeEqnTopLeft(e0, { minX + 0.5, minY + 0.5, 0 });
    double eval1 = EdgeEqnTopLeft(e1, { minX + 0.5, minY + 0.5, 0 });
    double eval2 = EdgeEqnTopLeft(e2, { minX + 0.5, minY + 0.5, 0 });

    double barycentricA = eval0 * inverseArea;
    double barycentricB = eval1 * inverseArea;
    double barycentricC = 1 - barycentricA - barycentricB;//eval2 * inverseArea;

    double incXA = e0.a * inverseArea;
    double incXB = e1.a * inverseArea;
    double incXC = e2.a * inverseArea;

    double incYA = e0.b * inverseArea;
    double incYB = e1.b * inverseArea;
    double incYC = e2.b * inverseArea;


    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;
    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);


    for (int y = intMinY; y < intMaxY; ++y) {
        double Heval0 = eval0;
        double Heval1 = eval1;
        double Heval2 = eval2;

        double HEa = barycentricA;
        double HEb = barycentricB;
        double HEc = barycentricC;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangle(Heval0, Heval1, Heval2, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                int bufferIndex = y * GLPbo::width + x;  // Adjusted buffer index calculation
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                if (z <= depthBuffer[bufferIndex])
                {
                    depthBuffer[bufferIndex] = z;
                    set_pixel(x, y, { static_cast<GLubyte>(z * 255.0),static_cast<GLubyte>(z * 255.0),static_cast<GLubyte>(z * 255.0) });
                }
            }
            Heval0 += e0a;
            Heval1 += e1a;
            Heval2 += e2a;

            HEa += incXA;
            HEb += incXB;
            HEc += incXC;
        }
        eval0 += e0b;
        eval1 += e1b;
        eval2 += e2b;

        barycentricA += incYA;
        barycentricB += incYB;
        barycentricC += incYC;

    }
    return true;
}

/*
@brief Renders a filled triangle on the screen using the provided points and colors.
@param p0 The first vertex of the triangle.
       p1 The second vertex of the triangle.
       p2 The third vertex of the triangle.
       c0 The color at the first vertex.
       c1 The color at the second vertex.
       c2 The color at the third vertex.
@return Returns a boolean value indicating the success of the rendering operation.
*/
bool GLPbo::render_shadow_map_light(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2,
    glm::vec3 const& c0, glm::vec3 const& c1, glm::vec3 const& c2,
    glm::vec3 const& vert1, glm::vec3 const& vert2, glm::vec3 const& vert3)
{
    
    //NORMALISED N

    GLPbo::Model& current_mdl = currModel->second;
    //pm0 to lightsource pos
    glm::vec3 pm0 = glm::inverse(current_mdl.modelTransform) * CORE10::lightSource.position - vert1;
    glm::vec3 pm1 = glm::inverse(current_mdl.modelTransform) * CORE10::lightSource.position - vert2;
    glm::vec3 pm2 = glm::inverse(current_mdl.modelTransform) * CORE10::lightSource.position - vert3;
    pm0 = glm::normalize(pm0);
    pm1 = glm::normalize(pm1);
    pm2 = glm::normalize(pm2);
    glm::dvec3 incomingLight1 = CORE10::lightSource.intensity * glm::max(0.f, glm::dot(c0, pm0));
    glm::dvec3 incomingLight2 = CORE10::lightSource.intensity * glm::max(0.f, glm::dot(c1, pm1));
    glm::dvec3 incomingLight3 = CORE10::lightSource.intensity * glm::max(0.f, glm::dot(c2, pm2));
    Edge e0, e1, e2;
    // Edge equation of the 3 lines0
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
    double inverseArea = 1.0 / area;

    // Bounding box of the triangle
    double minX = floor(std::min({ p0.x, p1.x, p2.x }));
    double maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    double minY = floor(std::min({ p0.y, p1.y, p2.y }));
    double maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eval0 = EdgeEqnTopLeft(e0, { minX + 0.5, minY + 0.5, 0 });
    double eval1 = EdgeEqnTopLeft(e1, { minX + 0.5, minY + 0.5, 0 });
    double eval2 = EdgeEqnTopLeft(e2, { minX + 0.5, minY + 0.5, 0 });

    double barycentricA = eval0 * inverseArea;
    double barycentricB = eval1 * inverseArea;
    double barycentricC = 1 - barycentricA - barycentricB;//eval2 * inverseArea;

    double incXA = e0.a * inverseArea;
    double incXB = e1.a * inverseArea;
    double incXC = e2.a * inverseArea;

    double incYA = e0.b * inverseArea;
    double incYB = e1.b * inverseArea;
    double incYC = e2.b * inverseArea;


    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;
    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);


    for (int y = intMinY; y < intMaxY; ++y) {
        double Heval0 = eval0;
        double Heval1 = eval1;
        double Heval2 = eval2;

        double HEa = barycentricA;
        double HEb = barycentricB;
        double HEc = barycentricC;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangle(Heval0, Heval1, Heval2, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                int bufferIndex = y * GLPbo::width + x;  // Adjusted buffer index calculation
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                if (z <= depthBuffer[bufferIndex])
                {
                    glm::dvec3 clr = HEa * incomingLight1 + HEb * incomingLight2 + HEc * incomingLight3;
                    depthBuffer[bufferIndex] = z;
                    set_pixel(x, y, { static_cast<GLubyte>(clr.x * 255.0),static_cast<GLubyte>(clr.y * 255.0),static_cast<GLubyte>(clr.z * 255.0) });
                }
            }
            Heval0 += e0a;
            Heval1 += e1a;
            Heval2 += e2a;

            HEa += incXA;
            HEb += incXB;
            HEc += incXC;
        }
        eval0 += e0b;
        eval1 += e1b;
        eval2 += e2b;

        barycentricA += incYA;
        barycentricB += incYB;
        barycentricC += incYC;

    }
    return true;
}

void GLPbo::setup_texobj(std::string pathname) {

    std::ifstream ifs{ pathname, std::ios::binary };
    if (!ifs) {
        std::cout << "ERROR: Unable to open scene file: "
            << pathname << "\n";
        exit(EXIT_FAILURE);
    }
    //ifs.seekg(0, std::ios::end);
    ifs.seekg(0, std::ios::beg);
    // remember all our images have width and height of 256 texels and
    // use 32-bit RGBA texel format
    //GLuint width, height, bytes_per_texel;
    ifs.read(reinterpret_cast<char*>(&CORE10::textureWidth), sizeof(int));
    ifs.read(reinterpret_cast<char*>(&CORE10::textureHeight), sizeof(int));
    ifs.read(reinterpret_cast<char*>(&CORE10::bytes_per_texel), sizeof(int));

    if (!ifs) {
        std::cout << "ERROR: Failed to read file: "
            << pathname << "\n";
        exit(EXIT_FAILURE);
    }

    for (GLuint i = 0; i < CORE10::textureWidth * CORE10::textureHeight * CORE10::bytes_per_texel; ++i)
    {
        unsigned char r, g, b;
        ifs.read(reinterpret_cast<char*>(&r), 1);
        ifs.read(reinterpret_cast<char*>(&g), 1);
        ifs.read(reinterpret_cast<char*>(&b), 1);
        
        textureArray.push_back(glm::vec3(r,g,b));
    }
    ifs.close();

}

// increment tex cord read from file 
//my tex cord is the pixel position in the vector of textureArray
//for(position: tex)
//{
// textureArray[position]
//}
// replace pd with tex cord
bool GLPbo::render_texture_map(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2, 
    glm::dvec2 const& texCord0, glm::dvec2 const& texCord1, glm::dvec2 const& texCord2)
{
    Edge e0, e1, e2;
    // Edge equation of the 3 lines0
    computeEdgeEqn(p1, p2, e0);
    computeEdgeEqn(p2, p0, e1);
    computeEdgeEqn(p0, p1, e2);

    double area = ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
    double inverseArea = 1.0 / area ;

    // Bounding box of the triangle
    double minX = floor(std::min({ p0.x, p1.x, p2.x }));
    double maxX = ceil(std::max({ p0.x, p1.x, p2.x }));
    double minY = floor(std::min({ p0.y, p1.y, p2.y }));
    double maxY = ceil(std::max({ p0.y, p1.y, p2.y }));

    double eval0 = EdgeEqnTopLeft(e0, { minX + 0.5, minY + 0.5, 0 });
    double eval1 = EdgeEqnTopLeft(e1, { minX + 0.5, minY + 0.5, 0 });
    double eval2 = EdgeEqnTopLeft(e2, { minX + 0.5, minY + 0.5, 0 });
    //tex cord t0 t1 t2 * eval1 eval2 eval3


    double barycentricA = eval0 * inverseArea;
    double barycentricB = eval1 * inverseArea;
    double barycentricC = 1 - barycentricA - barycentricB;//eval2 * inverseArea;

    double incXA = e0.a * inverseArea;
    double incXB = e1.a * inverseArea;
    double incXC = e2.a * inverseArea;

    double incYA = e0.b * inverseArea;
    double incYB = e1.b * inverseArea;
    double incYC = e2.b * inverseArea;


    double e0a = e0.a, e1a = e1.a, e2a = e2.a;
    double e0b = e0.b, e1b = e1.b, e2b = e2.b;
    int intMaxX = static_cast<int>(maxX);
    int intMaxY = static_cast<int>(maxY);
    int intMinX = static_cast<int>(minX);
    int intMinY = static_cast<int>(minY);


    for (int y = intMinY; y < intMaxY; ++y) {
        double Heval0 = eval0;
        double Heval1 = eval1;
        double Heval2 = eval2;

        double HEa = barycentricA;
        double HEb = barycentricB;
        double HEc = barycentricC;

        for (int x = intMinX; x < intMaxX; ++x)
        {
            if (PointInTriangle(Heval0, Heval1, Heval2, e0.topLeft, e1.topLeft, e2.topLeft))
            {
                glm::dvec2 texPos = (Heval0 * texCord0 + Heval1 * texCord1 + Heval2 * texCord2) * inverseArea;
                int bufferIndex = y * GLPbo::width + x;  // Adjusted buffer index calculation
                double z = HEa * p0.z + HEb * p1.z + HEc * p2.z;
                int texX = static_cast<int>(std::clamp(texPos.x,0.0,1.0) * (CORE10::textureHeight-1));
                int texY = static_cast<int>(std::clamp(texPos.y,0.0,1.0) * (CORE10::textureWidth-1));
                if (z <= depthBuffer[bufferIndex]&& texX>0 && texY > 0 && texX < CORE10::textureHeight && texY < CORE10::textureWidth)
                {
                    int textureIndex = texY * CORE10::textureWidth + texX;
                    glm::vec3 texClr = textureArray[textureIndex];
                    depthBuffer[bufferIndex] = z;   
                    set_pixel(x, y, { static_cast<GLubyte>(texClr.x),static_cast<GLubyte>(texClr.y),static_cast<GLubyte>(texClr.z) });
                }
            }
            Heval0 += e0a;
            Heval1 += e1a;
            Heval2 += e2a;

            HEa += incXA;
            HEb += incXB;
            HEc += incXC;
        }
        eval0 += e0b;
        eval1 += e1b;
        eval2 += e2b;

        barycentricA += incYA;
        barycentricB += incYB;
        barycentricC += incYC;

    }
    return true;
}