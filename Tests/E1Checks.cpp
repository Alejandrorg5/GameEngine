#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include "../Core/Components/MeshComponent.h"
#include "../Core/Components/TextureComponent.h"
#include "../Core/Components/TransformComponent.h"
#include "../Modules/Engine.h"
#include "../Modules/Input.h"
#include "../Modules/Scene.h"
#include "../Modules/Window.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

static void Check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

// Exercise the native Unicode drop message, SDL's translation, and the input pump.
static void Drop(const fs::path& file) {
    const std::wstring path = fs::absolute(file).wstring();
    const size_t bytes = (path.size() + 2) * sizeof(wchar_t);
    auto handle = GlobalAlloc(GHND, sizeof(DROPFILES) + bytes);
    Check(handle != nullptr, "allocate native drop");
    auto* drop = static_cast<DROPFILES*>(GlobalLock(handle));
    drop->pFiles = sizeof(DROPFILES);
    drop->fWide = TRUE;
    std::memcpy(drop + 1, path.c_str(), path.size() * sizeof(wchar_t));
    GlobalUnlock(handle);
    auto& engine = Engine::GetInstance();
    const auto props = SDL_GetWindowProperties(engine.GetWindow().Handle());
    auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    Check(hwnd != nullptr, "native window handle");
    SendMessageW(hwnd, WM_DROPFILES, reinterpret_cast<WPARAM>(handle), 0);
    engine.GetInput().PreUpdate();
}

static void CheckMaterials(GameObject& node, int& meshCount) {
    if (auto* mesh = node.GetComponent<MeshComponent>(); mesh && !mesh->meshes.empty()) {
        ++meshCount;
        auto* texture = node.GetComponent<TextureComponent>();
        Check(texture && texture->Id() && texture->Width() > 0, "diffuse texture on imported mesh");
    }
    for (const auto& child : node.GetChildren()) CheckMaterials(*child, meshCount);
}

static void CheckScene(Scene& scene, const fs::path& fixtures) {
    const auto source = fs::u8path(Scene::AssetPath("Assets/Models"));
    fs::create_directories(fixtures);
    const auto model = fixtures / L"Casa - v1.2.FBX";
    const auto texture = fixtures / L"pared - versi\u00f3n.2.PNG";
    // Same byte count keeps FBX binary offsets intact. Only the last directory
    // of the exporter's original texture path survives the move to this fixture.
    constexpr char oldRef[] = "Baker_house.png", movedRef[] = "x/y/z/house.png";
    static_assert(sizeof(oldRef) == sizeof(movedRef));
    std::ifstream input(source / "BakerHouse.fbx", std::ios::binary);
    std::string bytes{std::istreambuf_iterator<char>(input), {}};
    auto offset = bytes.find(oldRef);
    Check(offset != std::string::npos, "Baker House diffuse reference");
    while (offset != std::string::npos) {
        bytes.replace(offset, sizeof(oldRef)-1, movedRef);
        offset = bytes.find(oldRef, offset + sizeof(movedRef)-1);
    }
    std::ofstream(model, std::ios::binary).write(bytes.data(), bytes.size());
    fs::create_directories(fixtures / "z");
    fs::copy_file(source / "Baker_house.png", fixtures / "z/house.png", fs::copy_options::overwrite_existing);
    fs::copy_file(source / "Baker_house.png", texture, fs::copy_options::overwrite_existing);

    const auto before = scene.Root().GetChildren().size();
    Drop(model);
    auto* imported = scene.GetSelection();
    Check(scene.Root().GetChildren().size() == before + 1 && imported->GetName() == model.stem().u8string(),
          "FBX dropped from a complex path");
    Check(!imported->GetComponent<MeshComponent>() && !imported->GetComponent<TextureComponent>(), "group components");
    int meshes = 0;
    CheckMaterials(*imported, meshes);
    Check(meshes > 0, "FBX contains textured meshes");
    Check(scene.DeleteObject(imported) && scene.GetSelection() == nullptr, "delete selected subtree");

    scene.CreatePrimitive(Scene::Primitive::Cube);
    auto& cube = *scene.GetSelection();
    const auto& mesh = cube.GetComponent<MeshComponent>()->meshes.front();
    Check(mesh.indices.size() == 36, "cube triangle count");
    for (size_t face = 0; face < 36; face += 6) {
        const auto uv = [&](size_t i) { return mesh.vertices[mesh.indices[face+i]].TexCoords; };
        Check(uv(0) == uv(3) && uv(2) == uv(4), "shared cube diagonal has consistent UVs");
        Check(uv(0) != uv(1) && uv(0) != uv(2) && uv(0) != uv(5) &&
              uv(1) != uv(2) && uv(1) != uv(5) && uv(2) != uv(5), "four distinct UV corners");
    }
    cube.GetComponent<TransformComponent>()->position.x = 500;
    Check(scene.Raycast({500,1,-5}, {0,0,1}, 10) == &cube, "exact picking");
    Check(scene.Raycast({502,1,-5}, {0,0,1}, 10) == nullptr, "picking misses empty space");
    Check(cube.RemoveComponent<TextureComponent>(), "remove texture");
    Drop(texture);
    auto* material = cube.GetComponent<TextureComponent>();
    Check(material && material->Id() && fs::equivalent(fs::u8path(material->Path()), texture),
          "drop adds a texture to a mesh");
    const auto dds = fixtures / "one pixel - v1.0.DDS";
    // DDS magic, 124-byte header, BGRA masks and one opaque texel.
    const std::array<std::uint32_t, 33> pixel = {
        0x20534444,124,0x100f,1,1,4,0,0, 0,0,0,0,0,0,0,0,0,0,0,
        32,0x41,0,32,0x00ff0000,0x0000ff00,0x000000ff,0xff000000,
        0x1000,0,0,0,0,0xff336699
    };
    std::ofstream(dds, std::ios::binary).write(reinterpret_cast<const char*>(pixel.data()), sizeof(pixel));
    Drop(dds);
    Check(material->Width() == 1 && fs::equivalent(fs::u8path(material->Path()), dds), "DDS drop");
    const auto previous = material->Id();
    Drop(fixtures / L"missing.png");
    Check(material->Id() == previous, "failed texture import preserves original");
    Check(cube.AddComponent<TextureComponent>() == material, "one component per type");
}

int main(int argc, char** argv) {
    if (argc != 2) return 1;
    auto& engine = Engine::GetInstance();
    if (!engine.Awake() || !engine.Start()) return 1;
    int result = 0;
    try {
        CheckScene(engine.GetScene(), fs::u8path(argv[1]) / L"Gr\u00e1ficos - v1.2");
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        result = 1;
    }
    engine.CleanUp();
    return result;
}
