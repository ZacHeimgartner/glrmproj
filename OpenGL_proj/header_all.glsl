#version 460

#define MAX_STEPS 225
#define MAX_DIST 512.0
#define SURF_DIST 0.001
#define STEP_SIZE 1.0
#define NORMAL_SAMPLE_SIZE 0.001
#define FOV 1.45

#define TAU 6.283185
#define PI 3.141592

in vec2 tex_coords;
in vec3 camera_pos;
in vec3 camera_fwd;
in vec2 screen_size;
in float time;
in float boost;
in vec3 seed;