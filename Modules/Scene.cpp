#include "Scene.h"

struct RGB2 {
	float r;
	float g;
	float b;
};

int cubeIndicesSize;
Scene::Scene() : Module()
{
	name = "scene";
}

// Destructor
Scene::~Scene()
{
}

RGB2* GenerateChecker(int width, int height) {
	RGB2* checkerTexels = new RGB2[width * height];
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			float c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0)));
			checkerTexels[width * i + j] = { c, c, c };
		}
	}
	return checkerTexels;
}

void Scene::Init_ImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(Engine::GetInstance().window->window, Engine::GetInstance().window->context);
	ImGui_ImplOpenGL3_Init("#version 460");
}


// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	Engine::GetInstance().window->window;
	
	Init_ImGui();

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	
	Engine::GetInstance().window->GetWindowSize(wWidth, wHeight);

	static constexpr unsigned int tWidth = 512;
	static constexpr unsigned int tHeight = 256;

	RGB2* checkerTexels = GenerateChecker(tWidth, tHeight);

	tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_FLOAT, checkerTexels);
	delete[] checkerTexels;

	glViewport(0, 0, wWidth, wHeight);
	glClearColor(0.1f, 0.25f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Vértices del cubo 3D
	float cubeVertices[] = {
		// Frontal
		-0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		// Trasera
		-0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,   0.5f, 0.5f, 0.5f,   1.0f, 1.0f
	};

	cVBuff;
	glGenBuffers(1, &cVBuff);
	glBindBuffer(GL_ARRAY_BUFFER, cVBuff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	unsigned int cubeIndices[] = {
		0, 1, 2,  2, 3, 0,
		5, 4, 7,  7, 6, 5,
		4, 0, 3,  3, 7, 4,
		1, 5, 6,  6, 2, 1,
		4, 5, 1,  1, 0, 4,
		3, 2, 6,  6, 7, 3
	};

	cubeIndicesSize = sizeof(cubeIndices);

	iBuff;
	glGenBuffers(1, &iBuff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndicesSize, cubeIndices, GL_STATIC_DRAW);

	lastTime = SDL_GetTicks();

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	
	bool running = true;
	//Uint64 lastTime = SDL_GetTicks();

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL3_ProcessEvent(&e);

		if (e.type == SDL_EVENT_QUIT)
			running = false;
		else if (e.type == SDL_EVENT_WINDOW_RESIZED)
		{
			wWidth = e.window.data1;
			wHeight = e.window.data2;
			glViewport(0, 0, wWidth, wHeight);
		}
	}

	
	Uint64 currentTime = SDL_GetTicks();
	float deltaTime = (currentTime - lastTime) / 1000.0f;
	lastTime = currentTime;
	static float angle = 0.0f;
	angle += glm::radians(50.0f) * deltaTime;

	// Nuevos frames de ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Begin("Engine Control Panel");
	ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
	ImGui::End();

	// Renderizado 3D de la escena
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
	glLoadMatrixf(glm::value_ptr(projection));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.5f, 1.0f, 0.0f));
	glm::mat4 mvp = view * model;
	glLoadMatrixf(glm::value_ptr(mvp));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, cVBuff);
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 8, reinterpret_cast<void*>(0));
	glColorPointer(3, GL_FLOAT, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 3));
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 6));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuff);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glDrawElements(GL_TRIANGLES, cubeIndicesSize / sizeof(unsigned int), GL_UNSIGNED_INT, NULL);

	// Renderizado de ImGui sobre la escena
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(Engine::GetInstance().window->window);

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	return true;
}