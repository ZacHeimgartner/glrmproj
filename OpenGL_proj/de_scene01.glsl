const float MAX_OBJ_Y = 5.0;
const int MAX_OBJ_ON = 1;

const float inf = 1.0 / 0.0;
const float scale = 8.0;

float sdTrap(vec3 pos) {
    vec3 p = pos;
    float d = 0.0;
	
    float s = 3.0;
    p = abs(p)-s;
    d = length(max(p, 0.))+min(max(p.x, max(p.y, p.z)), 0.);

    return abs(d);
}

vec3 rep(vec3 p) {
    float spacing = 5.0;
    vec3 c = vec3(spacing, spacing, spacing);
    vec3 q = mod(p + 0.5 * c, c) - 0.5 * c;

    return q;
}

vec4 mbox(vec3 pos) {
    float mScale = 3.0;

    vec3 min_orb = vec3(inf);
	float d = 0.0;
    vec3 p = pos;
  	vec4 q;
    vec4 q0;
  	int nIter = 30;
  	q0 = vec4(p, 1.0);
  	q = q0;
  	for (int n = 0; n < nIter; n++) {
    		q.xyz = clamp(q.xyz, -1., 1.) * 2.0 - q.xyz;
    		q *= mScale / clamp(dot(q.xyz, q.xyz), 0.5, 1.0);
    		q += q0;

            if(sdTrap(q.xyz) < sdTrap(min_orb)) {
                min_orb = q.xyz;
            }
  	}

  	d = length(q.xyz) / abs (q.w);
	d -= 0.005;

    min_orb /= 4.0;
    min_orb.x = max(min_orb.x * 0.9, 0.8);
    min_orb.y = max(min_orb.y * 0.8, 0.3);
    min_orb.z = max(min_orb.z * 0.1, 0.2);
    min_orb /= 8.0;

	return vec4(min_orb, d);
}

vec4 de(vec3 pos) {
    vec3 p = pos / scale;
    vec4 mb = mbox(rep(p));

    float pulse_speed = 1.0 / 3.0;
    float pulse_scale = 0.02;

    mb.w -= (-cos(time * pulse_speed) * 0.5 + 0.5) * pulse_scale;

    return vec4(mb.x, mb.y, mb.z, max(p.y + (2.0 / scale ), mb.w) * scale);
}