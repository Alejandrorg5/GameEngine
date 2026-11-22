#pragma once
#include <filesystem>
#include <memory>
#include <string>

class GameObject;
class Mesh;
struct aiScene;
struct aiNode;
struct aiMaterial;
struct aiMesh;

class ModelLoader {
public:
    static std::unique_ptr<GameObject> LoadFBX(const std::string& path);

private:
    static void ProcessNode(const aiScene& scene, const aiNode& node, GameObject& parent,
                            const std::filesystem::path& directory);

    static void LoadMaterial(const aiScene& scene, const aiMaterial& material, GameObject& object,
                             const std::filesystem::path& directory);

    static Mesh ProcessMesh(const aiMesh* aiMeshPtr);
};