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
uniform samplerCube envMap;
uniform vec3 campos = vec3(0.0, -0.3, 1.0);
vec3 camaim = vec3(0.0, -0.3, -0.9);
float camfov = 1.0;
uniform float blend = 30.0;
uniform float blendAngle=0.5;
uniform float yMin = 0.39;

float cosa = cos(angle*(PI/180.0));
float sina = sin(angle*(PI/180.0));
mat3 rotationMatrix = mat3(cosa, 0.0, sina, 0.0, 1.0, 0.0, -sina, 0.0, cosa);

vec3 camdir = normalize(camaim - campos);

vec3 right = -normalize(cross(camdir, vec3(0.0,1.0,0.0)));
vec3 up = normalize(cross(right, camdir));

// 3 scene points control :
vec3 Mt = vec3(0.0, -1.0, 0.0);				//top
vec3 Ml = vec3(-sin(PI/3.0), 0.0, -0.5);	//left
vec3 Mr = vec3(sin(PI/3.0), 0.0, -0.5);		//right

//#define texPos(M) (vec2(cross(M-campos, camdir)/(camfov*(M-campos).z))).yx*vec2(-1.0,1.0)
//vec3 Mp, MpM;
/*#define texPos(M, pM) Mp = camdir*(dot(M-campos, camdir)) ; \
	MpM = (M - Mp)/(length(Mp-campos)); \
	pM= vec2(dot(MpM, right), dot(MpM, up));*/

#define texPos(M, pM, Mp, MpM) vec3 Mp = campos + camdir*(dot(M-campos, camdir)) ; \
	vec3 MpM = (M - Mp)/(length(Mp - campos)); \
	vec2 pM= vec2(dot(MpM, right), dot(MpM, up))

//#define texPos(M, pM) pM = vec2(M)
//#define texPos(M) vec2(0.0,0.0)
/*vec2 _texPos(vec3 x){
	vec3 Mp = camdir*(dot(M-campos, camdir));
	vec3 MpM = 0;(M - Mp)/(length(Mp-campos));
	return vec2(dot(MpM, right), dot(MpM, up));
}*/

/*vec2 pMt = texPos(Mt);
vec2 pMl = texPos(Ml);
vec2 pMr = texPos(Mr);*/
//vec2 pMt, pMl, pMr;
/*vec2 pMl, pMr;
vec2 pMt = _texPos(Mt);*/
//pMt = texPos(Mt);
/*texPos(Mt, pMt);
texPos(Ml, pMl);
texPos(Mr, pMr);*/
texPos(Mt, pMt, Mpt, MpMt);
texPos(Ml, pMl, Mpl, MpMl);
texPos(Mr, pMr, Mpr, MpMr);

// 3 screen points control :
uniform vec2 Pt = vec2(0.0, 0.45);			//top
uniform vec2 Pl = vec2(-0.45, -0.25) ;		//left
uniform vec2 Pr = vec2(0.45, -0.25) ;		//right
uniform float controlR = 0.0; //radius of visible control points, if not 0
uniform int controlSel = 0;//1=top 2=left 3=right
float controlR2 = controlR*controlR;

vec4 colSel = vec4(1.0,1.0,0.0,0.7);
vec4 colNotSel = vec4(0.0,1.0,0.0,0.7);
vec4 colT = controlSel==1?colSel:colNotSel;
vec4 colL = controlSel==2?colSel:colNotSel;
vec4 colR = controlSel==3?colSel:colNotSel;

mat2 screenToControlMat = (inverse(mat2(Pl-Pt, Pr-Pt)));
mat2 controlToTexMat = (mat2(pMl-pMt, pMr-pMt));
mat2 screenToTexMat = controlToTexMat * screenToControlMat;

/*vec2 screenToControl(vec2 v)
{
	return screenToControlMat*(-v-Pt);
}

vec2 controlToTex(vec2 v)
{
	return pMt + controlToTexMat*(v);
}*/

vec2 screenToTex(vec2 v)
{
	return pMt + screenToTexMat*(-v-Pt);
}

//from http://fhtr.blogspot.fr/2013/12/opus-2-glsl-ray-tracing-tutorial.html
float rayIntersectsSphere(vec3 ray, vec3 dir)
{
  vec3 rc = ray;//-center;
  float c = dot(rc, rc) - 1;
  float b = dot(dir, rc);
  float d = b*b - c;
  float t = -b + sqrt(abs(d));
  
  return d<0.0?0.0:t;
}

void main( void ) {
    vec2 uv = gl_TexCoord[0].st - 0.5;
    
    vec2 texuv = screenToTex(uv*-1);
    
    //vec3 d = normalize(camdir + vec3(camfov*texuv, 0.0));
    //vec3 d = normalize(camdir + uv.x*right + uv.y*up);
    vec3 d = normalize(camdir + texuv.x*right + texuv.y*up);
   
    float ts = rayIntersectsSphere(campos, d);
   
    if ( ts < 0.0 ) {
    	gl_FragColor = vec4(0.0);
    	return;
    }
    
    vec3 p = campos + d * ts;
    
    if((p.z>0.0)||(p.y>yMin)) {
    	gl_FragColor = vec4(0.0);
    	//return;
    }
    else {
    	gl_FragColor = textureCube(envMap, rotationMatrix*p);
		gl_FragColor.rgb *= clamp(blend*(abs(normalize(p.xz).y)-blendAngle), -1, 1)*0.5+0.5;
    }
	
    #define near(p) (dot(uv-p, uv-p) < controlR2)
    
    if(controlR>0) {
		if(near(Pt)) gl_FragColor=colT;
		else if(near(Pl)) gl_FragColor=colL;
		else if(near(Pr)) gl_FragColor=colR;
	}
}
