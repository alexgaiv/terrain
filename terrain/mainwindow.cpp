#include "mainwindow.h"
#include "transform.h"
#include <strsafe.h>
#include <time.h>

float randf(float min, float max) {
	return min + (float)rand() / RAND_MAX * (max - min);
}

float randf(float max) {
	return (float)rand() / RAND_MAX * max;
}

MainWindow::MainWindow()
{
	srand((int)time(NULL));
	heightScale = 15.0f;
	growth = 5;
	finalHeight = 0.0f;
	
	fGunAnim = false;
	gunAnim = 0.0f;
	fShowMuzzleFlash = false;

	this->Create("Terrain", CW_USEDEFAULT, CW_USEDEFAULT, 1000, 700);
	//this->CreateFullScreen("Terrain");
}

void MainWindow::GenerateGrass(Mesh *mesh)
{
	if (!mesh->LoadObj("models/grass.obj"))
		return;

	grass_pos[0] = new VertexBuffer(m_rc, GL_ARRAY_BUFFER);
	grass_pos[1] = new VertexBuffer(m_rc, GL_ARRAY_BUFFER);

	bottomPosLoc = grassShader->GetAttribLocation("Position");
	topPosLoc = grassShader->GetAttribLocation("Offset");

	vector<Vector3f> positions;
	vector<float> offsets;

	const float step = 2.0f;
	Vector2i s = terrain->GetSize();
	float hw = s.x * 0.5f;
	float hh = s.y * 0.5f;

	grassInstanceCount = 0;
	for (float x = -hw; x < hw; x += step)
		for (float z = -hh + step; z < hh; z += step)
		{
			float height = terrain->GetHeightAt(x, z);
			if (height < 0.24 * heightScale)
			{
				Vector3f shift_pos(
					randf(-0.5f, 0.5f),
					randf(-0.5f, 0.5f),
					randf(-0.5f, 0.5f));

				Vector3f grassPos = Vector3f(x, height, z);
				positions.push_back(grassPos + shift_pos);
				offsets.push_back(randf(M_PIf));

				grassInstanceCount++;
			}
		}

	grass_pos[0]->SetData(positions.size() * sizeof(Vector3f), positions.data(), GL_STATIC_DRAW);
	grass_pos[1]->SetData(offsets.size() * sizeof(float), offsets.data(), GL_STATIC_DRAW);
}

void MainWindow::Shot()
{
	if (fGunAnim) return;
	PlaySound("gun.wav", NULL, SND_ASYNC|SND_FILENAME|SND_NODEFAULT);
	fGunAnim = true;
	fShowMuzzleFlash = true;

	Matrix44f camView = camera.GetViewMatrix();
	for (int i = 0; i < numTargets; i++) 
	{
		if (targets[i].fShot) continue;

		Model *m = targets[i].model;
		Matrix44f mat = (camView * m->GetTransformRef()).GetInverse();

		Point3f p1 = Point3f(Point4f(0, 0, -3, 1) * mat);
		Point3f p2 = Point3f(Point4f(0, 0, -4, 1) * mat);
		Ray ray(p1, p2 - p1);

		if (m->meshes[1]->boundingBox.Intersects(ray) &&
			m->meshes[1]->boundingSphere.Intersects(ray)) {
			targets[i].fShot = targets[i].fAnim = true;
			break;
		}
	}
}

