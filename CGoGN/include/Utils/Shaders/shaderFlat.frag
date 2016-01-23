// ShaderFlat::fragmentShaderText
uniform vec4 diffuse;
uniform vec4 diffuseBack;
uniform vec4 ambient;
uniform bool doubleSided;

VARYING_FRAG float lambertTerm;
FRAG_OUT_DEF;

void main()
{
	if (gl_FrontFacing)
		FRAG_OUT = ambient + lambertTerm*diffuse;
	else
	{
		if (!doubleSided)
			discard;
		FRAG_OUT = ambient - lambertTerm*diffuseBack;
	}

}
