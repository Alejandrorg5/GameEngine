#pragma once

#include "Component.h"
#include "../Mesh.h"
#include "TextureComponent.h"


class MeshComponent : public Component {
public:
    //Mesh loadedMesh;
    std::vector<Mesh> meshes;
    bool showVertexNormals = false;
    bool showFaceNormals = false;
    float normalLength = 0.25f;

    void Render(); // Solo declarada aquí
    void Update(float deltaTime) override;


    
};
