#include "../Modules/Editor.h"
#include "../Modules/Engine.h"
#include "../Modules/RunEngine.h"

int main() {
    Engine::GetInstance().AddModule(std::make_shared<Editor>());
    return RunEngine();
}
