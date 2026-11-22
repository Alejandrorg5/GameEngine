#pragma once

#include "Component.h"
#include "../Mesh.h"



class MeshComponent : public Component {
public:
    std::vector<Mesh> meshes;
    bool showVertexNormals = false;
    bool showFaceNormals = false;
    float normalLength = 0.25f;

    void Render();
    void Update(float deltaTime) override;
};
