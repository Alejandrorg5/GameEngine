#pragma once

#include "Component.h"
#include "../Mesh.h"
#include "TextureComponent.h"


class MeshComponent : public Component {
public:
    //Mesh loadedMesh;
    std::vector<Mesh> meshes;

    void Render(); // Solo declarada aquí
    void Update(float deltaTime) override;
    void SetTexture(GLuint texID);

    
};