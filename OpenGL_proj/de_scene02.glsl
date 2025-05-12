const float MAX_OBJ_Y = 5.0;
const int MAX_OBJ_ON = 1;

const float SPH_RAD = 8.0;
const float SCALE = 8.0;
const float inf = 1.0 / 0.0;

float hash(vec3 p) {
	return mod(fract(sin(dot(p, vec3(12.9898, 78.2335, 54.7761))) * 43758.5453), SPH_RAD);
}

vec3 rep(vec3 p, float spacing) {
    vec3 c = vec3(spacing, spacing, spacing);
    vec3 q = mod(p + 0.5 * c, c) - 0.5 * c;

    return q;
}

float sph(vec3 i, vec3 f, vec3 c) {
	float rad = 0.5 * hash(i + c);

	return length(f - vec3(c)) - rad;
}

float sdBox2D(vec2 p, vec2 s) {
    vec2 d = abs(p) - s;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float sdBase(vec3 p) {
	vec3 i = vec3(floor(p));
	vec3 f = fract(p);

	return min(min(min(sph(i, f, vec3(0, 0, 0)),
                      sph(i, f, vec3(0, 0, 1))),
                  min(sph(i, f, vec3(0, 1, 0)),
                      sph(i, f, vec3(0, 1, 1)))),
              min(min(sph(i, f, vec3(1, 0, 0)),
                      sph(i, f, vec3(1, 0, 1))),
                  min(sph(i, f, vec3(1, 1, 0)),
                      sph(i, f, vec3(1, 1, 1)))));
}

float sdBox(vec3 p, vec3 s) {
    p = abs(p) - s;

	return length(max(p, 0.0)) + min(max(p.x, max(p.y, p.z)), 0.0);
}

float sdCross(vec3 p) {
	float da = sdBox2D(p.xy, vec2(1.0));
	float db = sdBox2D(p.yz, vec2(1.0));
	float dc = sdBox2D(p.zx, vec2(1.0));

	return min(da, min(db, dc));
}

float sdTrap(vec3 p) {
	return sdBox(p, vec3(0.5));
}

float sdPlane(vec3 p, float o) {
	return p.y + o;
}

vec4 sdMenger(vec3 p) {
	int iters = 5;
	float d = sdBox(p, vec3(1.0));
	float s = 1.0;
	vec3 min_orb = vec3(inf);

	for(int m = 0; m < iters; m++) {
		vec3 a = mod(p * s, 2.0) - 1.0;
		s *= 3.0;
		vec3 r = 1.0 - 3.0 * abs(a);
		float c = sdCross(r) / s;
		d = max(d, c);

		if(sdTrap(a) < sdTrap(min_orb)) {
			min_orb = a;
		}
	}

	return vec4(min_orb, d);
}

vec4 de(vec3 p) {
	vec3 pos = p * (1.0 / SCALE);
	vec4 ms = sdMenger(rep(pos, 2.0));

	vec3 rv1 = vec3(441.31, -34.74, -906.48);
	vec3 rv2 = vec3(-312.11, 726.72, 34.99);
	vec3 rv3 = vec3(186.45, -651.98, 839.43);

	float b1 = sdBase(rv1 + pos);
	float b2 = 0.5 * sdBase(rv2 + pos);
	float b3 = 0.25 * sdBase(rv3 + pos);
	float base = min(b1, min(b2, b3));

	float pln = sdPlane(pos, 0.25);
	float d = max(ms.w, max(-base, pln));

	ms.x = max(ms.x * 3.0, 0.8);
	ms.y = max(ms.y * 0.5, 0.7);
	ms.z = max(ms.z * 0.5, 0.4);
	ms.xyz /= 4.0;

	float pulse_speed = 1.0 / 2.0;
	float pulse_scale = 0.002;

	d -= (-cos(time * pulse_speed) * 0.5 - 0.5) * pulse_scale;

	return vec4(ms.x, ms.y, ms.z, d / (1.0 / SCALE));
}