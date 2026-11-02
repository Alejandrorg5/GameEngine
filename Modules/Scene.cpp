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

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (selectedObject == obj) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	// Si el objeto no tiene hijos no muestrar la flecha
	if (obj->GetChildren().empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	std::string label = obj->name + "##" + std::to_string(reinterpret_cast<uintptr_t>(obj));
	bool nodeOpen = ImGui::TreeNodeEx(label.c_str(), flags);

	if (ImGui::IsItemClicked()) {
		selectedObject = obj;
	}

	// Si el nodo está abierto, recorremos sus hijos
	if (nodeOpen) {
		for (const auto& child : obj->GetChildren()) {
			DrawGameObjectNode(child.get()); 
		}
		ImGui::TreePop();
	}
}

void Scene::Show_ImGui(float dt) {
	ImGui_ImplSDL3_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	//Para dejar las ventanas fijas
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

	/* Lista de objetos */
	int list_width = 300;

	ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(list_width, viewport->WorkSize.y), ImGuiCond_Always);

	ImGui::Begin("Engine Control Panel", nullptr, windowFlags);
	ImGui::Text("FPS: %.1f", 1.0f / dt);
	ImGui::Separator();
	for (const auto& obj : game_objects) {
		DrawGameObjectNode(obj);
	}
	ImGui::End();

	/* Inspector */
	int inspectorWidth = 400;
	float inspectorPosX = viewport->WorkPos.x + viewport->WorkSize.x - inspectorWidth;

	ImGui::SetNextWindowPos(ImVec2(inspectorPosX, viewport->WorkPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(inspectorWidth, viewport->WorkSize.y), ImGuiCond_Always);

	ImGui::Begin("Inspector", nullptr, windowFlags);
	if (selectedObject != nullptr) {
		// Mostrar y editar el nombre del GameObject
		char nameBuffer[128];
		strcpy_s(nameBuffer, sizeof(nameBuffer), selectedObject->name.c_str());
		if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
			selectedObject->name = nameBuffer;
		}

		ImGui::Separator();

		// Transform
		if (selectedObject->GetComponent<TransformComponent>() != nullptr) {
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::DragFloat3("Position", &selectedObject->GetComponent<TransformComponent>()->position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &selectedObject->GetComponent<TransformComponent>()->rotation.x, 0.5f);
				ImGui::DragFloat3("Scale", &selectedObject->GetComponent<TransformComponent>()->scale.x, 0.05f);
			}
		}

		
		TextureComponent* texComp = selectedObject->GetComponent<TextureComponent>();
		if (texComp != nullptr) {
			if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("Texture ID: %d", texComp->textureID);
				ImGui::Text("Path: %s", texComp->filePath.c_str());
			}
		}

	}
	else {
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No GameObject selected.");
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