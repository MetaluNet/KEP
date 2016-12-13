uniform samplerCube cubemap;
in vec3 TexCoords;
void main()
{
   gl_FragColor = texture(cubemap, gl_TexCoord[0]);
   // gl_FragColor = texture(cubemap, TexCoords);
}

