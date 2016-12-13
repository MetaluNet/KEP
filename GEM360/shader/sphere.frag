//from http://kylehalladay.com/blog/tutorial/math/2013/12/24/Ray-Sphere-Intersection.html

//bool intersect(Ray* r, Sphere* s, float* t1, float *t2)

uniform vec3 origin, focal;

uniform samplerCube /*sampler2D*/ envMap;

void main()
{
	gl_FragColor = 0;
	vec2 sc = gl_TexCoord[0].st;
	
	//solve for tc
	//float L = s->center - r->origin;
	#define L origin
	float tc = dot(L, /*r->*/direction);
	
	if ( tc < 0.0 ) return ;
	float d2 = (tc*tc) - (L*L);
	
	float radius2 = s->radius * s->radius;
	if ( d2 > radius2) return; 
	
	//solve for t1c
	float t1c = sqrt( radius2 - d2 );

	//solve for intersection points
	//*t1 = tc - t1c;
	float t = tc + t1c;
		
}

p1 = r->origin + t2*r->direction;

y0=0.5;
alpha = clamp(gain*(abs(normalize(p1.xy).y)-y0), -1, 1)*0.5+0.5;


