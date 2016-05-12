#version 110

varying vec3 fPosition;
varying vec3 fNormal;
varying vec2 fTexCoord;

uniform mat4 View;
uniform sampler2D tex_ground, tex_grass, tex_rocks;
uniform float HeightScale;

uniform struct {
	vec3 diffuse;
} FrontMaterial;

struct Light
{
	vec4 position;
	vec3 diffuse;
	vec3 ambient;
};

Light lightSource = Light(
	vec4(1.0, 1.0, 1.0, 0.0),
	vec3(0.8, 0.8, 0.8),
	vec3(0.4, 0.4, 0.4));

const float r1 = 0.24;
const float r2 = 0.37;
const float r3 = 0.52;
const float r4 = 0.67;

vec3 GetColor()
{
	float y = fPosition.y / HeightScale;
	if (y < r1)
		return vec3(texture2D(tex_grass, fTexCoord));

	if (y < r2)
	{
		float k = (y - r1) / (r2 - r1);
		vec4 c1 = texture2D(tex_grass, fTexCoord) * (1.0 - k);
		vec4 c2 = texture2D(tex_ground, fTexCoord) * k;
		return vec3(c1 + c2);
	}

	if (y < r3)
		return vec3(texture2D(tex_ground, fTexCoord));

	if (y < r4)
	{
		float k = (y - r3) / (r3 - r2);
		vec4 c1 = texture2D(tex_ground, fTexCoord) * (1.0 - k);
		vec4 c2 = texture2D(tex_rocks, fTexCoord) * k;
		return vec3(c1 + c2);
	}
	return vec3(texture2D(tex_rocks, fTexCoord));
}

vec3 GetDiffuse(vec3 lightDir, vec3 lightDiffuse) {
	float diffuseCoeff = max(0.0, dot(fNormal, lightDir));
	return diffuseCoeff * lightDiffuse;
}

vec4 PhongLight(Light l)
{
	vec3 lightDir = normalize(vec3(View * l.position));
	return vec4((l.ambient + GetDiffuse(lightDir, l.diffuse)) * GetColor(), 1.0);
}

void main()
{
	gl_FragColor = PhongLight(lightSource);
}