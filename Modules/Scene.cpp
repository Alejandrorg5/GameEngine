#include "Scene.h"
#include "Engine.h"
#include "Render.h"
#include "Editor.h"
#include "Log.h"
#include "../Core/ModelLoader.h"
#include <functional>
#include <cctype>
#include <filesystem>

Scene::Scene() { name = "scene"; }
Scene::~Scene() = default;
std::string Scene::AssetPath(const std::string& relative) {
    const std::filesystem::path base = std::filesystem::u8path(SDL_GetBasePath());
    for (const auto& directory : {base, (base / "..").lexically_normal(), std::filesystem::current_path()}) {
        const auto path = directory / std::filesystem::u8path(relative);
        if (std::filesystem::exists(path)) return path.u8string();
    }
    return (base / std::filesystem::u8path(relative)).u8string();
}
bool Scene::Awake() {
    ilInit();
    LOG("DevIL initialized: %s", ilGetString(IL_VERSION_NUM));
    stream.callback = [](const char* message, char*) { LOG("Assimp: %s", message); };
    aiAttachLogStream(&stream);
    editor = std::make_unique<Editor>();
    return editor->Init();
}
bool Scene::Start() {
    root.name = "Scene";
    if (!ImportModel(AssetPath("Assets/Models/BakerHouse.fbx"))) return false;
    auto* transform = selectedObject->GetComponent<TransformComponent>();
    // Convert the supplied Blender FBX presentation to the editor's Y-up, metre scale.
    transform->rotation.x = 90.0f;
    transform->scale = glm::vec3(0.01f);
    glm::vec3 low, high;
    if (selectedObject->GetWorldBounds(low, high)) transform->position.y -= low.y;
    if (!ImportModel(AssetPath("Assets/Models/sedan.fbx"))) return false;
    auto* car = selectedObject->GetComponent<TransformComponent>();
    car->scale = glm::vec3(0.01f);
    car->position.x = -5.0f;
    if (selectedObject->GetWorldBounds(low, high)) car->position.y -= low.y;
    if (!ImportModel(AssetPath("Assets/Models/armchair.fbx"))) return false;
    auto* chair = selectedObject->GetComponent<TransformComponent>();
    chair->scale = glm::vec3(0.01f);
    chair->position.x = 5.0f;
    if (selectedObject->GetWorldBounds(low, high)) chair->position.y -= low.y;
    editor->camera.yaw = -135.0f;
    editor->camera.Focus(&root, true);
    selectedObject = root.children.front().get();
    game_objects.push_back(&root);
    sceneGrid.Init(50, 1.0f);
    return true;
}
bool Scene::Update(float dt) { editor->Draw(*this, dt); return true; }
void Scene::RenderScene(const glm::mat4& view, const glm::mat4& projection, float dt) {
    glClearColor(background.r, background.g, background.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    auto& renderer = *Engine::GetInstance().render;
    renderer.BeginScene(view, projection);
    if (showGrid) { renderer.SetObject(glm::mat4(1.0f), 0, true); sceneGrid.Draw(); }
    root.Update(dt);
    glm::vec3 low, high;
    if (selectedObject && selectedObject != &root && selectedObject->GetWorldBounds(low, high)) {
        glm::vec3 corners[8];
        for (int i = 0; i < 8; ++i) corners[i] = {i & 1 ? high.x : low.x, i & 2 ? high.y : low.y, i & 4 ? high.z : low.z};
        std::vector<glm::vec3> lines;
        for (int i = 0; i < 8; ++i) for (int axis = 0; axis < 3; ++axis)
            if (!(i & (1 << axis))) { lines.push_back(corners[i]); lines.push_back(corners[i | (1 << axis)]); }
        renderer.DrawLines(lines, glm::vec3(1.0f, 0.65f, 0.1f));
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}
bool Scene::CleanUp() {
    LOG("Freeing scene and editor");
    selectedObject = nullptr;
    game_objects.clear(); root.children.clear(); root.components.clear();
    sceneGrid.CleanUp();
    if (editor) { editor->CleanUp(); editor.reset(); }
    aiDetachAllLogStreams();
    return true;
}

bool Scene::ImportModel(const std::string& path) {
    auto model = ModelLoader::LoadFBX(path);
    if (!model) return false;
    model->parent = &root;
    selectedObject = model.get();
    root.children.push_back(std::move(model));
    return true;
}

void Scene::HandleDrop(const char* path) {
    if (!path || !*path) return;
    std::string extension = std::filesystem::u8path(path).extension().u8string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension == ".fbx") {
        ImportModel(path);
        return;
    }
    if (extension != ".png" && extension != ".dds") {
        LOG("Unsupported dropped file: %s", path);
        return;
    }
    if (!selectedObject) {
        LOG("Select an object before dropping a texture.");
        return;
    }
    std::vector<TextureComponent*> targets;
    std::function<void(GameObject&)> collect = [&](GameObject& object) {
        auto* mesh = object.GetComponent<MeshComponent>();
        auto* texture = object.GetComponent<TextureComponent>();
        if (texture && (&object == selectedObject || (mesh && !mesh->meshes.empty())))
            targets.push_back(texture);
        for (const auto& child : object.GetChildren()) collect(*child);
    };
    collect(*selectedObject);
    // Each component owns its texture. Prepare all replacements before changing the scene.
    std::vector<GLuint> replacements;
    for (size_t i = 0; i < targets.size(); ++i) {
        const GLuint id = TextureLoader::LoadTexture(path);
        if (!id) {
            for (GLuint replacement : replacements) glDeleteTextures(1, &replacement);
            return;
        }
        replacements.push_back(id);
    }
    for (size_t i = 0; i < targets.size(); ++i) {
        targets[i]->SetTexture(replacements[i], path);
        targets[i]->showChecker = false;
    }
    LOG("Texture applied to %zu components: %s", targets.size(), path);
}


void Scene::CreatePrimitive(Primitive type) {
    Mesh mesh;
    const auto triangle = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
        const glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        const auto start = static_cast<unsigned int>(mesh.vertices.size());
        mesh.vertices.push_back({a, normal, {0, 0}});
        mesh.vertices.push_back({b, normal, {1, 0}});
        mesh.vertices.push_back({c, normal, {0.5f, 1}});
        mesh.indices.insert(mesh.indices.end(), {start, start + 1, start + 2});
    };
    const char* name = "Cube";
    if (type == Primitive::Cube || type == Primitive::Pyramid) {
        const glm::vec3 corners[] = {{-1,0,-1},{1,0,-1},{1,0,1},{-1,0,1},
            {-1,2,-1},{1,2,-1},{1,2,1},{-1,2,1}};
        if (type == Primitive::Cube) {
            const unsigned int faces[][4] = {{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{4,5,6,7},{3,2,1,0}};
            for (const auto& face : faces) {
                triangle(corners[face[0]],corners[face[2]],corners[face[1]]);
                triangle(corners[face[0]],corners[face[3]],corners[face[2]]);
            }
        } else {
            name = "Pyramid";
            for (int i = 0; i < 4; ++i) triangle(corners[i], {0,2,0}, corners[(i+1)%4]);
            triangle(corners[0],corners[1],corners[2]); triangle(corners[0],corners[2],corners[3]);
        }
    } else if (type == Primitive::Sphere) {
        name = "Sphere";
        constexpr unsigned int rings = 16, segments = 24;
        for (unsigned int y = 0; y <= rings; ++y) for (unsigned int x = 0; x <= segments; ++x) {
            const float u = static_cast<float>(x)/segments, v = static_cast<float>(y)/rings;
            const float angle = u * 6.2831853f, latitude = v * 3.14159265f;
            const glm::vec3 normal(sin(latitude)*cos(angle), cos(latitude), sin(latitude)*sin(angle));
            mesh.vertices.push_back({normal + glm::vec3(0,1,0), normal, {u,v}});
        }
        for (unsigned int y = 0; y < rings; ++y) for (unsigned int x = 0; x < segments; ++x) {
            const unsigned int a = y*(segments+1)+x, b = a+segments+1;
            if (y > 0) mesh.indices.insert(mesh.indices.end(), {a,a+1,b});
            if (y+1 < rings) mesh.indices.insert(mesh.indices.end(), {a+1,b+1,b});
        }
    } else {
        name = "Cylinder";
        constexpr unsigned int segments = 24;
        for (unsigned int i = 0; i < segments; ++i) {
            const float a = i * 6.2831853f/segments, b = (i+1)*6.2831853f/segments;
            const glm::vec3 p(cos(a),0,sin(a)), q(cos(b),0,sin(b)), up(0,2,0);
            triangle(p,p+up,q); triangle(q,p+up,q+up);
            triangle({0,0,0},p,q); triangle({0,2,0},q+up,p+up);
        }
    }
    mesh.SetupMesh();
    auto* object = root.AddChild(name);
    object->GetComponent<MeshComponent>()->meshes.push_back(std::move(mesh));
    Select(object);
    LOG("Created primitive: %s", name);
}


GameObject* Scene::GetModelRoot(GameObject* object) {
    if (!object || object == &root) return nullptr;
    while (object->parent && object->parent != &root) object = object->parent;
    return object;
}

bool Scene::DeleteObject(GameObject* object) {
    if (!object || object == &root || !object->parent) return false;
    auto& siblings = object->parent->children;
    const auto found = std::find_if(siblings.begin(), siblings.end(),
        [object](const auto& child) { return child.get() == object; });
    if (found == siblings.end()) return false;
    for (auto* selected = selectedObject; selected; selected = selected->parent) {
        if (selected == object) { selectedObject = nullptr; break; }
    }
    LOG("Deleted scene object: %s", object->name.c_str());
    siblings.erase(found);
    return true;
}

GameObject* Scene::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    if (maxDistance <= 0 || glm::length(direction) < 1e-8f) return nullptr;
    const glm::vec3 ray = glm::normalize(direction);
    struct Candidate { GameObject* object; float entry; };
    std::vector<Candidate> candidates;
    std::function<void(GameObject&)> gather = [&](GameObject& object) {
        const auto* component = object.GetComponent<MeshComponent>();
        if (component && !component->meshes.empty()) {
            glm::vec3 low, high;
            if (object.GetWorldBounds(low, high)) {
                float nearT = 0, farT = maxDistance;
                bool hit = true;
                for (int axis = 0; axis < 3; ++axis) {
                    if (std::abs(ray[axis]) < 1e-8f) {
                        if (origin[axis] < low[axis] || origin[axis] > high[axis]) { hit = false; break; }
                    } else {
                        float a = (low[axis] - origin[axis]) / ray[axis];
                        float b = (high[axis] - origin[axis]) / ray[axis];
                        if (a > b) std::swap(a, b);
                        nearT = std::max(nearT, a); farT = std::min(farT, b);
                        if (nearT > farT) { hit = false; break; }
                    }
                }
                if (hit) candidates.push_back({&object, nearT});
            }
        }
        for (const auto& child : object.children) gather(*child);
    };
    gather(root);
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.entry < b.entry; });
    GameObject* nearest = nullptr;
    float closest = maxDistance;
    for (const auto& candidate : candidates) {
        if (candidate.entry > closest) break;
        const auto model = candidate.object->GetGlobalMatrix();
        for (const auto& mesh : candidate.object->GetComponent<MeshComponent>()->meshes) {
            for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
                // World-space triangles keep hit distances comparable across different object scales.
                const glm::vec3 a(model * glm::vec4(mesh.vertices[mesh.indices[i]].Position, 1));
                const glm::vec3 b(model * glm::vec4(mesh.vertices[mesh.indices[i + 1]].Position, 1));
                const glm::vec3 c(model * glm::vec4(mesh.vertices[mesh.indices[i + 2]].Position, 1));
                const glm::vec3 edge1 = b - a, edge2 = c - a;
                const glm::vec3 p = glm::cross(ray, edge2);
                const float determinant = glm::dot(edge1, p);
                if (std::abs(determinant) < 1e-8f) continue;
                const glm::vec3 offset = origin - a;
                const float u = glm::dot(offset, p) / determinant;
                if (u < 0 || u > 1) continue;
                const glm::vec3 q = glm::cross(offset, edge1);
                const float v = glm::dot(ray, q) / determinant;
                if (v < 0 || u + v > 1) continue;
                const float distance = glm::dot(edge2, q) / determinant;
                if (distance >= 0 && distance < closest) { closest = distance; nearest = candidate.object; }
            }
        }
    }
    return nearest;
}
