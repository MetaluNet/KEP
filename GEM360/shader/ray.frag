/* 
 * Reviewing ray-tracing basics in glsl. Loosely based on Inigo Quilez's articles. 
 * Éric Renaud-Houde - num3ric.com
 * December 2012
 */

#ifdef GL_ES
precision highp float;
#endif

#define EPS 0.0001
#define PI 3.14159265
#define TWO_PI 6.28318530

uniform float angle;
uniform vec2 scale = vec2(1.0,1.0);
uniform samplerCube envMap;
uniform vec3 campos = vec3(0.0, -0.5, 1.0);
uniform vec3 camaim = vec3(0.0, -0.3, -0.9);
uniform float camfov = 1.5;
uniform float blend = 30.0;
uniform float blendAngle=0.5;
uniform float yMin = 0.39;

struct Ray {
    vec3 o; //origin
    vec3 d; //direction (should always be normalized)
};

struct Sphere {
    vec3 pos;   //center of sphere position
    float rad;  //radius
};

/*struct Camera {
    vec3 pos;   //camera position
    vec3 aim;   //view target 
    float fov;  //field of view
};*/

float cosa = cos(angle*(PI/180.0));
float sina = sin(angle*(PI/180.0));
mat3 rotationMatrix = mat3(cosa, 0.0, sina, 0.0, 1.0, 0.0, -sina, 0.0, cosa);
//float focusDistance = 5.0;

//Camera cam = Camera(/*rotationMatrix*vec3(0.0, 0.0, focusDistance)*/campos,
//           /* vec3(0.0, 0.5, 0.0)*/camaim, camfov);
vec3 camdir = normalize(camaim - campos);

Sphere sph = Sphere(vec3(0.0, 0.0, 0.0), 1.0);

// 3 scene points control :
uniform float controlR = 1.0;
vec3 Mt = controlR * vec3(0.0, -1.0, 0.0);			//top
vec3 Ml = controlR * vec3(-sin(PI/3.0), 0.0, -0.5);	//left
vec3 Mr = controlR * vec3(sin(PI/3.0), 0.0, -0.5);	//right
vec3 Mc = controlR * vec3(0.0, 0.0, -1.0);	//center
// in texture coord :
//vec2 texPos(in vec3 M) { return vec2(-cross(cross(M, camdir), M) /camfov);}
//#define texPos(M) vec2(-cross(cross(normalize(M-campos), camdir), normalize(M-campos)) /camfov)
//#define texPos(M) (vec2(normalize(M-campos) - camdir) / camfov)

#define texPos(M) (vec2(cross(M-campos, camdir)/(camfov*(M-campos).z))).yx*vec2(-1.0,1.0)
//#define texPos(M) vec2(-cross(cross(normalize(M-campos), camdir), normalize(M-campos)) /camfov)

vec2 pMt = texPos(Mt);
vec2 pMl = texPos(Ml);
vec2 pMr = texPos(Mr);
vec2 pMc = texPos(Mc);
float r = 0.01;
float r2 = r*r;

// 3 screen points control :
uniform vec2 Pt = vec2(0.0, 0.4);			//top
uniform vec2 Pl = vec2(-0.4, -0.4) ;			//left
uniform vec2 Pr = vec2(0.4, -0.4) ;			//right

mat2 screenToControlMat = (inverse(mat2(Pl-Pt, Pr-Pt)));
mat2 controlToTexMat = (mat2(pMl-pMt, pMr-pMt));

vec2 screenToControl(vec2 v)
{
	return screenToControlMat*(-v-Pt);
}

vec2 controlToTex(vec2 v)
{
	return pMt + controlToTexMat*(v);
}

/* ---------- Object intersection functions ---------- */

