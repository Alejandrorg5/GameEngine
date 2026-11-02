#pragma once

#include "Engine.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Module.h"

#include "../Core/GameObject.h"
#include "../Core/Components/MeshComponent.h"
#include "../Core/Grid.h"
#include "../Core/Camera.h"
#include "../Core/ModelLoader.h"
#include "../Core/TextureLoader.h"

#include <il.h>
#include <glad/glad.h>
#include <SDL3/SDL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ImGui e integraciones provistas por vcpkg
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct SDL_Texture;

class Scene : public Module
{
public:
	
	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void Init_ImGui();
	void Scene::DrawGameObjectNode(GameObject* obj);
	void Show_ImGui(float dt);
	void RenderScene(float dt);
	void UpdateViewport(int w, int h);

	vector<GameObject*> game_objects;
	GameObject root;
	MeshComponent* renderer = nullptr;

	aiLogStream stream;
private:
	int wWidth, wHeight;
	Uint64 lastTime;
	GLuint cVBuff;
	GLuint iBuff;
	GLuint tex;
	Grid sceneGrid;
	Camera editorCamera;
	GameObject* selectedObject = nullptr;
};