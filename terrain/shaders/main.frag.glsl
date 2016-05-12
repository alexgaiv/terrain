#version 110

varying vec3 fPosition;
varying vec3 fNormal;
varying vec2 fTexCoord;
varying vec3 fTangent;
varying vec3 fBinormal;

uniform int Mode;
uniform mat4 View;

uniform sampler2D ColorMap;
uniform sampler2D NormalMap;
uniform sampler2D SpecularMap;

uniform struct {
	int shininess;
} FrontMaterial;

vec3 fragNormal;
mat3 tbn;

struct Light
{
	vec4 position;
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
};

Light lightSource = Light(
	vec4(1.0, 1.0, 1.0, 0.0),
	vec4(0.8, 0.8, 0.8, 1.0),
	vec4(0.8, 0.8, 0.8, 1.0),
	vec4(1.0));

vec4 GetDiffuse(vec3 lightDir, vec4 lightDiffuse) {
	float diffuseCoeff = max(0.0, dot(fragNormal, lightDir));
	return diffuseCoeff * lightDiffuse;
}

vec4 GetSpecular(vec3 lightDir, vec4 lightSpecular)
{
	float specAngle = 0.0;
	vec3 viewDir = normalize(-fPosition);
	vec3 halfDir = normalize(lightDir + viewDir);
	specAngle = max(0.0, dot(fragNormal, halfDir));
	float specCoeff = pow(specAngle, 0.3*FrontMaterial.shininess);
	return specCoeff * lightSpecular;
}

vec4 PhongLight(Light l)
{
	vec3 lightDir = normalize(vec3(View * l.position));
	vec4 texel = texture2D(ColorMap, fTexCoord);
	vec4 color = (l.ambient + GetDiffuse(lightDir, l.diffuse)) * texel;
	if (Mode != 3)
		color += GetSpecular(lightDir, l.specular) * texture2D(SpecularMap, fTexCoord);
	return color;
}

void main()
{
	if (Mode == 1) {
		vec3 t = vec3(texture2D(NormalMap, fTexCoord));
		fragNormal = t * 2.0 - vec3(1.0);
		tbn = mat3(normalize(fTangent), normalize(fBinormal), normalize(fNormal));
		fragNormal = normalize(tbn * fragNormal);
		//fragNormal = normalize(fNormal);
		gl_FragColor = PhongLight(lightSource);
	}
	else if (Mode == 2) {
		gl_FragColor = texture2D(ColorMap, fTexCoord);
	}
	else if (Mode == 3) {
		fragNormal = normalize(fNormal);
		gl_FragColor = PhongLight(lightSource);
	}
}