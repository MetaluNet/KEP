//fragment program
// extension pour utiliser des textures rectangulaires, ne pas oublier d'envoyer [rectangle 1< à pix_texture

uniform sampler2D TTex0;


// teinte
uniform vec3 colorTeinte = vec3(1,1,1);
uniform bool teinteOn = false;
uniform float alpha = 1;

// colorToAlpha
uniform vec3 colorRGB = vec3(0,0,0);
uniform float tolbasse = 0;
uniform float tolhaute = 0;
uniform bool colorToAlphaOn = false;

// contrast
uniform float B,C,S = 0;
uniform bool contrastOn = false;


void main()
{
	vec4 macolor = texture2D(TTex0,gl_TexCoord[0].st);
	
	// colorToAlpha
	if (colorToAlphaOn) macolor.a *= clamp((distance(colorRGB, macolor.rgb) - tolbasse) / tolhaute, 0, 1);
	
		
	// contrast
	if (contrastOn){
	const vec3 lumCoeff = vec3(0.2125,  0.7154, 0.0721);
	float l=min(1,5*length(macolor.rgb));
	
	macolor.rgb /= macolor.a;
	macolor.rgb += B; // brightness
	
 	macolor.rgb = (macolor.rgb - 0.5) * C + 0.5 ;

 	vec3 intens = vec3(dot(macolor.rgb,lumCoeff));
 	macolor.rgb = mix(intens, macolor.rgb, S); // saturation
 	
        macolor.rgb *= macolor.a;
        macolor.rgb *= pow(l,0.1);   
	}
	
	// teinte
	if (teinteOn) macolor.rgb *= colorTeinte;
	
	
	
	// alpha general
	 gl_FragColor = vec4(macolor.rgb, macolor.a*alpha);
}
// le macolor permet de rendre les pixels dont la valeur de rouge est > 0.7 transparents en ajoutant un alpha à la chaine
