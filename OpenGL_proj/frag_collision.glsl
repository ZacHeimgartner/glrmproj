out float color;

vec4 getDist(vec3 p) {
    return de(p);
}

void main() {
    float res = 1.0;
    vec2 uv = gl_FragCoord.xy / vec2(2.0, 1.0);

    if(uv.x < 0.5) {
        res = getDist(camera_pos).w;
    } else {
        res = getObj(camera_pos).w;
    }

    color = res;
}