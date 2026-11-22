#include "Camera.h"
#include "../Modules/Engine.h"
#include "../Modules/RunEngine.h"
#include "../Modules/Scene.h"
#include "../Modules/Window.h"

namespace {
class GameView final : public Module {
    Camera camera;
public:
    bool Start() override {
        camera.yaw = -135.0f;
        camera.Focus(&Engine::GetInstance().GetScene().Root(), true);
        return true;
    }
    bool Update(float dt) override {
        auto& engine = Engine::GetInstance();
        int width, height;
        SDL_GetWindowSizeInPixels(engine.GetWindow().Handle(), &width, &height);
        if (width <= 0 || height <= 0) return true;
        glViewport(0, 0, width, height);
        const auto projection = camera.Projection(static_cast<float>(width) / height);
        engine.GetScene().RenderScene(camera.GetViewMatrix(), projection, dt);
        SDL_GL_SwapWindow(engine.GetWindow().Handle());
        return true;
    }
};
}

int main() {
    Engine::GetInstance().AddModule(std::make_shared<GameView>());
    return RunEngine();
}
