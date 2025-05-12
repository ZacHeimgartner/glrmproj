layout(binding = 0) uniform sampler2D tex_scr;

out vec3 color;

void main() {
    vec3 col = texture(tex_scr, tex_coords).xyz;

    color = col;
}