void MainWindow::OnCreate()
{
	glewInit();

	if (GL_ARB_vertex_array_object) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_COLOR_MATERIAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.82f, 0.85f, 0.96f, 1.0f);

	const char *sides[6] = 
	{
		"textures/skybox/ft.tga",
		"textures/skybox/bk.tga",
		"textures/skybox/up.tga",
		"textures/skybox/dn.tga",
		"textures/skybox/lf.tga",
		"textures/skybox/rt.tga",
	};

	skybox = new Skybox(m_rc, sides);
	mainShader = new ProgramObject(m_rc, "shaders/main.vert.glsl", "shaders/main.frag.glsl");
	terrainShader = new ProgramObject(m_rc, "shaders/terrain.vert.glsl", "shaders/terrain.frag.glsl");
	grass_mesh = new Mesh(m_rc);
	gun = new Model(m_rc);
	muzzle_flash = new Model(m_rc);
	crosshair = new Model(m_rc);
	terrain = new Terrain(m_rc);

	grassShader = new ProgramObject(m_rc);
	grassShader->BindAttribLocation(5, "TopPos");
	grassShader->BindAttribLocation(6, "BottomPos");
	grassShader->AttachShader(Shader(GL_VERTEX_SHADER, "shaders/grass.vert.glsl"));
	grassShader->AttachShader(Shader(GL_FRAGMENT_SHADER, "shaders/grass.frag.glsl"));
	grassShader->Link();

	tex_grassobj = new Texture2D("textures/grassobj.tga");
	tex_ground = new Texture2D("textures/ground.tga", GL_TEXTURE0);
	tex_grass = new Texture2D("textures/grass.tga", GL_TEXTURE1);
	tex_rocks = new Texture2D("textures/rocks.tga", GL_TEXTURE2);

	tex_grassobj->SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	tex_grassobj->SetWrapMode(GL_CLAMP, GL_CLAMP);
	tex_ground->SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	tex_grass->SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	tex_rocks->SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	tex_grassobj->BuildMipmaps();
	tex_ground->BuildMipmaps();
	tex_grass->BuildMipmaps();
	tex_rocks->BuildMipmaps();
	
	terrain->LoadHeightmap("textures/hm.tga", heightScale);

	GenerateGrass(grass_mesh);
	grass_mesh->BindTexture(*tex_grassobj);

	Texture2D gun_diffuse("textures/gun/diffuse.tga", GL_TEXTURE0),
			  gun_normal("textures/gun/normal.tga", GL_TEXTURE1),
			  gun_specular("textures/gun/specular.tga", GL_TEXTURE2);
	gun_diffuse.SetFilters(GL_LINEAR, GL_LINEAR);
	gun_normal.SetFilters(GL_LINEAR, GL_LINEAR);
	gun_specular.SetFilters(GL_LINEAR, GL_LINEAR);

	gun->LoadObj("models/gun.obj");
	gun->meshes[0]->BindTexture(gun_diffuse);
	gun->meshes[0]->BindNormalMap(gun_normal);
	gun->meshes[0]->BindSpecularMap(gun_specular);
	gun->BindShader(*mainShader);
	gun->location = Vector3f(0.7f, -2.0f, -2.0f);
	gun->scale = Vector3f(-1, 1, -1);

	Texture2D tex_muzzle("textures/muzzle.tga");
	tex_muzzle.SetFilters(GL_LINEAR, GL_LINEAR);
	muzzle_flash->LoadObj("models/quad.obj");
	muzzle_flash->meshes[0]->BindTexture(tex_muzzle);
	muzzle_flash->BindShader(*mainShader);
	muzzle_flash->location = Vector3f(0.72f, -0.26f, -3.5f);

	Texture2D tex_ie("textures/ie.tga");
	tex_ie.SetFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	tex_ie.BuildMipmaps();

	Model target(m_rc);
	target.LoadObj("models/target.obj");
	target.meshes[0]->BindTexture(tex_ie);
	target.meshes[1]->BindTexture(tex_ie);
	target.BindShader(*mainShader);

	targets = new Target[numTargets];
	Vector2i s = terrain->GetSize();
	for (int i = 0; i < numTargets; i++)
	{
		targets[i].model = new Model(target);
		float x = (float)(rand() % (s.x - 1) - s.x/2 + 1);
		float z = (float)(rand() % (s.y - 1) - s.y/2 + 1);
		float a = (float)(rand() % 360);
		targets[i].model->location = Vector3f(x, terrain->GetHeightAt(x, z), z);
		targets[i].model->rotation = Quaternion(Vector3f(0,1,0), a);
	}

	Texture2D tex_corsshair("textures/crosshair.tga");
	tex_corsshair.SetFilters(GL_LINEAR, GL_LINEAR);
	crosshair->AddMesh(*muzzle_flash->meshes[0]);
	crosshair->meshes[0]->BindTexture(tex_corsshair);
	crosshair->BindShader(*mainShader);
	crosshair->scale = Vector3f(4.0f);

	mainShader->Uniform("ColorMap", 0);
	mainShader->Uniform("NormalMap", 1);
	mainShader->Uniform("SpecularMap", 2);
	mainShader->Uniform("FrontMaterial.shininess", 100);

	terrainShader->Uniform("FrontMaterial.diffuse", 0.5f, 0.5f, 0.5f);
	terrainShader->Uniform("HeightScale", heightScale);
	terrainShader->Uniform("tex_ground", 0);
	terrainShader->Uniform("tex_grass", 1);
	terrainShader->Uniform("tex_rocks", 2);

	grassShader->Uniform("tex_grass", 0);
	camera.SetPosition(0, growth + terrain->GetHeightAt(0, 0), 0);
}

