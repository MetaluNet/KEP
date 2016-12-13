uniform samplerCube /*sampler2D*/ envMap;

void main() {
  
    //vec2 tc = gl_TexCoord[0].st / vec2(2.0) + 0.5;  //only line modified from the shader toy example
    vec2 tc = gl_TexCoord[0].st ;
    vec2 thetaphi = ((tc * 2.0) - vec2(1.0)) * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398);
    float ctpy = cos(thetaphi.y);
    
    //vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));
    vec3 rayDirection = vec3(ctpy * cos(thetaphi.x), sin(thetaphi.y), ctpy * sin(thetaphi.x));
    
    gl_FragColor = textureCube(envMap, rayDirection);
    //gl_FragColor = textureCube(envMap, vec3(tc,1));

    //gl_FragColor = texture2D(envMap,gl_TexCoord[0].st);
}
