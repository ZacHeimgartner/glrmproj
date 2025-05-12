#define ITERS 60
#define POWER 2.0
#define BAILOUT 4.0
#define SCALE 50.0

const float MAX_OBJ_Y = 0.0;
const int MAX_OBJ_ON = 0;

const float spacingy = 85.0;
const float spacingxz = 20;
const float cx = -1.75;
const float cyz = -0.05 + (0.01 * sin(0.5 * time));
const vec3 c = vec3(cx, cyz, cyz).zyx;
const float inf = 1.0 / 0.0;

vec3 rep(vec3 p) {
    vec3 c = vec3(spacingxz, spacingy, spacingxz) * (SCALE / 20);
    vec3 q = mod(p + 0.5 * c, c) - 0.5 * c;

    return q;
}

float sdTrap(vec3 p) {
	float s = 10.0;
	return length(p) - s;
}

vec4 sdBS(vec3 pos) {
	vec3 z = pos;
	float dist = 0.0;
	float dr = 1.0;
	float r = 0.0;
	vec3 min_orb = vec3(inf);

	for(int i = 0; i < ITERS; i++) {
        z = abs(z);
		r = length(z);

		if(r > BAILOUT) {
			break;
		}
		
		float theta = acos(z.z / r);
		float phi = atan(z.y, z.x);
		dr =  pow(r, POWER - 1.0) * POWER * dr + 1.0;
		
		float zr = pow(r, POWER);
		theta = theta * POWER;
		phi = phi * POWER;
		
		z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
		z += c; 
		r = length(z);

		if(sdTrap(z) < sdTrap(min_orb)) {
			min_orb = z;
		}
	}

	float rounding = 0.00015;
	dist = (0.5 * log(r) * r / dr) - rounding;

	return vec4(min_orb, dist);
}

vec4 de(vec3 pos) {
	vec3 offset = vec3(10.0, 0.0, -40.0);
	vec3 p = (rep(pos + offset)) / SCALE;

	vec4 res = sdBS(p.zxy);

	vec3 col = res.xyz;
	float d = res.w * SCALE;

	col.x = min(col.x * 0.0, 0.0);
	col.y = min(col.y * 1.0, 1.0);
	col.z = min(col.z * 0.8, 0.8);
	col = max(col, vec3(0.2));
	col /= 0.85;

	return vec4(col, d);
}