void MainWindow::OnDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_rc->PushModelView();
		mainShader->Uniform("Mode", 1);
		gun->Draw();

		camera.ApplyTransform(m_rc);
		skybox->Draw();		

		Matrix44f view = m_rc->GetModelView();
		mainShader->UniformMatrix("View", 1, false, view.data);

		mainShader->Uniform("Mode", 3);
		for (int i = 0; i < numTargets; i++)
			targets[i].model->Draw();

		terrainShader->Use();
		terrainShader->UniformMatrix("View", 1, false, view.data);
		tex_ground->Bind();
		tex_grass->Bind();
		tex_rocks->Bind();
		terrain->Draw();

		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);

		glEnableVertexAttribArray(bottomPosLoc);
		glEnableVertexAttribArray(topPosLoc);
		glVertexAttribDivisor(bottomPosLoc, 1);
		glVertexAttribDivisor(topPosLoc, 1);
		grass_pos[0]->AttribPointer(bottomPosLoc, 3, GL_FLOAT);
		grass_pos[1]->AttribPointer(topPosLoc, 3, GL_FLOAT);
		grassShader->Use();
		grass_mesh->DrawInstanced(grassInstanceCount);
		glVertexAttribDivisor(bottomPosLoc, 0);
		glVertexAttribDivisor(topPosLoc, 0);
		glDisableVertexAttribArray(bottomPosLoc);
		glDisableVertexAttribArray(topPosLoc);

		if (fShowMuzzleFlash) {
			m_rc->PushModelView();
				m_rc->SetModelView(Matrix44f::Identity());
				mainShader->Uniform("Mode", 2);
				muzzle_flash->Draw();
			m_rc->PopModelView();
		}

		m_rc->PushModelView();
		m_rc->PushProjection();
			float viewport[4] = { };
			glGetFloatv(GL_VIEWPORT, viewport);
			m_rc->SetProjection(Ortho2D(0, viewport[2], viewport[3], 0));
			m_rc->SetModelView(Matrix44f::Identity());

			mainShader->Uniform("Mode", 2);
			crosshair->location = Vector3f(viewport[2]*0.5f, viewport[3]*0.5f, 0);
			crosshair->Draw();
		m_rc->PopModelView();
		m_rc->PopProjection();

		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	m_rc->PopModelView();
}

void MainWindow::OnSize(int w, int h)
{
	glViewport(0, 0, w, h);
	m_rc->SetProjection(Perspective(45.0f, (float)w/h, 0.01f, 1000.0f));
}

