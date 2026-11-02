#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "GameObject.h"
#include "TextureLoader.h"
#include "../Modules/Log.h"

class ModelLoader {
public:
    static std::unique_ptr<GameObject> LoadFBX(const std::string& path) {
        const auto filePath = std::filesystem::u8path(path);
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file || file.tellg() <= 0) { LOG("Assimp: cannot open %s", path.c_str()); return nullptr; }
        std::vector<char> data(static_cast<size_t>(file.tellg()));
        file.seekg(0);
        if (!file.read(data.data(), static_cast<std::streamsize>(data.size()))) return nullptr;
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(data.data(), data.size(),
            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
            aiProcess_ValidateDataStructure, "fbx");
        if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode || !scene->HasMeshes()) {
            LOG("Assimp: import failed for %s: %s", path.c_str(), importer.GetErrorString());
            return nullptr;
        }
        auto model = std::make_unique<GameObject>(filePath.stem().u8string());
        ProcessNode(*scene, *scene->mRootNode, *model, filePath.parent_path());
        LOG("Assimp: imported %s (%u meshes)", path.c_str(), scene->mNumMeshes);
        return model;
    }

private:
    static void ProcessNode(const aiScene& scene, const aiNode& node, GameObject& parent,
                            const std::filesystem::path& directory) {
        auto* object = parent.AddChild(node.mName.length ? node.mName.C_Str() : "Node");
        auto* transform = object->GetComponent<TransformComponent>();
        aiVector3D scale, position;
        aiQuaternion rotation;
        node.mTransformation.Decompose(scale, rotation, position);
        transform->position = glm::vec3(position.x, position.y, position.z);
        transform->scale = glm::vec3(scale.x, scale.y, scale.z);
        transform->rotation = glm::degrees(glm::eulerAngles(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z)));
        for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
            const aiMesh* source = scene.mMeshes[node.mMeshes[i]];
            auto* meshObject = object->AddChild(source->mName.length ? source->mName.C_Str() : "Mesh");
            meshObject->GetComponent<MeshComponent>()->meshes.push_back(ProcessMesh(source));
            if (source->mMaterialIndex < scene.mNumMaterials)
                LoadMaterial(scene, *scene.mMaterials[source->mMaterialIndex], *meshObject, directory);
        }
        for (unsigned int i = 0; i < node.mNumChildren; ++i)
            ProcessNode(scene, *node.mChildren[i], *object, directory);
    }

    static void LoadMaterial(const aiScene& scene, const aiMaterial& material, GameObject& object,
                             const std::filesystem::path& directory) {
        aiString reference;
        if (material.GetTexture(aiTextureType_DIFFUSE, 0, &reference) != AI_SUCCESS) return;
        auto* texture = object.GetComponent<TextureComponent>();
        GLuint id = 0;
        if (const aiTexture* embedded = scene.GetEmbeddedTexture(reference.C_Str())) {
            if (embedded->mHeight == 0)
                id = TextureLoader::LoadMemory(embedded->pcData, embedded->mWidth, reference.C_Str());
            else
                id = TextureLoader::Upload(embedded->mWidth, embedded->mHeight, GL_BGRA, embedded->pcData);
            if (id) texture->SetTexture(id, std::string("Embedded: ") + reference.C_Str());
            return;
        }
        std::string relative = reference.C_Str();
        std::replace(relative.begin(), relative.end(), '\\', '/');
        auto candidate = std::filesystem::u8path(relative);
        if (!candidate.is_absolute()) candidate = directory / candidate;
        std::error_code error;
        if (!std::filesystem::is_regular_file(candidate, error))
            candidate = directory / std::filesystem::u8path(relative).filename();
        id = TextureLoader::LoadTexture(candidate.u8string());
        if (id) texture->SetTexture(id, candidate.u8string());
    }

    static Mesh ProcessMesh(const aiMesh* aiMeshPtr) {
        Mesh mesh;

        // 1. Extraer Vértices
        for (unsigned int i = 0; i < aiMeshPtr->mNumVertices; i++) {
            Render::Vertex vertex;

            vertex.Position.x = aiMeshPtr->mVertices[i].x;
            vertex.Position.y = aiMeshPtr->mVertices[i].y;
            vertex.Position.z = aiMeshPtr->mVertices[i].z;

            if (aiMeshPtr->HasNormals()) {
                vertex.Normal.x = aiMeshPtr->mNormals[i].x;
                vertex.Normal.y = aiMeshPtr->mNormals[i].y;
                vertex.Normal.z = aiMeshPtr->mNormals[i].z;
            }
            else {
                vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            if (aiMeshPtr->HasTextureCoords(0)) {
                vertex.TexCoords.x = aiMeshPtr->mTextureCoords[0][i].x;
                vertex.TexCoords.y = aiMeshPtr->mTextureCoords[0][i].y;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            mesh.vertices.push_back(vertex);
        }

        // 2. Extraer Índices
        for (unsigned int i = 0; i < aiMeshPtr->mNumFaces; i++) {
            aiFace face = aiMeshPtr->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                mesh.indices.push_back(face.mIndices[j]);
            }
        }

        // 3. Configurar los buffers OpenGL de esta malla específica
        mesh.SetupMesh();
        return mesh;
    }
};