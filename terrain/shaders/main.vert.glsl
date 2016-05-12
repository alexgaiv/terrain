#version 110

attribute vec3 Vertex;
attribute vec3 Normal;
attribute vec2 TexCoord;
attribute vec3 Tangent;
attribute vec3 Binormal;

varying vec3 fPosition;
varying vec2 fTexCoord;
varying vec3 fNormal;
varying vec3 fTangent;
varying vec3 fBinormal;

uniform mat4 ModelView;
uniform mat4 NormalMatrix;
uniform mat4 ModelViewProjection;

uniform int Mode;

void main()
{
	fPosition = (ModelView * vec4(Vertex, 1.0)).xyz;
	fTexCoord = TexCoord;

	fNormal   = normalize((NormalMatrix * vec4(Normal, 1.0)).xyz);
	if (Mode == 1) {
		fTangent  = normalize((NormalMatrix * vec4(Tangent, 1.0)).xyz);
		fBinormal = normalize((NormalMatrix * vec4(Binormal, 1.0)).xyz);
	}
	else fTangent = fBinormal = vec3(0.0);

	gl_Position = ModelViewProjection * vec4(Vertex, 1.0);
}