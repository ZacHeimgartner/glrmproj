const float MAX_OBJ_Y = 0.0;
const int MAX_OBJ_ON = 0;

const float inf = 1.0 / 0.0;
const float mbscale = 0.5;
const float spacing = 4.0 / mbscale;
const int iters = 10;


vec3 rep(vec3 p) {
    vec3 c = vec3(spacing, spacing, spacing);
    vec3 q = mod(p + 0.5 * c, c) - 0.5 * c;

    return q;
}

float sdTrap(vec3 pos) {
    vec3 p = pos;
    float d = 0.0;
	
    float s = 3.0;
    p = abs(p)-s;
    d = length(max(p, 0.))+min(max(p.x, max(p.y, p.z)), 0.);

    return abs(d);
}

vec4 sdMB(vec3 r) {
	vec3 zn = r;
	vec3 min_orb = vec3(inf);
	float rad = 0.0;
	float hit = 0.0;
	float p = 6.0 + 2 * sin(time * 0.5);
	float d = 1.0;

	for( int i = 0; i < iters; i++ ) {
			rad = length( zn );

			if( rad > 2.0 ) {	
				hit = 0.5 * log(rad) * rad / d;
			} else {
				float th = atan( length( zn.xy ), zn.z );
				float phi = atan( zn.y, zn.x );		
				float rado = pow(rad,8.0);
				d = pow(rad, 7.0) * 7.0 * d + 1.0;
			
				float sint = sin( th * p );
				zn.x = rado * sint * cos( phi * p );
				zn.y = rado * sint * sin( phi * p );
				zn.z = rado * cos( th * p ) ;
				zn += r;

				if(sdTrap(zn) < sdTrap(min_orb)) {
					min_orb = zn;
				}
			}
			
	}

	min_orb.x = max(min_orb.x * 0.5, 0.9);
	min_orb.y = max(min_orb.y * 0.3, 0.2);
	min_orb.z = max(min_orb.z * 0.7, 0.8);
	min_orb /= 1.5;
	min_orb = max(vec3(0.3), min_orb);
	
	return vec4(min_orb, hit - 0.0015);
}

vec4 sdApo(vec3 pos)
{
	float scale = 1.0;
	float s = 1.5;
	vec3 p = pos;
	float dist = 0.0;
	vec3 min_orb = vec3(inf);
	
	for( int i=0; i<8;i++ )
	{
		p = -1.0 + 2.0*fract(0.5*p+0.5);

		float r2 = dot(p,p);
		
		float k = s/r2;
		p     *= k;
		scale *= k;

		if(sdTrap(p) < sdTrap(min_orb)) {
			min_orb = p;
		}
	}
	
	dist = (0.25*abs(p.y) / scale) - 0.001;

	min_orb.x = max(min_orb.x * 0.2, 0.2);
	min_orb.y = max(min_orb.y * 0.2, 0.1);
	min_orb.z = max(min_orb.z * 1.5, 1.0);
	min_orb /= 2.0;

	float pulse_speed = 1.0 / 2.0;
	float pulse_scale = 0.01;

	dist -= (-cos(time * pulse_speed) * 0.5 + 0.5) * pulse_scale;

	return vec4(min_orb, dist);
}

vec4 de(vec3 pos) {
	float scale = 8.0;
	vec3 p = vec3(pos.x, pos.y - 10.0, pos.z - 5.5) / scale;
	vec4 apo;
	vec4 mb;
	vec4 d;

	apo = sdApo(p);
	mb = sdMB(rep((p + vec3(-1.0, 1.0, -1.0)) / mbscale)) * mbscale;

	if(apo.w <= mb.w) {
		d = apo;
	} else {
		d = mb;
	}

	return vec4(d.x, d.y, d.z, d.w * scale);
}