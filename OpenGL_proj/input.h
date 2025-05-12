#pragma once

#include <iostream>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#define GLM_SWIZZLE_XYZW 
#define SPEED 3.0
#define TURN_RATE 35.0
#define MAX_BOOST 1.7

using namespace std;
using namespace glm;

float camera_pitch = 0.0;
float camera_yaw = 0.0;
float boost = 1.0;

void checkInput(GLFWwindow* window, vec3* camera_pos, vec3* camera_fwd, float delta_time) {
    float step = SPEED * delta_time * boost;
    float turn = TURN_RATE * delta_time / boost;
    vec3 camera_up = vec3(0.0f, 1.0f, 0.0f);

    //Move player forward by SPEED
    *camera_pos += *camera_fwd * step;

    glfwPollEvents();

    //Get new camera forward vector
    camera_fwd->x = cos(radians(camera_yaw)) * cos(radians(camera_pitch));
    camera_fwd->y = sin(radians(camera_pitch));
    camera_fwd->z = sin(radians(camera_yaw)) * cos(radians(camera_pitch));

    //Check keyboard presses
    if (glfwGetKey(window, GLFW_KEY_W)) {
        camera_pitch -= turn;
    }

    if (glfwGetKey(window, GLFW_KEY_S)) {
        camera_pitch += turn;
    }

    if (glfwGetKey(window, GLFW_KEY_A)) {
        camera_yaw += turn;
    }

    if (glfwGetKey(window, GLFW_KEY_D)) {
        camera_yaw -= turn;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        boost = MAX_BOOST;
    }
    else {
        boost = 1.0;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (camera_pitch > 89.0) {
        camera_pitch = 89.0;
    }

    if (camera_pitch < -89.0) {
        camera_pitch = -89.0;
    }
}