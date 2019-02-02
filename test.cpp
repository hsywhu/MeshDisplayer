/*
    Sample code by Wallace Lira <http://www.sfu.ca/~wpintoli/> based on
    the four Nanogui examples and also on the sample code provided in
          https://github.com/darrenmothersele/nanogui-test
    
    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/glcanvas.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#pragma warning(pop)
#endif
#if defined(_WIN32)
#if defined(APIENTRY)
#undef APIENTRY
#endif
#include <windows.h>
#endif

using std::cerr;
using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::to_string;
using std::vector;

using nanogui::Arcball;
using nanogui::Button;
using nanogui::CheckBox;
using nanogui::GroupLayout;
using nanogui::Label;
using nanogui::MatrixXf;
using nanogui::MatrixXu;
using nanogui::Screen;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::Window;

class MyGLCanvas : public nanogui::GLCanvas
{
  public:
    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent)
    {
        using namespace nanogui;

        mShader.initFromFiles("a_smooth_shader", "StandardShading.vertexshader", "StandardShading.fragmentshader");

        mArcball = new Arcball();
        mArcball->setSize({400, 400});

        /* MatrixXf is just a dynamic sized matrix of floats initialized as 3 x 8 */
        /* which are the 3 dimensions and 8 possible vertice positions */
        positions.col(0) << 1, 1, -1;  //top right back
        positions.col(1) << -1, 1, -1; //bottom right back
        positions.col(2) << -1, 1, 1;  //bottom right front

        positions.col(3) << 1, 1, -1; //top right back
        positions.col(4) << -1, 1, 1; //bottom right front
        positions.col(5) << 1, 1, 1;  //top right front

        positions.col(6) << 1, 1, -1;   //top right back
        positions.col(7) << 1, -1, -1;  //top left back
        positions.col(8) << -1, -1, -1; //bottom left back

        positions.col(9) << 1, 1, -1;    //top right back
        positions.col(10) << -1, -1, -1; //bottom left back
        positions.col(11) << -1, 1, -1;  //bottom right back

        positions.col(12) << -1, 1, -1;  //bottom right back
        positions.col(13) << -1, -1, -1; //bottom left back
        positions.col(14) << -1, -1, 1;  //bottom left front

        positions.col(15) << -1, 1, -1; //bottom right back
        positions.col(16) << -1, -1, 1; //bottom left front
        positions.col(17) << -1, 1, 1;  //bottom right front

        positions.col(18) << -1, 1, 1;  //bottom right front
        positions.col(19) << -1, -1, 1; //bottom left front
        positions.col(20) << 1, -1, 1;  //top left front

        positions.col(21) << -1, 1, 1; //bottom right front
        positions.col(22) << 1, -1, 1; //top left front
        positions.col(23) << 1, 1, 1;  //top right front

        positions.col(24) << 1, 1, 1;   //top right front
        positions.col(25) << 1, -1, 1;  //top left front
        positions.col(26) << 1, -1, -1; //top left back

        positions.col(27) << 1, 1, 1;   //top right front
        positions.col(28) << 1, -1, -1; //top left back
        positions.col(29) << 1, 1, -1;  //top right back

        positions.col(30) << 1, -1, -1; //top left back
        positions.col(31) << 1, -1, 1;  //top left front
        positions.col(32) << -1, -1, 1; //bottom left front

        positions.col(33) << 1, -1, -1;  //top left back
        positions.col(34) << -1, -1, 1;  //bottom left front
        positions.col(35) << -1, -1, -1; //bottom left back

        //lines
        //notice that the line is jagged. You actually need to move them slightly out
        //To do that on an arbitrary mesh, just use the surface normals to your advantage!

        positions.col(36) << 1.005, 1.005, -1.005;  //top right back
        positions.col(37) << -1.005, 1.005, -1.005; //bottom right back

        positions.col(38) << -1.005, 1.005, -1.005; //bottom right back
        positions.col(39) << -1.005, 1.005, 1.005;  //bottom right front

        positions.col(40) << -1.005, 1.005, 1.005; //bottom right front
        positions.col(41) << 1.005, 1.005, -1.005; //top right back

        positions.col(42) << 1.005, 1.005, -1.005; //top right back
        positions.col(43) << -1.005, 1.005, 1.005; //bottom right front

        positions.col(44) << -1.005, 1.005, 1.005; //bottom right front
        positions.col(45) << 1.005, 1.005, 1.005;  //top right front

        positions.col(46) << 1.005, 1.005, -1.005; //top right back
        positions.col(47) << 1.005, 1.005, 1.005;  //top right front

        /* these are the vertex normals, normally you need to calculate this using face normals*/
        /* in turn, to calculate a face normal you need the cross product of face points*/
        /* this trick only works because it is a simple cube centered at the origin */
        
        normals.col(0) << Vector3f(1, 1, -1).normalized();
        normals.col(1) << Vector3f(-1, 1, -1).normalized();
        normals.col(2) << Vector3f(-1, 1, 1).normalized();

        normals.col(3) << Vector3f(1, 1, -1).normalized();
        normals.col(4) << Vector3f(-1, 1, 1).normalized();
        normals.col(5) << Vector3f(1, 1, 1).normalized();

        normals.col(6) << Vector3f(1, 1, -1).normalized();
        normals.col(7) << Vector3f(1, -1, -1).normalized();
        normals.col(8) << Vector3f(-1, -1, -1).normalized();

        normals.col(9) << Vector3f(1, 1, -1).normalized();
        normals.col(10) << Vector3f(-1, -1, -1).normalized();
        normals.col(11) << Vector3f(-1, 1, -1).normalized();

        normals.col(12) << Vector3f(-1, 1, -1).normalized();
        normals.col(13) << Vector3f(-1, -1, -1).normalized();
        normals.col(14) << Vector3f(-1, -1, 1).normalized();

        normals.col(15) << Vector3f(-1, 1, -1).normalized();
        normals.col(16) << Vector3f(-1, -1, 1).normalized();
        normals.col(17) << Vector3f(-1, 1, 1).normalized();

        normals.col(18) << Vector3f(-1, 1, 1).normalized();
        normals.col(19) << Vector3f(-1, -1, 1).normalized();
        normals.col(20) << Vector3f(1, -1, 1).normalized();

        normals.col(21) << Vector3f(-1, 1, 1).normalized();
        normals.col(22) << Vector3f(1, -1, 1).normalized();
        normals.col(23) << Vector3f(1, 1, 1).normalized();

        normals.col(24) << Vector3f(1, 1, 1).normalized();
        normals.col(25) << Vector3f(1, -1, 1).normalized();
        normals.col(26) << Vector3f(1, -1, -1).normalized();

        normals.col(27) << Vector3f(1, 1, 1).normalized();
        normals.col(28) << Vector3f(1, -1, -1).normalized();
        normals.col(29) << Vector3f(1, 1, -1).normalized();

        normals.col(30) << Vector3f(1, -1, -1).normalized();
        normals.col(31) << Vector3f(1, -1, 1).normalized();
        normals.col(32) << Vector3f(-1, -1, 1).normalized();

        normals.col(33) << Vector3f(1, -1, -1).normalized();
        normals.col(34) << Vector3f(-1, -1, 1).normalized();
        normals.col(35) << Vector3f(-1, -1, -1).normalized();

        //line normals

        normals.col(36) << Vector3f(1, 1, -1).normalized();
        normals.col(37) << Vector3f(-1, 1, -1).normalized();

        normals.col(38) << Vector3f(-1, 1, -1).normalized();
        normals.col(39) << Vector3f(-1, 1, 1).normalized();

        normals.col(40) << Vector3f(1, 1, -1).normalized();
        normals.col(41) << Vector3f(-1, 1, 1).normalized();

        normals.col(42) << Vector3f(1, 1, -1).normalized();
        normals.col(43) << Vector3f(-1, 1, 1).normalized();

        normals.col(44) << Vector3f(-1, 1, 1).normalized();
        normals.col(45) << Vector3f(1, 1, 1).normalized();

        normals.col(46) << Vector3f(1, 1, -1).normalized();
        normals.col(47) << Vector3f(1, 1, 1).normalized();

        /* Each vertice can have a color too (rgb in this case) */
        /* the face will interpolate the color of its vertices to positionsa gradient */

        //just a red cube by default
        colors.col(0) << 1, 0, 0;
        colors.col(1) << 1, 0, 0;
        colors.col(2) << 1, 0, 0;
        colors.col(3) << 1, 0, 0;
        colors.col(4) << 1, 0, 0;
        colors.col(5) << 1, 0, 0;
        colors.col(6) << 1, 0, 0;
        colors.col(7) << 1, 0, 0;
        colors.col(8) << 1, 0, 0;
        colors.col(9) << 1, 0, 0;
        colors.col(10) << 1, 0, 0;
        colors.col(11) << 1, 0, 0;
        colors.col(12) << 1, 0, 0;
        colors.col(13) << 1, 0, 0;
        colors.col(14) << 1, 0, 0;
        colors.col(15) << 1, 0, 0;
        colors.col(16) << 1, 0, 0;
        colors.col(17) << 1, 0, 0;
        colors.col(18) << 1, 0, 0;
        colors.col(19) << 1, 0, 0;
        colors.col(20) << 1, 0, 0;
        colors.col(21) << 1, 0, 0;
        colors.col(22) << 1, 0, 0;
        colors.col(23) << 1, 0, 0;
        colors.col(24) << 1, 0, 0;
        colors.col(25) << 1, 0, 0;
        colors.col(26) << 1, 0, 0;
        colors.col(27) << 1, 0, 0;
        colors.col(28) << 1, 0, 0;
        colors.col(29) << 1, 0, 0;
        colors.col(30) << 1, 0, 0;
        colors.col(31) << 1, 0, 0;
        colors.col(32) << 1, 0, 0;
        colors.col(33) << 1, 0, 0;
        colors.col(34) << 1, 0, 0;
        colors.col(35) << 1, 0, 0;

        //line colors
        colors.col(36) << 0, 0, 0;
        colors.col(37) << 0, 0, 0;
        colors.col(38) << 0, 0, 0;
        colors.col(39) << 0, 0, 0;
        colors.col(40) << 0, 0, 0;
        colors.col(41) << 0, 0, 0;
        colors.col(42) << 0, 0, 0;
        colors.col(43) << 0, 0, 0;
        colors.col(44) << 0, 0, 0;
        colors.col(45) << 0, 0, 0;
        colors.col(46) << 0, 0, 0;
        colors.col(47) << 0, 0, 0;

        // After binding the shader to the current context we can send data to opengl that will be handled
        // by the vertex shader and then by the fragment shader, in that order.
        // if you want to know more about modern opengl pipeline take a look at this link
        // https://www.khronos.org/opengl/wiki/Rendering_Pipeline_Overview
        mShader.bind();

        //mShader.uploadIndices(indices);
        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);
        mShader.uploadAttrib("vertexNormal_modelspace", normals);

        // ViewMatrixID
        // change your rotation to work on the camera instead of rotating the entire world with the MVP matrix
        Matrix4f V;
        V.setIdentity();
        //V = lookAt(Vector3f(0,12,0), Vector3f(0,0,0), Vector3f(0,1,0));
        mShader.setUniform("V", V);

        //ModelMatrixID
        Matrix4f M;
        M.setIdentity();
        mShader.setUniform("M", M);

        // This the light origin position in your environment, which is totally arbitrary
        // however it is better if it is behind the observer
        mShader.setUniform("LightPosition_worldspace", Vector3f(5, -5, -5));

        translation_.setIdentity();
        scale_.setIdentity();
        scale_(0, 0) = 0.25f;
        scale_(1, 1) = 0.25f;
        scale_(2, 2) = 0.25f;
        view_.setIdentity();
        projection_.setIdentity();
    }

    //flush data on call
    ~MyGLCanvas()
    {
        mShader.free();
    }

    //Method to update the rotation on each axis
    void setRotation(nanogui::Vector3f vRotation)
    {
        mRotation = vRotation;
    }

    void setRotation4x4(nanogui::Matrix4f vRotation)
    {
        mRotation4x4 = vRotation;
    }

    //Method to update the mesh itself, can change the size of it dynamically, as shown later
    void updateMeshPositions(MatrixXf newPositions, int new_num_triangles)
    {
        positions = newPositions;
        num_triangles = new_num_triangles;
    }

    void updateMeshColors(MatrixXf newColors, int new_num_triangles)
    {
        colors = newColors;
        num_triangles = new_num_triangles;
    }

    void updateMeshNormals(MatrixXf newNormals, MatrixXf newNormals_flat, int new_num_triangles)
    {
        normals = newNormals;
        normals_flat = newNormals_flat;
        num_triangles = new_num_triangles;
    }

    void updateRenderMode(int new_render_mode)
    {
        render_mode = new_render_mode;
    }

    //This is how you capture mouse events in the screen. If you want to implement the arcball instead of using
    //sliders, then you need to map the right click drag motions to suitable rotation matrices
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override {
        // In this example, we are using the left mouse button
        // to control the arcball motion
        if (button == GLFW_MOUSE_BUTTON_2) {
            mArcball->button(p, down);
            return true;
        }
        if (button == GLFW_MOUSE_BUTTON_1) {
            right_mouse_down = down;
            // cout << "right_mouse " << right_mouse_down << endl;
            return true;
        }
        return false;
    }

    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override
    {
        if (button == GLFW_MOUSE_BUTTON_3)
        {
            mArcball->motion(p);
            return true;
        }
        if (button == GLFW_MOUSE_BUTTON_2){
            if(right_mouse_down){
                translation_(0, 3) += rel.x()*0.005;
                translation_(1, 3) -= rel.y()*0.005;
            }
            return true;
        }
        return false;
    }

    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel){
        scale_(0, 0) += rel.y()*0.015;
        scale_(1, 1) += rel.y()*0.015;
        scale_(2, 2) += rel.y()*0.015;
        return true;
    }

    //OpenGL calls this method constantly to update the screen.
    virtual void drawGL() override
    {
        using namespace nanogui;
        nanogui::Matrix4f R = mArcball->matrix();
        setRotation4x4(R);
        //refer to the previous explanation of mShader.bind();
        mShader.bind();

        //this simple command updates the positions matrix. You need to do the same for color and indices matrices too
        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);

        if(render_mode == 1 || render_mode == 4)
            mShader.uploadAttrib("vertexNormal_modelspace", normals_flat);
        else
            mShader.uploadAttrib("vertexNormal_modelspace", normals);

        //This is a way to perform a simple rotation using a 4x4 rotation matrix represented by rmat
        //mvp stands for ModelViewProjection matrix
        Matrix4f mvp;
        mvp = projection_ * view_ * (translation_ * mRotation4x4 * scale_);
        mShader.setUniform("MVP", mvp);

        // If enabled, does depth comparisons and update the depth buffer.
        // Avoid changing if you are unsure of what this means.
        glEnable(GL_DEPTH_TEST);

        // draw triangles and lines according to render mode
        switch(render_mode){
            case 0: mShader.drawArray(GL_TRIANGLES, 0, num_triangles * 3); break;
            case 1: mShader.drawArray(GL_TRIANGLES, 0, num_triangles * 3); break;
            case 2: mShader.drawArray(GL_LINES, num_triangles * 3, num_triangles * 3 * 2); break;
            case 3: mShader.drawArray(GL_TRIANGLES, 0, num_triangles * 3);
                    mShader.drawArray(GL_LINES, num_triangles * 3, num_triangles * 3 * 2);
                    break;
            case 4: mShader.drawArray(GL_TRIANGLES, 0, num_triangles * 3);
                    mShader.drawArray(GL_LINES, num_triangles * 3, num_triangles * 3 * 2);
                    break;
            default: cout << "render mode error" << endl; break;
        }
        glDisable(GL_DEPTH_TEST);
    }

    //Instantiation of the variables that can be acessed outside of this class to interact with the interface
    //Need to be updated if a interface element is interacting with something that is inside the scope of MyGLCanvas