void MainWindow::Update(int timeElapsed)
{
	float kt = timeElapsed / 16.0f;

	static int dir = 1;
	static float anim = 0.0f;
	anim += 0.01f * dir * kt;
	grassShader->Uniform("anim", anim);

	for (int i = 0; i < numTargets; i++)
	{
		Target &t = targets[i];
		if (t.fAnim)
		{
			const float delta = 5.0f * kt;
			t.rotAngle += delta;
			t.model->rotation *= Quaternion(Vector3f(1,0,0), delta);
			if (t.rotAngle >= 70.0f) t.fAnim = false;
		}
	}

	if (fGunAnim)
	{
		static int dir = 1;
		static int t = 0;
		float maxAnim = 15.0f;

		if (dir == 1)
			gunAnim += 0.5f * (maxAnim - gunAnim) * kt;
		else gunAnim -= 1.0f * kt;

		gun->rotation = Quaternion(Vector3f(1,0,0), gunAnim);

		if (gunAnim >= maxAnim - 0.4f) {
			dir = -1;
			gunAnim = maxAnim;
		}
		else if (dir == -1 && gunAnim <= 0.0f) {
			dir = 1;
			fGunAnim = false;
			fShowMuzzleFlash = false;
			t = 0;
			gun->rotation = Quaternion::Identity();
		}

		if (t >= 70) {
			fShowMuzzleFlash = false;
			t = 0;
		}
		t += timeElapsed;
	}

	const float step = 0.01875f * timeElapsed;
	const float maxAngle = (float)cos(DEG_TO_RAD(45.0f));
	bool moved = false;
	float dx = 0.0f, dz = 0.0f;

	if (GetAsyncKeyState('W'))
		dz = -step;
	else if (GetAsyncKeyState('S'))
		dz = step;

	if (GetAsyncKeyState('A'))
		dx = -step;
	else if (GetAsyncKeyState('D'))
		dx = step;

	if (dx != 0.0f || dz != 0.0f) {
		Vector3f pos = camera.GetPosition();
		float curHeight = terrain->GetHeightAt(pos.x, pos.z);

		camera.MoveX(dx);
		camera.MoveZ(dz);
		pos = camera.GetPosition();
	
		Plane eq = terrain->GetPolyEquation(pos.x, pos.z);
		float newHeight = eq.GetY(pos.x, pos.z);

		if (newHeight < curHeight) {
			moved = true;
		}
		else {
			float cosAngle = abs(eq.B) / sqrt(eq.A*eq.A + eq.B*eq.B + eq.C*eq.C);
			if (cosAngle >= maxAngle) {
				moved = true;
			}
			else {
				camera.MoveX(-dx);
				camera.MoveZ(-dz);
			}
		}
	}

	if (moved)
	{
		Vector3f pos = camera.GetPosition();
		finalHeight = growth + terrain->GetHeightAt(pos.x, pos.z);

		if (!CmpReal(pos.y, finalHeight))
		{
			float diff = pow(finalHeight - pos.y, 3)*0.1f;

			pos.y = min(pos.y + diff, finalHeight);
			camera.SetPosition(pos.x, pos.y, pos.z);
		}
	}
}

void MainWindow::OnMouseDown(MouseButton btn, int x, int y)
{
	Shot();
}

void MainWindow::OnMouseMove(UINT keyPressed, int x, int y)
{
	SetCursor(NULL);
	static bool centerCursor = false;
	const float angle = 0.04f;

	if (centerCursor) {
		centerCursor = false;
		return;
	}

	RECT clientRect = { };
	GetClientRect(m_hwnd, &clientRect);
	int centerX = clientRect.right / 2;
	int centerY = clientRect.bottom / 2;

	camera.RotateX(angle * (centerY - y));
	camera.RotateY(angle * (centerX - x));

	POINT cursorPos = { centerX, centerY };
	ClientToScreen(m_hwnd, &cursorPos);
	SetCursorPos(cursorPos.x, cursorPos.y);
	centerCursor = true;
}

void MainWindow::OnDestroy()
{
	for (int i = 0; i < numTargets; i++)
		delete targets[i].model;
	delete [] targets;

	delete skybox;
	delete mainShader;
	delete terrainShader;
	delete grassShader;
	delete grass_mesh;
	delete gun;
	delete muzzle_flash;
	delete crosshair;
	delete terrain;
	delete tex_ground;
	delete tex_grass;
	delete tex_rocks;
	delete tex_grassobj;
	PostQuitMessage(0);
}