#define STB_IMAGE_IMPLEMENTATION
#define GLM_SWIZZLE_XYZ
#define DRAW_WIDTH 1024
#define DRAW_HEIGHT 576

#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "resolution.h"
#include "readshader.h"
#include "input.h"

using namespace std;
using namespace glm;

int main() {
    //Seed RNG
    srand(time(0));

    //Get user input for scene
    string user_in = "";

    cout << "Enter name of scene to open:  " << endl;
    cin >> user_in;
    cout << endl;
    user_in = "de_" + user_in + ".glsl";

    //Get window
    int resx;
    int resy;
    GLFWwindow* window;

    //Vertex array for screen plane
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    unsigned int vao;
    unsigned int vbo;
    unsigned int fbo_col;
    unsigned int fbo_post;
    unsigned int rbo_col;
    unsigned int rbo_post;
    unsigned int tex_post;
    unsigned int tex_hud[3];
    unsigned int tex_color;
    unsigned int vs_all;
    unsigned int fs_rendering;
    unsigned int fs_collision;
    unsigned int fs_post;
    unsigned int fs_hud;
    unsigned int rendering_prog;
    unsigned int collision_prog;
    unsigned int post_prog;
    unsigned int hud_prog;

    int hud_width[3];
    int hud_height[3];
    int hud_channels[3];
    unsigned char* hud_data[3];

    int color_width;
    int color_height;
    int color_channels;
    unsigned char* color_data;

    string cube_faces[6];

    //Game logic variables
    vec3 camera_pos = vec3(0.0f, 0.0f, 0.0f);
    vec3 camera_fwd = vec3(0.0f, 0.0f, -1.0f);
    vec2 screen_size = vec2(float(DRAW_WIDTH), float(DRAW_HEIGHT));
    float col_pixel[2] = { 1.0, 1.0 };
    float col_dist = 0.001;
    float last_dist = 0.0;
    float prox_dist = 0.2;
    float delta_time = 0.0;
    float last_time = 0.0;
    float reset_time = 0.5;
    int hud_state = 0;
    bool reset = false;
    int score = 0;
    float life_time = 0.0;
    int life_num = 1;

    unsigned int r_camera_pos;
    unsigned int r_camera_fwd;
    unsigned int r_screen_size;
    unsigned int r_time;
    unsigned int r_tex;
    unsigned int r_boost;
    unsigned int r_seed;

    unsigned int c_camera_pos;
    unsigned int c_time;
    unsigned int c_seed;

    unsigned int p_tex;

    unsigned int h_tex;

    //Read shader code from GLSL files
    const char* de_file = user_in.c_str();
    string vs_all_str = readShader("vert_all.glsl");
    string fs_rendering_str = readShader("header_all.glsl") + '\n' + readShader(de_file) + '\n' + readShader("header_obj.glsl") + '\n' + readShader("frag_raymarch.glsl");
    string fs_collision_str = readShader("header_all.glsl") + '\n' + readShader(de_file) + '\n' + readShader("header_obj.glsl") + '\n' + readShader("frag_collision.glsl");
    string fs_post_str = readShader("header_all.glsl") + '\n' + readShader("frag_post.glsl");
    string fs_hud_str = readShader("header_all.glsl") + '\n' + readShader(de_file) + '\n' + readShader("frag_hud.glsl");
    const char* vs_all_source = vs_all_str.c_str();
    const char* fs_rendering_source = fs_rendering_str.c_str();
    const char* fs_collision_source = fs_collision_str.c_str();
    const char* fs_post_source = fs_post_str.c_str();
    const char* fs_hud_source = fs_hud_str.c_str();

    //Initialize GLFW and create window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    getScreenResolution(&resx, &resy);
    window = glfwCreateWindow(resx, resy, "OpenGL", glfwGetPrimaryMonitor(), NULL);
    glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, resx, resy, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, resx, resy);

    //Set GLFW input parameters
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Create VAO and VBO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Create collision FBO and RBO
    glGenFramebuffers(1, &fbo_col);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_col);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glGenRenderbuffers(1, &rbo_col);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_col);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RED, 2, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo_col);

    //Create post-process FBO, RBO, and texture
    glGenFramebuffers(1, &fbo_post);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_post);

    glGenRenderbuffers(1, &rbo_post);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_post);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resx, resy);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_post);

    glGenTextures(1, &tex_post);
    glBindTexture(GL_TEXTURE_2D, tex_post);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DRAW_WIDTH, DRAW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_post, 0);

    //Create textures
    for (int i = 0; i < 6; i++) {
        cube_faces[i] = "cube" + to_string(i) + ".png";
    }

    glGenTextures(1, &tex_color);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_color);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; i++) {
        color_data = stbi_load(cube_faces[i].c_str(), &color_width, &color_height, &color_channels, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, color_width, color_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, color_data);
    }

    stbi_image_free(color_data);

    stbi_set_flip_vertically_on_load(true);

    for (int i = 0; i < 3; i++) {
        string filename_str = "hud0" + to_string(i) + ".png";

        glGenTextures(1, &(tex_hud[i]));
        glBindTexture(GL_TEXTURE_2D, tex_hud[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        hud_data[i] = stbi_load(filename_str.c_str(), &hud_width[i], &hud_height[i], &hud_channels[i], 0);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hud_width[i], hud_height[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, hud_data[i]);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(hud_data[i]);
    }

    //Compile shaders and create shader programs
    vs_all = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs_all, 1, &vs_all_source, NULL);
    glCompileShader(vs_all);

    fs_rendering = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_rendering, 1, &fs_rendering_source, NULL);
    glCompileShader(fs_rendering);

    fs_collision = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_collision, 1, &fs_collision_source, NULL);
    glCompileShader(fs_collision);

    fs_post = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_post, 1, &fs_post_source, NULL);
    glCompileShader(fs_post);

    fs_hud = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs_hud, 1, &fs_hud_source, NULL);
    glCompileShader(fs_hud);

    rendering_prog = glCreateProgram();
    glAttachShader(rendering_prog, vs_all);
    glAttachShader(rendering_prog, fs_rendering);
    glLinkProgram(rendering_prog);

    collision_prog = glCreateProgram();
    glAttachShader(collision_prog, vs_all);
    glAttachShader(collision_prog, fs_collision);
    glLinkProgram(collision_prog);

    post_prog = glCreateProgram();
    glAttachShader(post_prog, vs_all);
    glAttachShader(post_prog, fs_post);
    glLinkProgram(post_prog);

    hud_prog = glCreateProgram();
    glAttachShader(hud_prog, vs_all);
    glAttachShader(hud_prog, fs_hud);
    glLinkProgram(hud_prog);

    glDetachShader(rendering_prog, vs_all);
    glDetachShader(rendering_prog, fs_rendering);
    glDetachShader(collision_prog, vs_all);
    glDetachShader(collision_prog, fs_collision);
    glDetachShader(post_prog, vs_all);
    glDetachShader(post_prog, fs_post);
    glDetachShader(hud_prog, vs_all);
    glDetachShader(hud_prog, fs_hud);

    glDeleteShader(vs_all);
    glDeleteShader(fs_rendering);
    glDeleteShader(fs_collision);
    glDeleteShader(fs_post);
    glDeleteShader(fs_hud);

    //Setup attribute arrays and uniform variables for vertex shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    r_camera_pos = glGetUniformLocation(rendering_prog, "vert_camera_pos");
    r_camera_fwd = glGetUniformLocation(rendering_prog, "vert_camera_fwd");
    r_screen_size = glGetUniformLocation(rendering_prog, "vert_screen_size");
    r_time = glGetUniformLocation(rendering_prog, "vert_time");
    r_tex = glGetUniformLocation(rendering_prog, "tex_color");
    r_boost = glGetUniformLocation(rendering_prog, "vert_boost");
    r_seed = glGetUniformLocation(rendering_prog, "vert_seed");

    c_camera_pos = glGetUniformLocation(collision_prog, "vert_camera_pos");
    c_time = glGetUniformLocation(collision_prog, "vert_time");
    c_seed = glGetUniformLocation(collision_prog, "vert_seed");

    p_tex = glGetUniformLocation(post_prog, "tex_scr");

    h_tex = glGetUniformLocation(hud_prog, "tex_scr");

    //Set random seed
    vec3 seed_vec = vec3(0.0);

    seed_vec.x = (rand() % 99) - 50.0;
    seed_vec.y = (rand() % 99) - 50.0;
    seed_vec.z = (rand() % 99) - 50.0;
    
    seed_vec = normalize(seed_vec);

    //Disable depth test
    glDisable(GL_DEPTH_TEST);

    //Main event loop
    while (!glfwWindowShouldClose(window)) {
        //Calculate delta time
        delta_time = glfwGetTime() - last_time;
        last_time = glfwGetTime();
        life_time += delta_time;

        if (reset) {
            delta_time -= reset_time;
        }

        reset = false;

        //Pass game information to shader programs
        glProgramUniform3fv(rendering_prog, r_camera_pos, 1, value_ptr(camera_pos));
        glProgramUniform3fv(rendering_prog, r_camera_fwd, 1, value_ptr(camera_fwd));
        glProgramUniform2fv(rendering_prog, r_screen_size, 1, value_ptr(screen_size));
        glProgramUniform1f(rendering_prog, r_time, float(glfwGetTime()));
        glProgramUniform1i(rendering_prog, r_tex, 0);
        glProgramUniform1f(rendering_prog, r_boost, boost);
        glProgramUniform3fv(rendering_prog, r_seed, 1, value_ptr(seed_vec));

        glProgramUniform3fv(collision_prog, c_camera_pos, 1, value_ptr(camera_pos));
        glProgramUniform1f(collision_prog, c_time, float(glfwGetTime()));
        glProgramUniform3fv(collision_prog, c_seed, 1, value_ptr(seed_vec));

        glProgramUniform1i(post_prog, p_tex, 0);

        glProgramUniform1i(hud_prog, h_tex, 0);

        //Draw collision test image
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_col);
        glViewport(0, 0, 2, 1);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(collision_prog);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //Process user input
        checkInput(window, &camera_pos, &camera_fwd, delta_time);

        //Check collision
        glReadPixels(0, 0, 2, 1, GL_RED, GL_FLOAT, &col_pixel);

        //Render scene image
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_post);
        glViewport(0, 0, DRAW_WIDTH, DRAW_HEIGHT);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_color);

        glUseProgram(rendering_prog);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //Render post-processed image
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, resx, resy);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_post);

        glUseProgram(post_prog);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        //Render HUD
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (col_pixel[0] <= col_dist) {
            hud_state = 2;
        }
        else if (col_pixel[0] <= prox_dist) {
            hud_state = 1;
        }
        else {
            hud_state = 0;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_hud[hud_state]);

        glUseProgram(hud_prog);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisable(GL_BLEND);

        glfwSwapBuffers(window);

        //Resolve collision and calculate score
        if (col_pixel[1] <= col_dist && last_dist > col_dist) {
            score++;
            
            seed_vec.x = (rand() % 99) - 50.0;
            seed_vec.y = (rand() % 99) - 50.0;
            seed_vec.z = (rand() % 99) - 50.0;

            seed_vec = normalize(seed_vec);
        }

        last_dist = col_pixel[1];

        if (col_pixel[0] <= col_dist) {
            cout << "Life: " + to_string(life_num) << endl;
            cout << "Score: " + to_string(score) << endl;
            cout << "Time: " + to_string(life_time) << endl;
            cout << endl;
            score = 0;
            life_time = 0.0;
            life_num++;

            seed_vec.x = (rand() % 99) - 50.0;
            seed_vec.y = (rand() % 99) - 50.0;
            seed_vec.z = (rand() % 99) - 50.0;

            seed_vec = normalize(seed_vec);

            this_thread::sleep_for(chrono::milliseconds(int(reset_time * 1000.0)));
            camera_pos = vec3(0.0f, 0.0f, 0.0f);
            camera_yaw = 0.0;
            camera_pitch = 0.0;
            reset = true;
        }
    }

    //Display score on exit
    cout << "Quit in progress!" << endl;
    cout << "Life: " + to_string(life_num) << endl;
    cout << "Score: " + to_string(score) << endl;
    cout << "Time: " + to_string(life_time) << endl;
    cout << endl;

    //Cleanup and exit
    glDeleteProgram(collision_prog);
    glDeleteProgram(rendering_prog);
    glDeleteProgram(post_prog);
    glDeleteProgram(hud_prog);

    glfwDestroyWindow(window);
    glfwTerminate();

    string exit_buf;
    
    cout << "Type 'q' to quit." << endl;

    while (true) {
        cin >> exit_buf;

        if (exit_buf == "q") {
            break;
        }
    }

    return 0;
}