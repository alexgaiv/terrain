#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "image.h"
#include "texture.h"
#include "vertexbuffer.h"
#include "glcontext.h"
#include "geometry.h"

class Terrain
{
public:
	Terrain(GLRenderingContext *rc);

	Vector2i GetSize() const { return Vector2i(img.GetWidth(), img.GetHeight()); };
	bool LoadHeightmap(const char *filename, float heightScale);
	void Draw();

	Plane GetPolyEquation(float x, float z) const;
	float GetHeightAt(float x, float z) const;
private:
	GLRenderingContext *rc;
	
	Image img;
	VertexBuffer vertices;
	VertexBuffer indices;
	VertexBuffer normals;
	VertexBuffer texCoords;

	int indicesCount;
	float heightScale;
};

#endif // _TERRAIN_H_