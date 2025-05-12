#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 in_tex_coords;

uniform vec3 vert_camera_pos;
uniform vec3 vert_camera_fwd;
uniform vec2 vert_screen_size;
uniform float vert_time;
uniform float vert_boost;
uniform vec3 vert_seed;

out vec2 tex_coords;
out vec3 camera_pos;
out vec3 camera_fwd;
out vec2 screen_size;
out float time;
out float boost;
out vec3 seed;

void main() {
    gl_Position = vec4(pos, 1.0);
    tex_coords = in_tex_coords;
    camera_pos = vert_camera_pos;
    camera_fwd = vert_camera_fwd;
    screen_size = vert_screen_size;
    time = vert_time;
    boost = vert_boost;
    seed = vert_seed;
}