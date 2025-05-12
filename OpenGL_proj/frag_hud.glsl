layout(binding = 0) uniform sampler2D tex_scr;

out vec4 color;

void main() {
	vec4 col = texture(tex_scr, tex_coords);

	color = col;
}