#pragma once
#include "Module.h"
#include "../Core/GameObject.h"
#include "../Core/Grid.h"
#include <assimp/cimport.h>
#include <memory>
class Editor;
class Scene : public Module {
public:
    Scene();
    ~Scene() override;
    bool Awake() override;
    bool Start() override;
    bool Update(float dt) override;
    bool CleanUp() override;
    void RenderScene(const glm::mat4& view, const glm::mat4& projection, float dt);
    void UpdateViewport(int, int) {}
    bool ImportModel(const std::string& path);
    void HandleDrop(const char* path);
    GameObject* Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);
    GameObject* GetModelRoot(GameObject* object);
    bool DeleteObject(GameObject* object);
    enum class Primitive { Cube, Sphere, Pyramid, Cylinder };
    void CreatePrimitive(Primitive type);
    static std::string AssetPath(const std::string& relative);
    GameObject* GetSelection() const { return selectedObject; }
    void Select(GameObject* object) { selectedObject = object; }
    GameObject root;
    std::vector<GameObject*> game_objects;
    bool showGrid = true, wireframe = false, depthTest = true;
    glm::vec3 background{0.12f, 0.13f, 0.15f};
private:
    std::unique_ptr<Editor> editor;
    aiLogStream stream{};
    Grid sceneGrid;
    GameObject* selectedObject = nullptr;
};
