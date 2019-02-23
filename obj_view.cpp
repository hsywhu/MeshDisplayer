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

#define PI 3.14159265

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
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1000, 600), "NanoGUI Cube and Menus", false)
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
            
            if (obj_file.is_open()){
                origin_vertex_x.clear();
                origin_vertex_y.clear();
                origin_vertex_z.clear();
                origin_face_0.clear();
                origin_face_1.clear();
                origin_face_2.clear();
                while(getline(obj_file, line)){
                    trim(line);
                    if(line[0] == '#')
                        continue;
                    else if(line[0] == 'v'){
                        line.erase(0, 2);   //remain only position data
                        vector<float> vertex_p;
                        get_position(line, ' ', vertex_p);
                        origin_vertex_x.push_back(vertex_p[0]);
                        origin_vertex_y.push_back(vertex_p[1]);
                        origin_vertex_z.push_back(vertex_p[2]);
                    }else if(line[0] == 'f'){
                        line.erase(0, 2);
                        vector<int> face_idx;
                        get_index(line, ' ', face_idx);
                        origin_face_0.push_back(face_idx[0] - 1);
                        origin_face_1.push_back(face_idx[1] - 1);
                        origin_face_2.push_back(face_idx[2] - 1);
                    }
                    else
                        continue;
                }
                obj_file.close();
            }
            load_data();
            render_model(true);
            cout << "v_Wedges size: " << v_Wedges.size() << endl;
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

        new Label(anotherWindow, "Choose subdivision level and function", "sans-bold");
        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        TextBox *subLevelBox = new TextBox(tools);
        subLevelBox->setEditable(false);
        subLevelBox->setValue("1");
        Slider *rotSlider = new Slider(tools);
        rotSlider->setRange(std::pair<float, float>(1.0, 5.0));
        rotSlider->setFixedWidth(150);
        rotSlider->setValue(1);
        
        rotSlider->setCallback([&, subLevelBox](float value) {
            subLevelBox->setValue(std::to_string(int(round(value))));
            updateSubLevel(int(round(value)));
        });

        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Loop");
        b->setCallback([&] {
            int total_loop = getSubLevel();
            for(int loop_count = 0; loop_count < total_loop; loop_count++){
                // apply vertex rule
                origin_vertex_x.clear();
                origin_vertex_y.clear();
                origin_vertex_z.clear();
                origin_face_0.clear();
                origin_face_1.clear();
                origin_face_2.clear();
                edge_rule.clear();
                for (auto this_v : v_vertex) {
                    W_edge *e0 = this_v->edge;
                    W_edge *edge = e0;
                    vector<int> idx_vertex;
                    int count = 0;
                    do{
                
                        if (edge->start == this_v)
                            edge = edge -> right_next;
                        else
                            edge = edge -> left_next;
                        if (edge->start == this_v)
                            idx_vertex.push_back(vertex_idx_map[edge->end]);
                        count++;
                    }while(edge != e0);
                    if(count != idx_vertex.size())
                        cout << "error finding surrounding vertex" << endl;
                    if(idx_vertex.size() == 6){
                        float new_x = 0;
                        float new_y = 0;
                        float new_z = 0;
                        for(int idx : idx_vertex){
                            new_x += v_vertex[idx]->x;
                            new_y += v_vertex[idx]->y;
                            new_z += v_vertex[idx]->z;
                        }
                        origin_vertex_x.push_back((this_v->x)*5.0/8.0+(new_x/16.0));
                        origin_vertex_y.push_back((this_v->y)*5.0/8.0+(new_y/16.0));
                        origin_vertex_z.push_back((this_v->z)*5.0/8.0+(new_z/16.0));
                    }else{
                        int K = idx_vertex.size();
                        float beta = ((5.0/8.0)-pow((3.0/8.0)+cos(2.0*PI/K)/4.0, 2.0))/K;
                        float new_x = 0;
                        float new_y = 0;
                        float new_z = 0;
                        for(int idx : idx_vertex){
                            new_x += v_vertex[idx]->x;
                            new_y += v_vertex[idx]->y;
                            new_z += v_vertex[idx]->z;
                        }
                        float center_beta = 1-K*beta;
                        origin_vertex_x.push_back(this_v->x*center_beta+(new_x*beta));
                        origin_vertex_y.push_back(this_v->y*center_beta+(new_y*beta));
                        origin_vertex_z.push_back(this_v->z*center_beta+(new_z*beta));
                    }
                }
                if (origin_vertex_x.size() == v_vertex.size())
                    cout << "successfully applied vertex rule" << endl;
                else
                    cout << "error applying vertex rule" << endl;
                // apply edge rule
                for(auto face : v_faces){
                    W_edge *e0 = face->edge;
                    W_edge *edge = e0;
                    do {
                        if (edge->left == face)
                            edge = edge->left_next;
                        else{
                            edge = edge->right_next;
                        }
                        pair<int, int> p(vertex_idx_map[edge->start], vertex_idx_map[edge->end]);
                        pair<int, int> p_inv(vertex_idx_map[edge->end], vertex_idx_map[edge->start]);
                        std::map<pair<int, int>, int>::iterator edge_rule_it;
                        std::map<pair<int, int>, int>::iterator edge_rule_inv_it;
                        edge_rule_it = edge_rule.find(p);
                        edge_rule_inv_it = edge_rule.find(p_inv);
                        int idx = 0;
                        if(edge_rule_it==edge_rule.end() && edge_rule_inv_it==edge_rule.end()){
                            float new_x = 0;
                            float new_y = 0;
                            float new_z = 0;
                            float tmp = edge->start->x;
                            new_x += (edge->start->x)*3.0/8.0;
                            // cout << "new_x" << new_x << endl;
                            new_y += (edge->start->y)*3.0/8.0;
                            new_z += (edge->start->z)*3.0/8.0;

                            new_x += (edge->end->x)*3.0/8.0;
                            new_y += (edge->end->y)*3.0/8.0;
                            new_z += (edge->end->z)*3.0/8.0;

                            new_x += (edge->right_next->end->x)/8.0;
                            new_y += (edge->right_next->end->y)/8.0;
                            new_z += (edge->right_next->end->z)/8.0;

                            new_x += (edge->left_next->end->x)/8.0;
                            new_y += (edge->left_next->end->y)/8.0;
                            new_z += (edge->left_next->end->z)/8.0;
                            
                            origin_vertex_x.push_back(new_x);
                            origin_vertex_y.push_back(new_y);
                            origin_vertex_z.push_back(new_z);
                            idx = origin_vertex_x.size() - 1;
                            edge_rule[p] = idx;
                            edge_rule[p_inv] = idx;
                        }
                    } while (edge != e0);   
                }
                for(auto face : v_faces){
                    W_edge *e0 = face->edge;
                    W_edge *edge = e0;
                    vector<int> vertex_idx;

                    vertex_idx.push_back(vertex_idx_map[edge->start]);
                    pair<int, int> p0(vertex_idx_map[edge->start], vertex_idx_map[edge->end]);
                    // cout << edge_rule.find(p)->second << endl;
                    vertex_idx.push_back(edge_rule.find(p0)->second);
                    vertex_idx.push_back(vertex_idx_map[edge->end]);
                    pair<int, int> p1(vertex_idx_map[edge->left_next->start], vertex_idx_map[edge->left_next->end]);
                    vertex_idx.push_back(edge_rule.find(p1)->second);
                    vertex_idx.push_back(vertex_idx_map[edge->left_next->end]);
                    pair<int, int> p2(vertex_idx_map[edge->left_next->left_next->start], vertex_idx_map[edge->left_next->left_next->end]);
                    vertex_idx.push_back(edge_rule.find(p2)->second);

                    origin_face_0.push_back(vertex_idx[0]);
                    origin_face_1.push_back(vertex_idx[1]);
                    origin_face_2.push_back(vertex_idx[5]);

                    origin_face_0.push_back(vertex_idx[5]);
                    origin_face_1.push_back(vertex_idx[1]);
                    origin_face_2.push_back(vertex_idx[3]);

                    origin_face_0.push_back(vertex_idx[5]);
                    origin_face_1.push_back(vertex_idx[3]);
                    origin_face_2.push_back(vertex_idx[4]);

                    origin_face_0.push_back(vertex_idx[1]);
                    origin_face_1.push_back(vertex_idx[2]);
                    origin_face_2.push_back(vertex_idx[3]);
                }
                cout << "successfully applied edge rule" << endl;

                // all vertex and face data have been saved, reload them
                load_data();

                render_model();

                // mCanvas->updateMeshPositions(newPositions, (int)(newPosition_col/3));
                // mCanvas->updateMeshColors(newColors, (int)(newPosition_col/3));
                // mCanvas->updateMeshNormals(newVertexNormals, newVertexNormals_flat, (int)(newPosition_col/3));
            }
        });
        
        b = new Button(tools, "Butterfly");
        b->setCallback([&] {
            int total_loop = getSubLevel();
            for(int loop_count = 0; loop_count < total_loop; loop_count++){
                origin_vertex_x.clear();
                origin_vertex_y.clear();
                origin_vertex_z.clear();
                origin_face_0.clear();
                origin_face_1.clear();
                origin_face_2.clear();
                edge_rule.clear();
                // copy the original vertex position informations to new subdivision vectors
                for (auto this_v : v_vertex) {
                    origin_vertex_x.push_back(this_v->x);
                    origin_vertex_y.push_back(this_v->y);
                    origin_vertex_z.push_back(this_v->z);
                }
                // go through all the edges, apply edge rules to compute new vertexs
                for(auto face : v_faces){
                    W_edge *e0 = face->edge;
                    W_edge *edge = e0;
                    do {
                        if (edge->left == face)
                            edge = edge->left_next;
                        else{
                            edge = edge->right_next;
                        }
                        pair<int, int> p(vertex_idx_map[edge->start], vertex_idx_map[edge->end]);
                        pair<int, int> p_inv(vertex_idx_map[edge->end], vertex_idx_map[edge->start]);
                        std::map<pair<int, int>, int>::iterator edge_rule_it;
                        std::map<pair<int, int>, int>::iterator edge_rule_inv_it;
                        edge_rule_it = edge_rule.find(p);
                        edge_rule_inv_it = edge_rule.find(p_inv);
                        int idx = 0;
                        if (edge_rule_it==edge_rule.end() && edge_rule_inv_it==edge_rule.end()){
                            float new_x = 0;
                            float new_y = 0;
                            float new_z = 0;

                            // compute new vertex position
                            // firstly see if both side of the edge are regular degree
                            // starting vertex degree
                            W_edge *e1 = edge;
                            W_edge *edge1 = e1;
                            vector<int> start_surround_idx;
                            do{
                                if (edge1->start == edge->start)
                                    edge1 = edge1 -> right_next;
                                else
                                    edge1 = edge1 -> left_next;
                                start_surround_idx.push_back(vertex_idx_map[edge1->end]);
                            }while(edge1 != e1);

                            // ending vertex degree
                            W_edge *e2 = edge->end->edge;
                            W_edge *edge2 = e2;
                            vector<int> end_surround_idx;
                            do{
                                if (edge2->start == edge->end)
                                    edge2 = edge2 -> right_next;
                                else
                                    edge2 = edge2 -> left_next;
                                end_surround_idx.push_back(vertex_idx_map[edge2->end]);
                            }while(edge2 != e2);

                            // apply different masks according to different cases
                            if (start_surround_idx.size() == 6 && end_surround_idx.size() == 6){
                                // cout << "66" << endl;
                                new_x += (1.0/2.0) * edge->start->x;
                                new_y += (1.0/2.0) * edge->start->y;
                                new_z += (1.0/2.0) * edge->start->z;

                                new_x += (1.0/2.0) * edge->end->x;
                                new_y += (1.0/2.0) * edge->end->y;
                                new_z += (1.0/2.0) * edge->end->z;

                                new_x += (1.0/8.0) * edge->left_next->end->x;
                                new_y += (1.0/8.0) * edge->left_next->end->y;
                                new_z += (1.0/8.0) * edge->left_next->end->z;

                                new_x += (1.0/8.0) * edge->right_next->end->x;
                                new_y += (1.0/8.0) * edge->right_next->end->y;
                                new_z += (1.0/8.0) * edge->right_next->end->z;

                                new_x += -(1.0/16.0) * edge->left_next->right_next->end->x;
                                new_y += -(1.0/16.0) * edge->left_next->right_next->end->y;
                                new_z += -(1.0/16.0) * edge->left_next->right_next->end->z;

                                new_x += -(1.0/16.0) * edge->left_next->left_next->right_next->end->x;
                                new_y += -(1.0/16.0) * edge->left_next->left_next->right_next->end->y;
                                new_z += -(1.0/16.0) * edge->left_next->left_next->right_next->end->z;

                                new_x += -(1.0/16.0) * edge->right_next->right_next->end->x;
                                new_y += -(1.0/16.0) * edge->right_next->right_next->end->y;
                                new_z += -(1.0/16.0) * edge->right_next->right_next->end->z;

                                new_x += -(1.0/16.0) * edge->right_next->left_next->right_next->end->x;
                                new_y += -(1.0/16.0) * edge->right_next->left_next->right_next->end->y;
                                new_z += -(1.0/16.0) * edge->right_next->left_next->right_next->end->z;
                            }else if (start_surround_idx.size() == 6 && end_surround_idx.size() != 6){
                                // cout << "6!6" << endl;
                                if (end_surround_idx.size() == 3){
                                    new_x += (5.0/12.0) * edge->start->x;
                                    new_y += (5.0/12.0) * edge->start->y;
                                    new_z += (5.0/12.0) * edge->start->z;

                                    new_x += (3.0/4.0) * edge->end->x;
                                    new_y += (3.0/4.0) * edge->end->y;
                                    new_z += (3.0/4.0) * edge->end->z;

                                    new_x += -(1.0/12.0) * edge->left_next->end->x;
                                    new_y += -(1.0/12.0) * edge->left_next->end->y;
                                    new_z += -(1.0/12.0) * edge->left_next->end->z;

                                    new_x += -(1.0/12.0) * edge->right_next->end->x;
                                    new_y += -(1.0/12.0) * edge->right_next->end->y;
                                    new_z += -(1.0/12.0) * edge->right_next->end->z;
                                }else if (end_surround_idx.size() == 4){
                                    new_x += (3.0/8.0) * edge->start->x;
                                    new_y += (3.0/8.0) * edge->start->y;
                                    new_z += (3.0/8.0) * edge->start->z;

                                    new_x += (3.0/4.0) * edge->end->x;
                                    new_y += (3.0/4.0) * edge->end->y;
                                    new_z += (3.0/4.0) * edge->end->z;

                                    new_x += -(1.0/8.0) * edge->left_next->right_next->end->x;
                                    new_y += -(1.0/8.0) * edge->left_next->right_next->end->y;
                                    new_z += -(1.0/8.0) * edge->left_next->right_next->end->z;
                                }else if (end_surround_idx.size() > 4){
                                    float center = 1.0;
                                    int starting_idx = 0;
                                    for (int sur_idx = 0; sur_idx < end_surround_idx.size(); sur_idx++){
                                        if (vertex_idx_map[edge->start] == end_surround_idx[sur_idx]){
                                            starting_idx = sur_idx;
                                            break;
                                        }else
                                            continue;
                                    }
                                    for (int sur_idx = 0; sur_idx < end_surround_idx.size(); sur_idx++){
                                        float j = (end_surround_idx.size() + sur_idx - starting_idx) % end_surround_idx.size();
                                        float s = (0.25 + cos(PI * 2.0 * j / end_surround_idx.size()) + 0.5 * cos(4.0 * PI * j / end_surround_idx.size())) / end_surround_idx.size();
                                        center -= s;
                                        new_x += s * v_vertex[end_surround_idx[sur_idx]]->x;
                                        new_y += s * v_vertex[end_surround_idx[sur_idx]]->y;
                                        new_z += s * v_vertex[end_surround_idx[sur_idx]]->z;
                                    }
                                    new_x += center * edge->end->x;
                                    new_y += center * edge->end->y;
                                    new_z += center * edge->end->z;
                                }
                            }else if (start_surround_idx.size() != 6 && end_surround_idx.size() == 6){
                                if (start_surround_idx.size() == 3){
                                    new_x += (5.0/12.0) * edge->end->x;
                                    new_y += (5.0/12.0) * edge->end->y;
                                    new_z += (5.0/12.0) * edge->end->z;

                                    new_x += (3.0/4.0) * edge->start->x;
                                    new_y += (3.0/4.0) * edge->start->y;
                                    new_z += (3.0/4.0) * edge->start->z;

                                    new_x += -(1.0/12.0) * edge->left_next->end->x;
                                    new_y += -(1.0/12.0) * edge->left_next->end->y;
                                    new_z += -(1.0/12.0) * edge->left_next->end->z;

                                    new_x += -(1.0/12.0) * edge->right_next->end->x;
                                    new_y += -(1.0/12.0) * edge->right_next->end->y;
                                    new_z += -(1.0/12.0) * edge->right_next->end->z;
                                } else if (start_surround_idx.size() == 4){
                                    new_x += (3.0/8.0) * edge->end->x;
                                    new_y += (3.0/8.0) * edge->end->y;
                                    new_z += (3.0/8.0) * edge->end->z;

                                    new_x += (3.0/4.0) * edge->start->x;
                                    new_y += (3.0/4.0) * edge->start->y;
                                    new_z += (3.0/4.0) * edge->start->z;

                                    new_x += -(1.0/8.0) * edge->left_next->left_next->right_next->end->x;
                                    new_y += -(1.0/8.0) * edge->left_next->left_next->right_next->end->y;
                                    new_z += -(1.0/8.0) * edge->left_next->left_next->right_next->end->z;
                                } else if (start_surround_idx.size() > 4){
                                    float center = 1.0;
                                    for (int sur_idx = 0; sur_idx < start_surround_idx.size(); sur_idx++){
                                        float j = (sur_idx + 1) % start_surround_idx.size();
                                        float s = (0.25 + cos(PI * 2.0 * j / start_surround_idx.size()) + 0.5 * cos(4.0 * PI * j / start_surround_idx.size())) / start_surround_idx.size();
                                        center -= s;
                                        new_x += s * v_vertex[start_surround_idx[sur_idx]]->x;
                                        new_y += s * v_vertex[start_surround_idx[sur_idx]]->y;
                                        new_z += s * v_vertex[start_surround_idx[sur_idx]]->z;
                                    }
                                    new_x += center * (edge->start->x);
                                    new_y += center * (edge->start->y);
                                    new_z += center * (edge->start->z);
                                }
                            }else{
                                if (end_surround_idx.size() == 3){
                                    new_x += (5.0/12.0) * edge->start->x;
                                    new_y += (5.0/12.0) * edge->start->y;
                                    new_z += (5.0/12.0) * edge->start->z;

                                    new_x += (3.0/4.0) * edge->end->x;
                                    new_y += (3.0/4.0) * edge->end->y;
                                    new_z += (3.0/4.0) * edge->end->z;

                                    new_x += -(1.0/12.0) * edge->left_next->end->x;
                                    new_y += -(1.0/12.0) * edge->left_next->end->y;
                                    new_z += -(1.0/12.0) * edge->left_next->end->z;

                                    new_x += -(1.0/12.0) * edge->right_next->end->x;
                                    new_y += -(1.0/12.0) * edge->right_next->end->y;
                                    new_z += -(1.0/12.0) * edge->right_next->end->z;
                                }else if (end_surround_idx.size() == 4){
                                    new_x += (3.0/8.0) * edge->start->x;
                                    new_y += (3.0/8.0) * edge->start->y;
                                    new_z += (3.0/8.0) * edge->start->z;

                                    new_x += (3.0/4.0) * edge->end->x;
                                    new_y += (3.0/4.0) * edge->end->y;
                                    new_z += (3.0/4.0) * edge->end->z;

                                    new_x += -(1.0/8.0) * edge->left_next->right_next->end->x;
                                    new_y += -(1.0/8.0) * edge->left_next->right_next->end->y;
                                    new_z += -(1.0/8.0) * edge->left_next->right_next->end->z;
                                }else if (end_surround_idx.size() > 4){
                                    float center = 1.0;
                                    int starting_idx = 0;
                                    for (int sur_idx = 0; sur_idx < end_surround_idx.size(); sur_idx++){
                                        if (vertex_idx_map[edge->start] == end_surround_idx[sur_idx]){
                                            starting_idx = sur_idx;
                                            break;
                                        }else{
                                            continue;
                                        }
                                    }
                                    for (int sur_idx = 0; sur_idx < end_surround_idx.size(); sur_idx++){
                                        float j = (end_surround_idx.size() + sur_idx - starting_idx) % end_surround_idx.size();
                                        float s = (0.25 + cos(PI * 2.0 * j / end_surround_idx.size()) + 0.5 * cos(4.0 * PI * j / end_surround_idx.size())) / end_surround_idx.size();
                                        center -= s;
                                        new_x += s * v_vertex[end_surround_idx[sur_idx]]->x;
                                        new_y += s * v_vertex[end_surround_idx[sur_idx]]->y;
                                        new_z += s * v_vertex[end_surround_idx[sur_idx]]->z;
                                    }
                                    new_x += center * edge->end->x;
                                    new_y += center * edge->end->y;
                                    new_z += center * edge->end->z;
                                }
                                if (start_surround_idx.size() == 3){
                                    new_x += (5.0/12.0) * edge->end->x;
                                    new_y += (5.0/12.0) * edge->end->y;
                                    new_z += (5.0/12.0) * edge->end->z;

                                    new_x += (3.0/4.0) * edge->start->x;
                                    new_y += (3.0/4.0) * edge->start->y;
                                    new_z += (3.0/4.0) * edge->start->z;

                                    new_x += -(1.0/12.0) * edge->left_next->end->x;
                                    new_y += -(1.0/12.0) * edge->left_next->end->y;
                                    new_z += -(1.0/12.0) * edge->left_next->end->z;

                                    new_x += -(1.0/12.0) * edge->right_next->end->x;
                                    new_y += -(1.0/12.0) * edge->right_next->end->y;
                                    new_z += -(1.0/12.0) * edge->right_next->end->z;
                                } else if (start_surround_idx.size() == 4){
                                    new_x += (3.0/8.0) * edge->end->x;
                                    new_y += (3.0/8.0) * edge->end->y;
                                    new_z += (3.0/8.0) * edge->end->z;

                                    new_x += (3.0/4.0) * edge->start->x;
                                    new_y += (3.0/4.0) * edge->start->y;
                                    new_z += (3.0/4.0) * edge->start->z;

                                    new_x += -(1.0/8.0) * edge->left_next->left_next->right_next->end->x;
                                    new_y += -(1.0/8.0) * edge->left_next->left_next->right_next->end->y;
                                    new_z += -(1.0/8.0) * edge->left_next->left_next->right_next->end->z;
                                } else if (start_surround_idx.size() > 4){
                                    float center = 1.0;
                                    int starting_idx = 0;
                                    for (int sur_idx = 0; sur_idx < start_surround_idx.size(); sur_idx++){
                                        float j = (sur_idx + 1) % start_surround_idx.size();
                                        float s = (0.25 + cos(PI * 2.0 * j / start_surround_idx.size()) + 0.5 * cos(4.0 * PI * j / start_surround_idx.size())) / start_surround_idx.size();
                                        center -= s;
                                        new_x += s * v_vertex[start_surround_idx[sur_idx]]->x;
                                        new_y += s * v_vertex[start_surround_idx[sur_idx]]->y;
                                        new_z += s * v_vertex[start_surround_idx[sur_idx]]->z;
                                    }
                                    new_x += center * edge->start->x;
                                    new_y += center * edge->start->y;
                                    new_z += center * edge->start->z;
                                }
                                new_x /= 2.0;
                                new_y /= 2.0;
                                new_z /= 2.0;
                            }
                            origin_vertex_x.push_back(new_x);
                            origin_vertex_y.push_back(new_y);
                            origin_vertex_z.push_back(new_z);
                            idx = origin_vertex_x.size() - 1;
                            edge_rule[p] = idx;
                            edge_rule[p_inv] = idx;
                        }
                    } while (edge != e0);   
                }
                for(auto face : v_faces){
                    W_edge *e0 = face->edge;
                    W_edge *edge = e0;
                    vector<int> vertex_idx;

                    vertex_idx.push_back(vertex_idx_map[edge->start]);
                    pair<int, int> p0(vertex_idx_map[edge->start], vertex_idx_map[edge->end]);
                    // cout << edge_rule.find(p)->second << endl;
                    vertex_idx.push_back(edge_rule.find(p0)->second);
                    vertex_idx.push_back(vertex_idx_map[edge->end]);
                    pair<int, int> p1(vertex_idx_map[edge->left_next->start], vertex_idx_map[edge->left_next->end]);
                    vertex_idx.push_back(edge_rule.find(p1)->second);
                    vertex_idx.push_back(vertex_idx_map[edge->left_next->end]);
                    pair<int, int> p2(vertex_idx_map[edge->left_next->left_next->start], vertex_idx_map[edge->left_next->left_next->end]);
                    vertex_idx.push_back(edge_rule.find(p2)->second);

                    origin_face_0.push_back(vertex_idx[0]);
                    origin_face_1.push_back(vertex_idx[1]);
                    origin_face_2.push_back(vertex_idx[5]);

                    origin_face_0.push_back(vertex_idx[5]);
                    origin_face_1.push_back(vertex_idx[1]);
                    origin_face_2.push_back(vertex_idx[3]);

                    origin_face_0.push_back(vertex_idx[5]);
                    origin_face_1.push_back(vertex_idx[3]);
                    origin_face_2.push_back(vertex_idx[4]);

                    origin_face_0.push_back(vertex_idx[1]);
                    origin_face_1.push_back(vertex_idx[2]);
                    origin_face_2.push_back(vertex_idx[3]);
                }
                cout << "successfully applied edge rule" << endl;

                // all vertex and face data have been saved, reload them
                load_data();

                // render model
                render_model();
            }
        });

        new Label(anotherWindow, "Select K and number of edges to collapse", "sans-bold");
        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 10));
        TextBox *selectK = new TextBox(tools, "value of K");
        selectK->setEditable(true);
        selectK->setValue("20");
        new Label(tools, "K", "sans-bold");

        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 10));
        TextBox *numOfEdge = new TextBox(tools, "edge number");
        numOfEdge->setEditable(true);
        numOfEdge->setValue("10000");
        new Label(tools, "Num of edges", "sans-bold");
        
        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 5));
        Button *decimate = new Button(tools, "Decimate");
        decimate->setCallback([&, selectK, numOfEdge] {
            int K = stoi(selectK->value());
            int edgeToCollapse = stoi(numOfEdge->value());
            int numOfEdges = v_Wedges.size();
            cout << numOfEdges << endl;
            for(int collapseCount = 0; collapseCount < edgeToCollapse; collapseCount++){
                
            }
        });
        
        Button *quit_button = new Button(anotherWindow, "Quit");
        quit_button->setCallback([&] {
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
    }

    void get_Q(MatrixXf this_face, Eigen::Matrix4f &this_Q){
        float a1, a2, a3, b1, b2, b3, c1, c2, c3;
		a1 = this_face.col(1)[0] - this_face.col(0)[0];
		a2 = this_face.col(1)[1] - this_face.col(0)[1];
		a3 = this_face.col(1)[2] - this_face.col(0)[2];

		b1 = this_face.col(2)[0] - this_face.col(0)[0];
		b2 = this_face.col(2)[1] - this_face.col(0)[1];
		b3 = this_face.col(2)[2] - this_face.col(0)[2];

		c1 = a2*b3 - a3*b2;
		c2 = a3*b1 - a1*b3;
		c3 = a1*b2 - a2*b1;

        float area = 0.5*(c1*c1+c2*c2+c3*c3);
        float d = -c1 * this_face.col(0)[0] - c2 * this_face.col(0)[1] - c3 * this_face.col(0)[2];
        float aa = area * c1 * c1;
        float ab = area * c1 * c2;
        float ac = area * c1 * c3;
        float ad = area * c1 * d;
        float bb = area * c2 * c2;
        float bc = area * c2 * c3;
        float bd = area * c2 * d;
        float cc = area * c3 * c3;
        float cd = area * c3 * d;
        float dd = area * d * d;
        this_Q.col(0) << aa, ab, ac, ad;
        this_Q.col(1) << ab, bb, bc, bd;
        this_Q.col(2) << ac, bc, cc, cd;
        this_Q.col(3) << ad, bd, cd, dd;
    }

    void updateSubLevel(int newLevel){
        subLevel = newLevel;
    }

    int getSubLevel(){
        return subLevel;
    }

    void load_data(){       // load vertex and face data to winged edge data structure
        v_vertex.clear();
        v_faces.clear();
        v_Wedges.clear();
        vertex_idx_map.clear();
        face_idx_map.clear();
        
        for (int origin_vertex_idx = 0; origin_vertex_idx<origin_vertex_x.size(); origin_vertex_idx++){
            vector<float> vertex_p;
            vertex_p.push_back(origin_vertex_x[origin_vertex_idx]);
            vertex_p.push_back(origin_vertex_y[origin_vertex_idx]);
            vertex_p.push_back(origin_vertex_z[origin_vertex_idx]);
            //start loading vertex data to vector
            Vertex *vertex_temp = new Vertex(vertex_p[0], vertex_p[1], vertex_p[2]);
            v_vertex.push_back(vertex_temp);
            vertex_idx_map[vertex_temp] = v_vertex.size() - 1;
        }
        for (int origin_face_idx = 0; origin_face_idx<origin_face_0.size(); origin_face_idx++){
            vector<int> face_idx;
            face_idx.push_back(origin_face_0[origin_face_idx]+1);
            face_idx.push_back(origin_face_1[origin_face_idx]+1);
            face_idx.push_back(origin_face_2[origin_face_idx]+1);
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
        
        cout << "successfully reloaded winged edge data structure" << endl;
    }

    void render_model(bool is_initializeQ = false){
        // render model
        cout << "v_vertex length: " <<  v_faces.size() << endl;
        MatrixXf newPositions = MatrixXf(3, v_faces.size()*9);
        MatrixXf newColors = MatrixXf(3, v_faces.size()*9);
        MatrixXf newFaceNormals = MatrixXf(3, v_faces.size());
        MatrixXf newVertexNormals = MatrixXf(3, v_faces.size()*9);
        MatrixXf newVertexNormals_flat = MatrixXf(3, v_faces.size()*9);
        vector<Eigen::Matrix4f> newFace;
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
            vector<int> vertex_in_face;     // save index of vertexes in the face
            do {
                vertex_in_face.push_back(vertex_idx_map[edge->start]);
                temp_face.col(check_count) << v_vertex[vertex_idx_map[edge->start]]->x, v_vertex[vertex_idx_map[edge->start]]->y, v_vertex[vertex_idx_map[edge->start]]->z;
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

            if(is_initializeQ){
                // compute matrix Q of the face
                Eigen::Matrix4f this_Q;
                get_Q(temp_face, this_Q);
                // add the Q of the face to the Q of vertexes belonging to the face
                v_vertex[vertex_in_face[0]]->Q += this_Q;
                v_vertex[vertex_in_face[1]]->Q += this_Q;
                v_vertex[vertex_in_face[2]]->Q += this_Q;
            }
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
            newVertexNormals_flat.col(v_order.size() + i*2) = newFaceNormals.col(face_idx_map[edge -> left]) * 1.05;
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
            newVertexNormals.col(v_order.size() + i*2) = newVertexNormals.col(i) * 1.05;

            e0 = v_vertex[vertex_idx_map[v_order[i]]] -> edge -> end -> edge;
            edge = e0;
            face_count = 0;
            newVertexNormals.col(v_order.size() + i*2 + 1) << 0, 0, 0;
            newVertexNormals_flat.col(v_order.size() + i*2 + 1) = newFaceNormals.col(face_idx_map[edge -> left]) * 1.05;
            do{
                if(edge->end == v_vertex[vertex_idx_map[v_order[i]]] -> edge -> end){
                    newVertexNormals.col(v_order.size() + i*2 + 1) += newFaceNormals.col(face_idx_map[edge -> left]);
                    edge = edge->left_next;
                    face_count++;
                }else{
                    newVertexNormals.col(v_order.size() + i*2 + 1) += newFaceNormals.col(face_idx_map[edge -> right]);
                    edge = edge->right_next;
                    face_count++;
                }
            }while (edge != e0);
            newVertexNormals.col(v_order.size() + i*2 + 1) /= face_count * 1.05;
        }
        mCanvas->updateMeshPositions(newPositions, (int)(newPosition_col/3));
        mCanvas->updateMeshColors(newColors, (int)(newPosition_col/3));
        mCanvas->updateMeshNormals(newVertexNormals, newVertexNormals_flat, (int)(newPosition_col/3));
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
        Eigen::Matrix4f Q;
        Vertex(float x, float y, float z): 
            edge(nullptr), 
            x(x), 
            y(y), 
            z(z){
                Q << 0, 0, 0, 0,
                     0, 0, 0, 0, 
                     0, 0, 0, 0, 
                     0, 0, 0, 0;
            };
    };

    // Structure Face
    struct Face {
        W_edge *edge;
        Face(): 
            edge(nullptr) {};
    };

    vector<Vertex *> v_vertex;
    vector<Face *> v_faces;
    std::map<pair<int, int>, W_edge *> v_Wedges;
    std::map<Vertex *, int> vertex_idx_map;
    std::map<Face *, int> face_idx_map;
    std::map<pair<int, int>, int> edge_rule;
    vector<float> origin_vertex_x;
    vector<float> origin_vertex_y;
    vector<float> origin_vertex_z;
    vector<int> origin_face_0;
    vector<int> origin_face_1;
    vector<int> origin_face_2;
    int subLevel = 1;
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
