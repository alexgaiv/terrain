#version 110

varying vec2 fTexCoord;
uniform sampler2D tex_grass;

void main()
{
	vec4 texel = texture2D(tex_grass, fTexCoord);
	if (texel.a < 0.5) discard;
	gl_FragColor = texel;
}