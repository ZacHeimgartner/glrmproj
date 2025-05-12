const float obj_rad = 0.4;
const float max_space = 45.0;
const float min_space = 17.5;
const float max_offset = 15;

float rand(vec3 p) {
	float res = fract(sin(dot(p, vec3(12.9898, 78.2337, 35.5941))) * 43758.5453);

	return mod(res, 2.0);
}

vec3 repSph(vec3 p, float sx, float sy, float sz) {
    vec3 c = vec3(sx, sy, sz);
    vec3 q = mod(p + 0.5 * c, c) - 0.5 * c;

    return q;
}

float deSphereObj(vec3 p) {
	float d = length(p) - obj_rad;

	return d;
}

float sdObjPlane(vec3 p, float o) {
	return p.y + o;
}

vec4 getObj(vec3 pos) {
	vec3 p = pos;
	vec4 res = vec4(0.0);
	float pln = sdObjPlane(pos, MAX_OBJ_Y);

	p += seed * max_offset;

	res.x = sin(time / 1.21 + 205.3) * 0.5 + 0.5;
	res.y = -sin(time / 0.93 + 619.7) * 0.5 + 0.5;
	res.z = cos(time / 2.17 + 12.9) * 0.5 + 0.5;

	res.xyz = res.xyz * res.xyz * 1.2;

	float hashx = mod(rand(seed.zyx) * max_space, (max_space - min_space + 1.0)) + min_space;
	float hashy = mod(rand(seed.yzx) * max_space, (max_space - min_space + 1.0)) + min_space;
	float hashz = mod(rand(seed.zxy) * max_space, (max_space - min_space + 1.0)) + min_space;

	float sph_dist = deSphereObj(repSph(p, hashx, hashy, hashz));

	if(MAX_OBJ_ON > 0) {
		res.w = max(sph_dist, pln);
	} else {
		res.w = sph_dist;
	}

	return res;
}