#version 110

attribute vec3 Vertex;
attribute vec3 Normal;
attribute vec2 TexCoord;

varying vec3 fNormal;
varying vec2 fTexCoord;
varying vec3 fPosition;

uniform mat4 ModelView;
uniform mat4 NormalMatrix;
uniform mat4 ModelViewProjection;

void main()
{
	fPosition = Vertex.xyz;
	fNormal = normalize((NormalMatrix * vec4(Normal, 1.0)).xyz);
	fTexCoord = TexCoord;
	gl_Position = ModelViewProjection * vec4(Vertex, 1.0);
}