private:
    MatrixXf positions = MatrixXf(3, 48);
    MatrixXf colors = MatrixXf(3, 48);
    MatrixXf normals = MatrixXf(3, 48);
    MatrixXf normals_flat = MatrixXf(3, 48);
    int num_triangles = 12;
    bool right_mouse_down = false;

    nanogui::Matrix4f translation_;
    nanogui::Matrix4f scale_;
    nanogui::Matrix4f projection_;
    nanogui::Matrix4f view_;
    /*
        render_mode:
            0: smooth shaded
            1: flat shaded
            2: wireframe
            3: smooth shaded with wireframe
            4: flat shaded with wireframe
    */
    int render_mode = 0;
    nanogui::GLShader mShader;
    Eigen::Vector3f mRotation;
    nanogui::Matrix4f mRotation4x4;
    nanogui::Arcball *mArcball;
};

class ExampleApplication : public nanogui::Screen
{
  public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(900, 600), "NanoGUI Cube and Menus", false)
    {
        using namespace nanogui;

        //OpenGL canvas demonstration
        //First, we need to create a window context in which we will render both the interface and OpenGL canvas
        Window *window = new Window(this, "GLCanvas Demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        //OpenGL canvas initialization, we can control the background color and also its size
        mCanvas = new MyGLCanvas(window);
        mCanvas->setBackgroundColor({100, 100, 100, 255});
        mCanvas->setSize({400, 400});

        //This is how we add widgets, in this case, they are connected to the same window as the OpenGL canvas
        Widget *tools = new Widget(window);

        //Then, we can create another window and insert other widgets into it
        Window *anotherWindow = new Window(this, "Basic widgets");
        anotherWindow->setPosition(Vector2i(500, 15));
        anotherWindow->setLayout(new GroupLayout());

        // Demonstrates how a button called "New Mesh" can update the positions matrix.
        // This is just a demonstration, you actually need to bind mesh updates with the open file interface
        Button *button = new Button(anotherWindow, "New Mesh");
        button->setCallback([&] {
            //MatrixXf has dynamic size, so you can actually change its dimensions on the fly here
            //Make sure that the new mesh is not overblown by scaling it to a proper size and centering at origin
            //If you do not do that, the object may not appear at all, impacting the tests
            MatrixXf newPositions = MatrixXf(3, 12);
            newPositions.col(0) << 0, 0, 0; //top right back
            newPositions.col(1) << 1, 0, 0; //bottom right back
            newPositions.col(2) << 0, 0, 1; //bottom right back

            newPositions.col(3) << 0, 0, 0; //bottom right front
            newPositions.col(4) << 0, 1, 0; //bottom right front
            newPositions.col(5) << 0, 0, 1; //top right back

            newPositions.col(6) << 0, 0, 0; //top right back
            newPositions.col(7) << 1, 0, 0; //bottom right front
            newPositions.col(8) << 0, 1, 0; //bottom right front

            newPositions.col(9) << 0, 0, 1;  //top right front
            newPositions.col(10) << 1, 0, 0; //top right back
            newPositions.col(11) << 0, 1, 0; //top right front
            mCanvas->updateMeshPositions(newPositions, 4);
        });
        button->setTooltip("Demonstrates how a button can update the positions matrix.");

        //Here is how you can get the string that represents file paths both for opening and for saving.
        //you need to implement the rest of the parser logic.
        new Label(anotherWindow, "File dialog", "sans-bold");
        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        Button *b = new Button(tools, "Open");
        b->setCallback([&] {
            string file_dir = file_dialog({{"obj", "3D image file"}}, false);
            cout << "File dialog result: " << file_dir << endl;
            string line;
            std::ifstream obj_file(file_dir);
            
            if (obj_file.is_open())
            {
                v_vertex.clear();
                v_faces.clear();
                v_Wedges.clear();
                while(getline(obj_file, line)){
                    trim(line);
                    if(line[0] == '#')
                        continue;
                    else if(line[0] == 'v'){
                        line.erase(0, 2);   //remain only position data
                        vector<float> vertex_p;
                        get_position(line, ' ', vertex_p);
                        
                        //start loading vertex data to vector
                        Vertex *vertex_temp = new Vertex(vertex_p[0], vertex_p[1], vertex_p[2]);
                        v_vertex.push_back(vertex_temp);
                        vertex_idx_map[vertex_temp] = v_vertex.size() - 1;
                        
                    }else if(line[0] == 'f'){
                        line.erase(0, 2);
                        vector<int> face_idx;
                        get_index(line, ' ', face_idx);
                        Face *face_temp = new Face();
                        v_faces.push_back(face_temp);
                        face_idx_map[face_temp] = v_faces.size() - 1;
                        
                        // start loading edge and face data to vector
                        for(int i = 0; i < face_idx.size(); i++){
                            Vertex *v0 = v_vertex[face_idx[i] - 1];
                            Vertex *v1 = v_vertex[face_idx[(i+1)%face_idx.size()] - 1];
                            W_edge *edge_temp = new W_edge(v0, v1);
                            pair<int, int> p(face_idx[i], face_idx[(i+1)%face_idx.size()]);
                            face_temp -> edge = edge_temp;
                            edge_temp -> left = face_temp;
                            v_Wedges[p] = edge_temp;
                            v0->edge = edge_temp;
                        }

                        // combine old data with new data
                        for(int i = 0; i < face_idx.size(); i++){
                            pair<int, int> p(face_idx[i], face_idx[(i + 1) % face_idx.size()]);
                            pair<int, int> p_prev(face_idx[(i - 1 + face_idx.size()) % face_idx.size()], face_idx[i]);
                            pair<int, int> p_next(face_idx[(i + 1) % face_idx.size()], face_idx[(i + 2) % face_idx.size()]);
                            pair<int, int> p_reverse(face_idx[(i + 1) % face_idx.size()], face_idx[i]);
                            W_edge *edge = v_Wedges.find(p)->second;               // This edge
                            W_edge *edge_prev = v_Wedges.find(p_prev)->second;     // The previous edge
                            W_edge *edge_next = v_Wedges.find(p_next)->second;     // The next edge
                            edge->left_prev = edge_prev;
                            edge->left_next = edge_next;

                            // if the reverse of p exists, we can update right side of p as well as the found p
                            if(v_Wedges.find(p_reverse) != v_Wedges.end()){
                                W_edge *edge2 = v_Wedges.find(p_reverse)->second;
                                edge->right = edge2->left;
                                edge->right_prev = edge2->left_prev;
                                edge->right_next = edge2->left_next;
                                edge2->right = edge->left;
                                edge2->right_prev = edge->left_prev;
                                edge2->right_next = edge->left_next;
                            }
                        }
                    }
                    else
                        continue;
                }

                obj_file.close();
                cout << "successfully saved data to winged-edge data structure" << endl;
            }

            // render model
            cout << "v_vertex length: " <<  v_faces.size() << endl;
            MatrixXf newPositions = MatrixXf(3, v_faces.size()*9);
            // MatrixXf newLines = MatrixXf(3, v_faces.size()*6);
            MatrixXf newColors = MatrixXf(3, v_faces.size()*9);
            MatrixXf newFaceNormals = MatrixXf(3, v_faces.size());
            MatrixXf newVertexNormals = MatrixXf(3, v_faces.size()*9);
            MatrixXf newVertexNormals_flat = MatrixXf(3, v_faces.size()*9);
            vector<Vertex *> v_order;
            int check_count = 3;    // make sure we find 3 vertex every iteration
            int newPosition_col = 0;
            int newFaceNormal_col = 0;
            float max_axis_value = -100000; // record max value among all three axises for normalization
            float total_x = 0;  // record total value for each axises for centeralization
            float total_y = 0;
            float total_z = 0;

            // build new position and color matrix for the use of rendering
            for (auto face : v_faces) {
                W_edge *e0 = face->edge;
                W_edge *edge = e0;
                if(check_count != 3){
                    cout << "unqualified!" << endl;
                }
                check_count = 0;
                MatrixXf temp_face = MatrixXf(3, 3);
                do {
                    temp_face.col(check_count) << v_vertex[vertex_idx_map[edge->start]]->x, v_vertex[vertex_idx_map[edge->start]]->y, v_vertex[vertex_idx_map[edge->start]]->z;
                    // if(newPosition_col == 0)
                        // cout << v_vertex[vertex_idx_map[edge->start]]->x << endl;
                    v_order.push_back(edge->start);
                    check_count++;

                    if(abs(v_vertex[vertex_idx_map[edge->start]]->x) > max_axis_value)
                        max_axis_value = abs(v_vertex[vertex_idx_map[edge->start]]->x);
                    if(abs(v_vertex[vertex_idx_map[edge->start]]->y) > max_axis_value)
                        max_axis_value = abs(v_vertex[vertex_idx_map[edge->start]]->y);
                    if(abs(v_vertex[vertex_idx_map[edge->start]]->z) > max_axis_value)
                        max_axis_value = abs(v_vertex[vertex_idx_map[edge->start]]->z);

                    total_x += v_vertex[vertex_idx_map[edge->start]]->x;
                    total_y += v_vertex[vertex_idx_map[edge->start]]->y;
                    total_z += v_vertex[vertex_idx_map[edge->start]]->z;

                    newPositions.col(newPosition_col) << v_vertex[vertex_idx_map[edge->start]]->x, v_vertex[vertex_idx_map[edge->start]]->y, v_vertex[vertex_idx_map[edge->start]]->z;
                    newColors.col(newPosition_col) << 1, 0, 0;
                    newColors.col(v_faces.size()*3 + newPosition_col*2) << 0, 0, 0;
                    newColors.col(v_faces.size()*3 + newPosition_col*2+1) << 0, 0, 0;

                    // store vertex postion of lines
                    newPositions.col(v_faces.size()*3 + newPosition_col*2) << v_vertex[vertex_idx_map[edge->start]]->x, v_vertex[vertex_idx_map[edge->start]]->y, v_vertex[vertex_idx_map[edge->start]]->z;
                    newPositions.col(v_faces.size()*3 + newPosition_col*2+1) << v_vertex[vertex_idx_map[edge->end]]->x, v_vertex[vertex_idx_map[edge->end]]->y, v_vertex[vertex_idx_map[edge->end]]->z;
                    
                    newPosition_col++;
                    if (edge->left == face)
                        edge = edge->left_next;
                    else
                        edge = edge->right_next;
                } while (edge != e0);
                vector<float> this_face_normal;
                get_normal(temp_face, this_face_normal);
                newFaceNormals.col(newFaceNormal_col) << this_face_normal[0], this_face_normal[1], this_face_normal[2];
                newFaceNormal_col++;
            }
            cout << "newPosition cols: " << newPosition_col << endl;
            
            // centeralize and normalize postion data
            float mean_x = total_x / newPosition_col;
            float mean_y = total_y / newPosition_col;
            float mean_z = total_z / newPosition_col;
            for(int i = 0; i < newPosition_col*3; i++){
                newPositions.col(i)[0] -= mean_x;
                newPositions.col(i)[1] -= mean_y;
                newPositions.col(i)[2] -= mean_z;
            }
            newPositions /= max_axis_value;
            newPositions *= 3;

            // make lines out of original postion a little
            for(int i = newPosition_col; i < newPosition_col*3; i++)
                newPositions.col(i) *= 1.005;

            // build new normal matrix for the use of rendering            
            for(int i = 0; i < v_order.size(); i++){
                W_edge *e0 = v_vertex[vertex_idx_map[v_order[i]]] -> edge;
                W_edge *edge = e0;
                int face_count = 0;
                newVertexNormals.col(i) << 0, 0, 0;
                newVertexNormals_flat.col(i) = newFaceNormals.col(face_idx_map[edge -> left]);
                newVertexNormals_flat.col(v_order.size() + i*2) = newFaceNormals.col(face_idx_map[edge -> left]);
                do{
                    if(edge->end == v_vertex[vertex_idx_map[v_order[i]]]){
                        newVertexNormals.col(i) += newFaceNormals.col(face_idx_map[edge -> left]);
                        edge = edge->left_next;
                        face_count++;
                    }else{
                        newVertexNormals.col(i) += newFaceNormals.col(face_idx_map[edge -> right]);
                        edge = edge->right_next;
                        face_count++;
                    }
                }while (edge != e0);
                newVertexNormals.col(i) /= face_count;
                newVertexNormals.col(v_order.size() + i*2) = newVertexNormals.col(i);

                e0 = v_vertex[vertex_idx_map[v_order[i]]] -> edge -> end -> edge;
                edge = e0;
                face_count = 0;
                newVertexNormals.col(v_order.size() + i*2 + 1) << 0, 0, 0;
                newVertexNormals_flat.col(v_order.size() + i*2 + 1) = newFaceNormals.col(face_idx_map[edge -> left]);
                do{
                    if(edge->end == v_vertex[vertex_idx_map[v_order[i]]] -> edge -> end){
                        newVertexNormals.col(i) += newFaceNormals.col(face_idx_map[edge -> left]);
                        edge = edge->left_next;
                        face_count++;
                    }else{
                        newVertexNormals.col(i) += newFaceNormals.col(face_idx_map[edge -> right]);
                        edge = edge->right_next;
                        face_count++;
                    }
                }while (edge != e0);
                newVertexNormals.col(i) /= face_count;
            }

            mCanvas->updateMeshPositions(newPositions, (int)(newPosition_col/3));
            mCanvas->updateMeshColors(newColors, (int)(newPosition_col/3));
            mCanvas->updateMeshNormals(newVertexNormals, newVertexNormals_flat, (int)(newPosition_col/3));
        });

        b = new Button(tools, "Save");
        b->setCallback([&] {
            string file_dir = file_dialog({{"obj", "3D image file"}}, true);
            std::ofstream fout(file_dir, std::ofstream::out);
            if (fout.is_open()){
                fout << "# " << v_vertex.size() << " " << v_faces.size() << endl;
                for(auto vertex : v_vertex)
                    fout << "v " << vertex->x << " " << vertex->y << " " << vertex->z << endl;
                for(auto face : v_faces){
                    W_edge *e0 = face->edge;
                    W_edge *edge = e0;
                    fout << "f";
                    do {
                        fout << " " << vertex_idx_map[edge->start] + 1;      // The faces data
                        if (edge->left == face)
                            edge = edge->left_next;
                        else
                            edge = edge->right_next;
                    } while (edge != e0);
                    fout << endl;
                }
                fout.close();
            }else
                cout << "error creating obj file" << endl;
            cout << "successfully saved mesh data to obj file" << endl;
        });

        //This is how to implement a combo box, which is important in A1
        new Label(anotherWindow, "Choose render mode", "sans-bold");
        ComboBox *combo = new ComboBox(anotherWindow, {"Smooth shaded", "Flat shaded", "Wireframe", "Smooth&Wireframe", "Flat&Wireframe"});
        combo->setCallback([&](int value) {
            cout << "Choose render mode: " << value << endl;
            mCanvas->updateRenderMode(value);
        });

        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 20));
        b = new Button(tools, "Quit");
        b->setCallback([&] {
            nanogui::shutdown();
        });

        //Method to assemble the interface defined before it is called
        performLayout();
    }

    void get_position(string &s, char delim, vector<float> &res){
        std::stringstream ss(s);
        string str;
        while (getline(ss, str, delim)) {
            if (str.compare("") != 0)
                res.push_back((float)stof(str));
        }
    }

    void get_index(string &s, char delim, vector<int> &res){
        std::stringstream ss(s);
        string str;
        while (getline(ss, str, delim)) {
            if (str.compare("") != 0)
                res.push_back((int)stof(str));
        }
    }

    void get_normal(MatrixXf this_face, vector<float> &res){
        vector<float> v1 = {(this_face.col(1)[0] - this_face.col(0)[0]), (this_face.col(1)[1] - this_face.col(0)[1]), (this_face.col(1)[2] - this_face.col(0)[2])};
        vector<float> v2 = {(this_face.col(2)[0] - this_face.col(0)[0]), (this_face.col(2)[1] - this_face.col(0)[1]), (this_face.col(2)[2] - this_face.col(0)[2])};
        res.push_back((v1[1]*v2[2]-v1[2]*v2[1]));
        res.push_back(-(v1[0]*v2[2]-v1[2]*v2[0]));
        res.push_back((v1[0]*v2[1]-v1[1]*v2[0]));
        // normalize vector to length 1
        float length = sqrt(res[0]*res[0] + res[1]*res[1] + res[2]*res[2]);
        for(int i = 0; i < res.size(); i++)
            res[i] /= length;
        // cout << "normal" << res[0] << endl;
    }

    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    virtual void draw(NVGcontext *ctx)
    {
        /* Animate the scrollbar */
        // mProgress->setValue(std::fmod((float)glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        Screen::draw(ctx);
    }

  private:
    // nanogui::ProgressBar *mProgress;
    MyGLCanvas *mCanvas;
    
    float rotation0_tracking = 0.0;
    float rotation1_tracking = 0.0;
    float rotation2_tracking = 0.0;

    struct Vertex;
    struct W_edge;
    struct Face;

    // Structure W_edge
    struct W_edge {
        Vertex *start, *end;
        Face *left, *right;
        W_edge *left_prev, *right_prev;
        W_edge *left_next, *right_next;
        W_edge(Vertex *v0, Vertex *v1): start(v0), end(v1) {
            left = nullptr;
            right = nullptr;
            left_prev = nullptr;
            left_next = nullptr;
            right_prev = nullptr;
            right_next = nullptr;
        }
    };

    // Structure Vertex
    struct Vertex {
        W_edge *edge;
        float x, y, z;
        Vertex(float x, float y, float z): edge(nullptr), x(x), y(y), z(z) {};
    };

    // Structure Face
    struct Face {
        W_edge *edge;
        Face(): edge(nullptr) {};
    };

    vector<Vertex *> v_vertex;
    vector<Face *> v_faces;
    std::map<pair<int, int>, W_edge *> v_Wedges;
    std::map<Vertex *, int> vertex_idx_map;
    std::map<Face *, int> face_idx_map;

};

int main(int /* argc */, char ** /* argv */)
{
    try
    {
        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    }
    catch (const std::runtime_error &e)
    {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }

    return 0;
}