float intersectSphere(in Ray ray, in Sphere sphere)
{
    vec3 oc = ray.o - sphere.pos;
    float b = 2.0 * dot(ray.d, oc);
    float c = dot(oc, oc) - sphere.rad*sphere.rad;
    float disc = b * b - 4.0 * c;

    if (disc < 0.0)
        return -1.0;

    // compute q as described above
    float q;
    if (b < 0.0)
        q = (-b - sqrt(disc))/2.0;
    else
        q = (-b + sqrt(disc))/2.0;

    float t0 = q;
    float t1 = c / q;

    // make sure t0 is smaller than t1
    if (t0 > t1) {
        // if t0 is bigger than t1 swap them around
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0.0)
        return -1.0;
	else return t1;

    /*// if t0 is less than zero, the intersection point is at t1
    if (t0 < 0.0) {
        return t1;
    } else {
        return t0; 
    }*/
}

#if 1
//http://kylehalladay.com/blog/tutorial/math/2013/12/24/Ray-Sphere-Intersection.html
float intersectS(Ray r, Sphere s)
{
	//solve for tc
	vec3 L = s.pos - r.o;
	float tc = dot(L, r.d);
	
	//if ( tc < 0.0 ) return -1.0;	
	float d2 = (tc*tc) - dot(L,L);
	
	float radius2 = s.rad * s.rad;
	if ( d2 > radius2) return -1.0;

	//solve for t1c
	float t1c = sqrt( radius2 - d2 );

	return tc + t1c;
	//solve for intersection points
	/**t1 = tc - t1c;
	*t2 = tc + t1c;*/
}
#endif
// http://fhtr.blogspot.fr/2013/12/opus-2-glsl-ray-tracing-tutorial.html
float rayIntersectsSphere(vec3 ray, vec3 dir, vec3 center, float radius, float closestHit)
{
  vec3 rc = ray-center;
  float c = dot(rc, rc) - (radius*radius);
  float b = dot(dir, rc);
  float d = b*b - c;
  float t = -b + sqrt(abs(d));
  /*if (d < 0.0 || t < 0.0 || t > closestHit) {
    return closestHit; // Didn't hit, or wasn't the closest hit.
  } else*/ {
    return d<0.0?0.0:t;
  }
}

void main( void ) {
    vec2 uv = gl_TexCoord[0].st - 0.5;//gl_FragCoord.xy/resolution.xy - 0.5;
    uv*=-1;
    
    uv = controlToTex(screenToControl(uv));
    
    vec3 d = camdir + /*rotationMatrix**/vec3(camfov*uv, 0.0);
    //d.y *= resolution.y/resolution.x;
    //d *= vec3(scale,1.0);
    
    Ray ray = Ray(campos, normalize(d));
    
    //float ts = intersectSphere(ray, sph);
    //float ts = intersectS(ray, sph);
    float ts = rayIntersectsSphere(ray.o, ray.d, sph.pos, sph.rad, 0.0);
    
    if ( ts < 0.0 ) {
    	gl_FragColor = vec4(0.0);
    	return;
    }
    
    vec3 p = ray.o + ray.d * ts - sph.pos;
    
    gl_FragColor = textureCube(envMap, rotationMatrix*p);
    
    
	gl_FragColor.rgb *= clamp(blend*(abs(normalize(p.xz).y)-blendAngle), -1, 1)*0.5+0.5;
	
    if(p.z>0.0) {
    	gl_FragColor = vec4(0.0);
    	//return;
    }
    
    if(p.y>yMin) {
    	gl_FragColor = vec4(0.0);
    	//return;
    }
    
	if( (length(uv - pMt)<r)
		||(length(uv - pMr)<r)
		|| (length(uv - pMc)<r)
	) gl_FragColor=vec4(1.0,0.0,0.0,0.5);
	else if(length(uv - pMl)<r) gl_FragColor=vec4(0.0,1.0,0.0,0.5);

	if( (length(p - Mt)<r)
		||(length(p - Mr)<r)
		|| (length(p - Mc)<r)
	) gl_FragColor=vec4(1.0,0.0,0.0,1.0);
	else if(length(p - Ml)<r) gl_FragColor=vec4(0.0,1.0,0.0,1.0);
    //gl_FragColor = textureCube(envMap, ray.d);
}
