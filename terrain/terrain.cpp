#include "terrain.h"
#include "datatypes.h"

Terrain::Terrain(GLRenderingContext *rc)
	: rc(rc),
	vertices(rc, GL_ARRAY_BUFFER),
	indices(rc, GL_ELEMENT_ARRAY_BUFFER),
	normals(rc, GL_ARRAY_BUFFER),
	texCoords(rc, GL_ARRAY_BUFFER)
{
	indicesCount = 0;
}

bool Terrain::LoadHeightmap(const char *filename, float heightScale)
{
	img.LoadTga(filename);
	if (!img) return false;

	this->heightScale = heightScale;
	int w = img.GetWidth();
	int h = img.GetHeight();
	indicesCount = (w-1)*(h-1)*6;

	Vector3f *verts = new Vector3f[w * h];
	Vector3f *norms = new Vector3f[w * h];
	Vector2f *texs = new Vector2f[w * h];
	UINT *inds = new UINT[indicesCount];
	Vector3f *polygonNormals[2];

	for (int i = 0; i < 2; i++)
		polygonNormals[i] = new Vector3f[(w-1)*(h-1)*2];

	float hw = w * 0.5f;
	float hh = h * 0.5f;
	float k = 1.0f / 255.0f * heightScale;

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			int idx = i * h + j;
			verts[idx].x = i - hw;
			verts[idx].z = j - hh;
			verts[idx].y = img.GetPixel(i, j).r * k;

			texs[idx].x = i*0.3f;
			texs[idx].y = j*0.3f;
		}

	int n = 0;
	for (int i = 0; i < w - 1; i++)
		for (int j = 0; j < h - 1; j++)
		{
			inds[n] = i*h + j;
			inds[n+1] = i*h + (j+1);
			inds[n+2] = (i+1)*h + (j+1);

			inds[n+3] = inds[n];
			inds[n+4] = inds[n+2];
			inds[n+5] = (i+1)*h + j;

			Vector3f t1[3] = {
				verts[inds[n]],
				verts[inds[n+1]],
				verts[inds[n+2]]
			};

			Vector3f t2[3] = {
				verts[inds[n+3]],
				verts[inds[n+4]],
				verts[inds[n+5]]
			};

			polygonNormals[0][i*h + j] = Normalize(Cross(t1[0] - t1[1], t1[1] - t1[2]));
			polygonNormals[1][i*h + j] = Normalize(Cross(t2[0] - t2[1], t2[1] - t2[2]));

			n += 6;
		}

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
		{
			Vector3f &normal = norms[i*h + j];

			if (i != 0 && j != 0) {
				for (int k = 0; k < 2; k++)
					normal += polygonNormals[k][(i-1)*h + (j-1)];
			}

			if (i != w - 1 && j != h - 1) {
				for (int k = 0; k < 2; k++)
					normal += polygonNormals[k][i*h + j];
			}

			if (i != 0 && j != h - 1)
				normal += polygonNormals[1][(i-1)*h + j];
			if (j != 0 && i != w - 1)
				normal += polygonNormals[0][i*h + (j-1)];

			normal.Normalize();
		}

	vertices.SetData(w * h * sizeof(Vector3f), verts, GL_STATIC_DRAW);
	normals.SetData(w * h * sizeof(Vector3f), norms, GL_STATIC_DRAW);
	texCoords.SetData(w * h * sizeof(Vector2f), texs, GL_STATIC_DRAW);
	indices.SetData(indicesCount * sizeof(UINT), inds, GL_STATIC_DRAW);

	delete [] polygonNormals[0];
	delete [] polygonNormals[1];

	delete [] verts;
	delete [] norms;
	delete [] texs;
	delete [] inds;
	return true;
}

void Terrain::Draw()
{
	glEnableVertexAttribArray(AttribsLocations.Vertex);
	glEnableVertexAttribArray(AttribsLocations.Normal);

	glEnableVertexAttribArray(AttribsLocations.TexCoord);
	texCoords.AttribPointer(AttribsLocations.TexCoord, 2, GL_FLOAT);

	vertices.AttribPointer(AttribsLocations.Vertex, 3, GL_FLOAT);
	normals.AttribPointer(AttribsLocations.Normal, 3, GL_FLOAT);
	indices.DrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT);
	
	glDisableVertexAttribArray(AttribsLocations.Vertex);
	glDisableVertexAttribArray(AttribsLocations.Normal);
	glDisableVertexAttribArray(AttribsLocations.TexCoord);
}

float Terrain::GetHeightAt(float x, float z) const
{
	return GetPolyEquation(x, z).GetY(x, z);
}

Plane Terrain::GetPolyEquation(float x, float z) const
{
	int w = img.GetWidth(), h = img.GetHeight();
	int i = (int)x + w/2;
	int j = (int)z + h/2;

	if (i < 0 || i >= w - 1 || j < 0 || j >= h - 1) {
		Plane p(0, 0, 0, 0);
		return p;
	}

	float k = 1.0f / 255.0f * heightScale;
	Vector3f v1(
		i - w*0.5f,
		img.GetPixel(i, j).r * k,
		j - h*0.5f
	);

	Vector3f v2(
		i + 1 - w*0.5f,
		img.GetPixel(i + 1, j).r * k,
		j - h*0.5f
	);

	Vector3f v3(
		i - w*0.5f,
		img.GetPixel(i, j + 1).r * k,
		j + 1 - h*0.5f
	);

	return Plane(v1, v2, v3);
}