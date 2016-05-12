#version 110

attribute vec3 Vertex;
attribute vec2 TexCoord;
attribute vec3 Position;
attribute float Offset;

varying vec2 fTexCoord;

uniform vec3 ViewerPos;
uniform mat4 ModelViewProjection;
uniform float anim;

void main()
{
	vec3 vertex = Vertex;
	vertex += Position;

	if (TexCoord.y > 0.5) {
		vertex.x += sin(anim + Offset);
	}

	fTexCoord = TexCoord;
	gl_Position = ModelViewProjection * vec4(vertex, 1.0);
}