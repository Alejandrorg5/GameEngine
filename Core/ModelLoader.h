#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include <iostream>
#include "Components/MeshComponent.h"
#include "../Core/TextureLoader.h"
class ModelLoader {
public:
    static void LoadFBXIntoRenderer(const std::string& path, MeshComponent* renderer, string texturepath) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }

        renderer->meshes.clear();

        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* aiMeshPtr = scene->mMeshes[i];
            Mesh mesh = ProcessMesh(aiMeshPtr);

            if (aiMeshPtr->mMaterialIndex >= 0) {
                aiMaterial* material = scene->mMaterials[aiMeshPtr->mMaterialIndex];
                aiString texPath;

                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                    std::string pathStr = texPath.C_Str();
                    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');

                    // CASO A: La textura está INCRUSTADA en el FBX (empieza por '*')
                    if (pathStr[0] == '*') {
                        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(pathStr.c_str());
                        if (embeddedTexture) {
                            GLuint texID = TextureLoader::LoadEmbeddedTexture(embeddedTexture);
                            if (texID != 0) {
                                mesh.textureID = texID;
                            }
                        }
                    }
                    // CASO B: La textura es un archivo externo en una carpeta
                    else {
                        if (!texturepath.empty()) {
                            GLuint texID = TextureLoader::LoadTexture(texturepath);
                            if (texID != 0) {
                                mesh.textureID = texID;
                            }
                        }
                        else {
                            texturepath = "../Assets/Models/" + pathStr;
                            GLuint texID = TextureLoader::LoadTexture(texturepath);
                            if (texID != 0) {
                                mesh.textureID = texID;
                            }
                        }
                    }
                }
            }
            renderer->meshes.push_back(mesh);
        }
    }

private:
    static Mesh ProcessMesh(aiMesh* aiMeshPtr) {
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