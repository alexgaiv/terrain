#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include "glwindow.h"
#include "mesh.h"
#include "terrain.h"
#include "camera.h"
#include "skybox.h"
#include "model.h"

class MainWindow : public GLWindow
{
public:
	MainWindow();
	void Update(int timeElapsed);
private:
	Camera camera;
	Skybox *skybox;
	ProgramObject *mainShader, *terrainShader, *grassShader;
	Model *gun, *muzzle_flash, *crosshair;
	Terrain *terrain;

	int bottomPosLoc;
	int topPosLoc;
	VertexBuffer *grass_pos[2];
	Mesh *grass_mesh;
	int grassInstanceCount;

	struct Target
	{
		Model *model;
		bool fShot;
		bool fAnim;
		float rotAngle;

		Target() : model(NULL), rotAngle(0.0f) { fShot = fAnim = false; }
	};

	static const int numTargets = 20;
	Target *targets;
	
	float heightScale;
	float growth;
	float finalHeight;

	bool fGunAnim;
	float gunAnim;
	bool fShowMuzzleFlash;

	Texture2D *tex_grassobj;
	Texture2D *tex_ground, *tex_grass, *tex_rocks;

	WindowInfoStruct GetWindowInfo()
	{
		WindowInfoStruct wi = { };
		wi.hCursor = NULL;
		wi.hIcon = wi.hIconSmall = LoadIcon(NULL, IDI_APPLICATION);
		wi.lpClassName = "mainwindow";
		return wi;
	}

	void Shot();
	void GenerateGrass(Mesh *mesh);

	void OnCreate();
	void OnDisplay();
	void OnSize(int w, int h);
	void OnKeyDown(UINT keyCode) {
		if (keyCode == 27) DestroyWindow(m_hwnd);
	}
	void OnMouseDown(MouseButton btn, int x, int y);
	void OnMouseMove(UINT keyPressed, int x, int y);
	void OnDestroy();
};

#endif // _MAIN_WINDOW_H_