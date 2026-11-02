#include "MeshComponent.h"
#include "../GameObject.h"
#include <glm/gtc/type_ptr.hpp>

void MeshComponent::Render() {
    GLuint currentTextureID = 0;

    /* Posicion */
    glPushMatrix();

    if (owner->GetComponent<TextureComponent>() != nullptr) {
        glm::mat4 modelMatrix = owner->GetComponent<TransformComponent>()->GetModelMatrix();
        // Multiplicamos la matriz del modelo actual en la pila de OpenGL
        glMultMatrixf(glm::value_ptr(modelMatrix));
    }


    /* Textura */
    if (owner != nullptr) {
        TextureComponent* texComp = owner->GetComponent<TextureComponent>();
        if (texComp != nullptr) {
            currentTextureID = texComp->textureID;
        }
    }

    glEnable(GL_TEXTURE_2D);

    for (auto& mesh : meshes) {
        // Si encontramos una textura en el componente, la usamos; si no, se dibuja sin textura (o con la por defecto)
        if (currentTextureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, currentTextureID);
        }

        mesh.Draw();

        if (currentTextureID != 0) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    glDisable(GL_TEXTURE_2D);


}

void MeshComponent::Update(float deltaTime) {
    Render();
}

void MeshComponent::SetTexture(GLuint texID) {
    for (auto& mesh : meshes) {
        mesh.textureID = texID;
    }
}