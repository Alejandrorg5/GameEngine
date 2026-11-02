#include "Scene.h"


struct RGB {
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

RGB* GenerateChecker(int width, int height) {
	RGB* checkerTexels = new RGB[width * height];
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

void Scene::DrawGameObjectNode(GameObject* obj) {
	if (!obj) return;

	// Flags útiles para ImGui (puedes añadir ImGuiTreeNodeFlags_Leaf si no tiene hijos)
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

	// Si el objeto no tiene hijos, podemos marcarlo como hoja para que no muestre la flecha de desplegar
	if (obj->GetChildren().empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	// Renderizamos el nodo con el nombre del GameObject (usamos un puntero o ID único combinando la dirección de memoria para evitar conflictos si hay nombres repetidos)
	std::string label = obj->name + "##" + std::to_string(reinterpret_cast<uintptr_t>(obj));
	bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

	// Si el nodo está abierto, recorremos recursivamente a sus hijos
	if (nodeOpen) {
		for (const auto& child : obj->GetChildren()) {
			DrawGameObjectNode(child.get()); // Llamada recursiva para cada hijo
		}

		ImGui::TreePop();
	}
}

void Scene::Show_ImGui(float dt) {
	ImGui_ImplSDL3_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();

	ImGui::Begin("Engine Control Panel");
	ImGui::Text("FPS: %.1f", 1.0f / dt);
	ImGui::Separator();

	for (const auto& obj : game_objects) {
		DrawGameObjectNode(obj);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	Engine::GetInstance().window->window;
	
	ilInit();

	Init_ImGui();

	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	
	Engine::GetInstance().window->GetWindowSize(wWidth, wHeight);

	glViewport(0, 0, wWidth, wHeight);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	lastTime = SDL_GetTicks();
	
	root.name = "Root";

	GameObject* model = root.AddChild("Model1");
	
	MeshComponent* mesh = model->GetComponent<MeshComponent>();
	ModelLoader::LoadFBXIntoRenderer("../Assets/Models/BakerHouse.fbx", mesh);
	
	TextureComponent* texture = model->AddComponent<TextureComponent>();
	GLuint customTexture = TextureLoader::LoadTexture("../Assets/Models/Baker_House.png");
	texture->SetTexture(customTexture, "Assets/Models/Baker_House.png");
	

	mesh->SetTexture(customTexture);

	game_objects.push_back(&root);


	// Grid:
	sceneGrid.Init(50, 1.0f);

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

	editorCamera.Update();

	RenderScene(dt);
	
	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;
	return ret;
}

void Scene::RenderScene(float dt)
{
	Uint64 currentTime = SDL_GetTicks();
	float deltaTime = (currentTime - lastTime) / 1000.0f;
	lastTime = currentTime;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Configurar las matrices de Proyección y Vista (Cámara)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
	glLoadMatrixf(glm::value_ptr(projection));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Alejar la cámara en Z para ver el modelo FBX cargado
	glm::mat4 view = editorCamera.GetViewMatrix();
	glLoadMatrixf(glm::value_ptr(view));

	//Draw the grid
	sceneGrid.Draw();

	//Update GameObjects
	for (const auto& obj : game_objects) {
		obj->Update(dt);
	}

	// Renderizado de ImGui sobre la escena
	Show_ImGui(deltaTime);

	SDL_GL_SwapWindow(Engine::GetInstance().window->window);
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	aiDetachAllLogStreams();

	return true;
}

void Scene::UpdateViewport(int w, int h) {
	wWidth = w;
	wHeight = h;
	glViewport(0, 0, wWidth, wHeight);
}