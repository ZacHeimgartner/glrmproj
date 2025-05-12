layout(binding = 0) uniform samplerCube tex_color;

const vec3 light_dir = normalize(vec3(1.0, 2.0, 3.0));
const vec3 miss_col = vec3(0.3, 0.3, 0.3);
const float ao_exp = 3.0;
const float ao_k = 2.0;
const float ao_dist = 0.15;
const float far_dist = MAX_DIST;
const float mid_dist = 0.75 * MAX_DIST;

out vec3 color;

vec4 getDist(vec3 p) {
    vec4 res = vec4(0.0);
    vec4 scene = de(p);
    vec4 obj = getObj(p);

    if(scene.w < obj.w) {
        res = scene;
    } else {
        res = obj;
    }

    return res;

    return de(p);
}

mat2 rot(float a) {
    float s = sin(a); 
    float c = cos(a);

    return mat2(c, -s, s, c);
}

vec3 getNormal(vec3 p) {
    vec2 e = vec2(NORMAL_SAMPLE_SIZE, 0.0);
    vec3 n = getDist(p).w - vec3(getDist(p - e.xyy).w, getDist(p - e.yxy).w, getDist(p - e.yyx).w);
    
    return normalize(n);
}

vec4 raymarch(vec3 ro, vec3 rd) {
    float dist = 0.0;
    vec3 col = vec3(1.0, 1.0, 1.0);

    for(int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dist;
        vec4 est = getDist(p);
        dist += est.w * STEP_SIZE; 
        col = est.xyz;

        if(dist >= MAX_DIST || dist <= SURF_DIST) {
            break;
        }
    }

    return vec4(col, dist);
}

vec3 getRayDir(vec2 uv, vec3 p, vec3 l, float z) {
	vec3 f = normalize(l - p);
	vec3 r = normalize(cross(vec3(0.0, 1.0, 0.0), f));
	vec3 u = cross(f, r);
	vec3 c = f * z;
	vec3 i = c + uv.x * r + uv.y * u;

	return normalize(i);
}

float calcAO(vec3 p, vec3 n) {
    float ao_sum = 0.0;

    for(float i = 0.0; i < 5.0; ++i) {
        float coeff = 1.0 / pow(2.0, i);
        ao_sum += coeff * (i * ao_dist - getDist(p + n * i * ao_dist).w);
    }

    return 1.0 - ao_k * ao_sum;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * screen_size) / screen_size.y;
    vec3 ro = camera_pos;
    vec3 rd = getRayDir(uv, ro, ro + camera_fwd, 1.0 / FOV);

    vec3 col = miss_col;
    vec4 rm = raymarch(ro, rd);
    float d = rm.w;
    vec3 p = ro + rd * d;
    vec3 orb_col = rm.xyz;
    vec3 ref_vec = getNormal(p);
    float spin_time = 0.025;

    ref_vec.xz *= rot(time * spin_time);
    ref_vec.xy *= rot(time * spin_time);
    ref_vec.yz *= rot(time * spin_time);

    vec3 ref_col = texture(tex_color, reflect(rd, ref_vec) / 1.0).xyz;
    vec3 ref_des = ref_col;

    ref_des.xyz = vec3((ref_des.x + ref_des.y + ref_des.z) / 3.0);

    ref_col = (ref_col + ref_des) / 2.0;

    float ref_int = 0.7;

    orb_col += ref_int * ref_col;
    orb_col /= ref_int + 1.0;

    if(d < MAX_DIST) {
	    vec3 n = getNormal(p);

	    float dif = dot(n, normalize(light_dir)) * 0.5 + 0.5;
	    col = vec3(dif) * orb_col * min(1.0, pow(calcAO(p, n), ao_exp));
    }

    vec3 fog_far = miss_col + 0.3 * col;
    vec3 fog_mid = max(miss_col, (miss_col + 0.5 * col) / max(1.0, rm.w - mid_dist));
    vec3 fog_close = mix(col, fog_mid, rm.w / mid_dist);

    if(rm.w > far_dist) {
        col = fog_far;
    } else if(rm.w > mid_dist) {
        col = fog_mid;
    } else {
        col = fog_close;
    }

    if(getDist(camera_pos).w <= SURF_DIST * 2.5 + 0.001) {
        col = vec3(0.0);
    }

    if(boost > 1.0) {
        float half_screen = screen_size.y / 0.5;
        float boost_pos = max(0.0, (screen_size.y - gl_FragCoord.y) / half_screen - 0.4);
        boost_pos += max(0.0, (gl_FragCoord.y) / half_screen - 0.4);

        col.x += max(0.0, boost_pos);
        col.y += max(0.0, boost_pos) / 16.0;
    }

    if(getObj(camera_pos).w <= SURF_DIST) {
        col.y += 0.3;
    }

    col = pow(col, vec3(0.4545));

    color = col;
}