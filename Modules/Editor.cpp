#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#include "Editor.h"
#include "Scene.h"
#include "Engine.h"
#include "Window.h"
#include "Log.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <IL/il.h>
#include <assimp/version.h>
#include <algorithm>

namespace {
constexpr ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
void Place(float x, float y, float w, float h) {
    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(std::max(w, 1.0f), std::max(h, 1.0f)));
}
void Link(const char* label, const char* url) {
    if (ImGui::Button(label) && !SDL_OpenURL(url)) LOG("Cannot open link: %s", SDL_GetError());
}
}
bool Editor::Init() {
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    style.WindowRounding = 0.0f; style.FrameRounding = 3.0f;
    style.WindowPadding = ImVec2(10, 10); style.ItemSpacing = ImVec2(8, 6);
    const float scale = SDL_GetWindowDisplayScale(Engine::GetInstance().window->window);
    if (scale > 0) { style.FontScaleDpi = scale; style.ScaleAllSizes(scale); }
    if (!ImGui_ImplSDL3_InitForOpenGL(Engine::GetInstance().window->window, Engine::GetInstance().window->context)) return false;
    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) return false;
    LOG("Editor initialized: Dear ImGui %s", IMGUI_VERSION);
    return true;
}
void Editor::ProcessEvent(const SDL_Event& event) {
    if (ImGui::GetCurrentContext()) ImGui_ImplSDL3_ProcessEvent(&event);
}
bool Editor::ResizeTarget(int width, int height) {
    if (width == targetWidth && height == targetHeight && framebuffer) return true;
    if (!framebuffer) {
        glGenFramebuffers(1, &framebuffer); glGenTextures(1, &colorTexture); glGenRenderbuffers(1, &depthBuffer);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    const bool valid = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); glBindTexture(GL_TEXTURE_2D, 0); glBindRenderbuffer(GL_RENDERBUFFER, 0);
    if (!valid) { LOG("Scene framebuffer is incomplete"); return false; }
    targetWidth = width; targetHeight = height;
    return true;
}
void Editor::Hierarchy(GameObject& object, Scene& scene) {
    ImGui::PushID(&object);
    auto flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
    if (scene.GetSelection() == &object) flags |= ImGuiTreeNodeFlags_Selected;
    if (object.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (revealSelection) {
        for (auto* ancestor = scene.GetSelection(); ancestor; ancestor = ancestor->parent) {
            if (ancestor == &object) { ImGui::SetNextItemOpen(true); break; }
        }
    }
    const bool open = ImGui::TreeNodeEx("node", flags, "%s", object.name.c_str());
    if (revealSelection && scene.GetSelection() == &object) ImGui::SetScrollHereY();
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        scene.Select(&object);
        transformRejected = false;
    }
    if (&object != &scene.root && ImGui::BeginPopupContextItem("Object actions")) {
        if (ImGui::MenuItem("Delete", "Del")) deleteRequest = &object;
        ImGui::EndPopup();
    }
    if (open) { for (auto& child : object.children) Hierarchy(*child, scene); ImGui::TreePop(); }
    ImGui::PopID();
}
void Editor::Inspector(GameObject* selectedObject) {
	if (selectedObject != nullptr) {
		// Mostrar y editar el nombre del GameObject
		char nameBuffer[128];
		strncpy_s(nameBuffer, sizeof(nameBuffer), selectedObject->name.c_str(), _TRUNCATE);
		if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
			selectedObject->name = nameBuffer;
		}

		ImGui::Separator();

		// Transform
		if (selectedObject->GetComponent<TransformComponent>() != nullptr) {
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::DragFloat3("Position", &selectedObject->GetComponent<TransformComponent>()->position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &selectedObject->GetComponent<TransformComponent>()->rotation.x, 0.5f);
				ImGui::DragFloat3("Scale", &selectedObject->GetComponent<TransformComponent>()->scale.x, 0.05f);
			}
		}

		
        auto* meshComponent = selectedObject->GetComponent<MeshComponent>();
        if (meshComponent && ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
            size_t vertices = 0, indices = 0;
            for (const auto& mesh : meshComponent->meshes) {
                vertices += mesh.vertices.size();
                indices += mesh.indices.size();
            }
            ImGui::Text("Vertices: %zu", vertices);
            ImGui::Text("Indices: %zu", indices);
            ImGui::Text("Triangles: %zu", indices / 3);
            if (vertices == 0) ImGui::TextWrapped("Select a mesh child to inspect its geometry.");
            ImGui::BeginDisabled(vertices == 0);
            ImGui::Checkbox("Vertex normals", &meshComponent->showVertexNormals);
            ImGui::Checkbox("Face normals", &meshComponent->showFaceNormals);
            if (meshComponent->showVertexNormals || meshComponent->showFaceNormals)
                ImGui::DragFloat("Normal length", &meshComponent->normalLength, 0.01f, 0.01f, 10.0f);
            ImGui::EndDisabled();
        }
		TextureComponent* texComp = selectedObject->GetComponent<TextureComponent>();
		if (texComp != nullptr) {
			if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("Texture ID: %d", texComp->textureID);
				ImGui::TextWrapped("Path: %s", texComp->filePath.c_str());
                ImGui::Text("Size: %d x %d pixels", texComp->width, texComp->height);
                if (ImGui::Checkbox("Checker texture", &texComp->showChecker)) {
                    const bool checker = texComp->showChecker;
                    std::function<void(GameObject&)> apply = [&](GameObject& object) {
                        if (auto* texture = object.GetComponent<TextureComponent>()) texture->showChecker = checker;
                        for (auto& child : object.children) apply(*child);
                    };
                    apply(*selectedObject);
                }
			}
		}

	}
	else {
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No GameObject selected.");
	}

}
void Editor::Configuration(Scene& scene) {
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::PlotLines("##FPS", fps.data(), static_cast<int>(fps.size()), fpsOffset, "Frames per second", 0, FLT_MAX, ImVec2(-1, 80));
    if (ImGui::CollapsingHeader("Engine", ImGuiTreeNodeFlags_DefaultOpen)) {
        int limit = Engine::GetInstance().GetFrameLimit();
        if (ImGui::SliderInt("FPS limit", &limit, 0, 300)) Engine::GetInstance().SetFrameLimit(limit);
        ImGui::TextDisabled("0 = unlimited");
    }
    if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        int interval = 0; SDL_GL_GetSwapInterval(&interval); bool vsync = interval != 0;
        if (ImGui::Checkbox("VSync", &vsync) && !SDL_GL_SetSwapInterval(vsync ? 1 : 0)) LOG("VSync: %s", SDL_GetError());
        ImGui::Checkbox("Depth test", &scene.depthTest); ImGui::Checkbox("Wireframe", &scene.wireframe);
        ImGui::Checkbox("Grid", &scene.showGrid); ImGui::ColorEdit3("Background", &scene.background.x);
    }
    if (ImGui::CollapsingHeader("Window")) {
        auto* window = Engine::GetInstance().window->window;
        int size[2]; SDL_GetWindowSize(window, &size[0], &size[1]);
        if (ImGui::InputInt2("Size", size, ImGuiInputTextFlags_EnterReturnsTrue))
            SDL_SetWindowSize(window, std::max(size[0], 800), std::max(size[1], 600));
        const auto flags = SDL_GetWindowFlags(window);
        bool fullscreen = flags & SDL_WINDOW_FULLSCREEN, borderless = flags & SDL_WINDOW_BORDERLESS;
        bool resizable = flags & SDL_WINDOW_RESIZABLE;
        if (ImGui::Checkbox("Fullscreen", &fullscreen)) SDL_SetWindowFullscreen(window, fullscreen);
        if (ImGui::Checkbox("Borderless", &borderless)) SDL_SetWindowBordered(window, !borderless);
        if (ImGui::Checkbox("Resizable", &resizable)) SDL_SetWindowResizable(window, resizable);
    }
    if (ImGui::CollapsingHeader("Input / Camera")) {
        ImGui::DragFloat("Move speed", &camera.moveSpeed, 0.1f, 0.1f, 100.0f);
        ImGui::DragFloat("Look sensitivity", &camera.sensitivity, 0.01f, 0.01f, 2.0f);
        ImGui::DragFloat("Zoom speed", &camera.zoomSensitivity, 0.1f, 0.1f, 100.0f);
        ImGui::TextWrapped("RMB + WASD: move / look. Shift: double speed. Alt + LMB: orbit selection. Wheel: zoom. F: focus.");
    }
    if (ImGui::CollapsingHeader("Hardware / Memory / Libraries")) {
        PROCESS_MEMORY_COUNTERS memory{}; memory.cb = sizeof(memory);
        if (GetProcessMemoryInfo(GetCurrentProcess(), &memory, sizeof(memory)))
            ImGui::Text("Process RAM: %.1f MiB", memory.WorkingSetSize / (1024.0 * 1024.0));
        ImGui::Text("System RAM: %d MiB", SDL_GetSystemRAM());
        ImGui::Text("CPU logical cores: %d", SDL_GetNumLogicalCPUCores());
        ImGui::TextWrapped("GPU: %s", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        ImGui::TextWrapped("OpenGL: %s", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        const int version = SDL_GetVersion();
        ImGui::Text("SDL %d.%d.%d", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
        ImGui::Text("DevIL %s", ilGetString(IL_VERSION_NUM));
        ImGui::Text("Assimp %u.%u.%u", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionPatch());
        ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
    }
}
void Editor::About() {
    ImGui::TextUnformatted("GameEngine 1.0"); ImGui::Separator();
    ImGui::TextUnformatted("Hugo Escobar (Hugg_ESC)");
    Link("Hugo on GitHub", "https://github.com/HuggoESC");
    ImGui::TextUnformatted("Alejandro Rodriguez (Alejandrorg5)");
    Link("Alejandro on GitHub", "https://github.com/Alejandrorg5");
    ImGui::Separator();
    ImGui::TextWrapped("Additional models: Kenney Car Kit and KayKit Furniture Bits (Kay Lousberg), CC0.");
    ImGui::TextWrapped("Libraries: SDL3, OpenGL, GLAD, GLM, Dear ImGui, ImGuizmo, Assimp, DevIL and pugixml.");
    ImGui::TextWrapped("MIT License. Copyright (c) 2026 Hugo Escobar and Alejandro Rodriguez.");
    ImGui::TextWrapped("Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the Software), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:");
    ImGui::TextWrapped("The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.");
    ImGui::TextWrapped("THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");
}
void Editor::Draw(Scene& scene, float dt) {
    ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplSDL3_NewFrame(); ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    fps[fpsOffset] = dt > 0 ? 1000.0f / dt : 0; fpsOffset = (fpsOffset + 1) % static_cast<int>(fps.size());
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { SDL_Event quit{}; quit.type = SDL_EVENT_QUIT; SDL_PushEvent(&quit); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Scene", nullptr, &showScene); ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &showInspector); ImGui::MenuItem("Console", nullptr, &showConsole);
            ImGui::MenuItem("Configuration", nullptr, &showConfiguration); ImGui::MenuItem("About", nullptr, &showAbout);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Cube")) scene.CreatePrimitive(Scene::Primitive::Cube);
            if (ImGui::MenuItem("Sphere")) scene.CreatePrimitive(Scene::Primitive::Sphere);
            if (ImGui::MenuItem("Pyramid")) scene.CreatePrimitive(Scene::Primitive::Pyramid);
            if (ImGui::MenuItem("Cylinder")) scene.CreatePrimitive(Scene::Primitive::Cylinder);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Documentation")) SDL_OpenURL("https://github.com/Alejandrorg5/GameEngine/tree/main/docs");
            if (ImGui::MenuItem("Report a bug")) SDL_OpenURL("https://github.com/Alejandrorg5/GameEngine/issues");
            if (ImGui::MenuItem("Latest release")) SDL_OpenURL("https://github.com/Alejandrorg5/GameEngine/releases");
            ImGui::MenuItem("About", nullptr, &showAbout);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    const auto* viewport = ImGui::GetMainViewport();
    const float menuHeight = ImGui::GetFrameHeight();
    const float x = viewport->Pos.x, y = viewport->Pos.y + menuHeight;
    const float width = viewport->Size.x, height = viewport->Size.y - menuHeight;
    const float left = showHierarchy ? std::min(280.0f, width * 0.20f) : 0;
    const float right = showInspector ? std::min(360.0f, width * 0.26f) : 0;
    const float bottom = showConsole ? std::min(210.0f, height * 0.25f) : 0;
    if (showHierarchy) {
        Place(x, y, left, height);
        if (ImGui::Begin("Hierarchy", &showHierarchy, panelFlags)) {
            const bool canDelete = scene.GetSelection() && scene.GetSelection() != &scene.root;
            ImGui::BeginDisabled(!canDelete || ImGuizmo::IsUsing());
            if (ImGui::Button("Delete selected")) deleteRequest = scene.GetSelection();
            ImGui::EndDisabled();
            if (ImGui::IsWindowFocused() && !ImGui::GetIO().WantTextInput && !ImGuizmo::IsUsing()
                && ImGui::IsKeyPressed(ImGuiKey_Delete, false) && canDelete) deleteRequest = scene.GetSelection();
            ImGui::Separator();
            Hierarchy(scene.root, scene);
            revealSelection = false;
        }
        ImGui::End();
    }
    if (showInspector) {
        Place(x + width - right, y, right, height);
        if (ImGui::Begin("Inspector", &showInspector, panelFlags)) Inspector(scene.GetSelection());
        ImGui::End();
    }
    if (showConsole) {
        Place(x + left, y + height - bottom, width - left - right, bottom);
        if (ImGui::Begin("Console", &showConsole, panelFlags)) {
            if (ImGui::Button("Clear")) ClearLogMessages();
            ImGui::SameLine(); ImGui::TextUnformatted("Import, initialization and errors");
            if (ImGui::BeginChild("Messages", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
                const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();
                const auto messages = GetLogMessages();
                ImGuiListClipper clipper; clipper.Begin(static_cast<int>(messages.size()));
                while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) ImGui::TextUnformatted(messages[i].c_str());
                if (atBottom) ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
    if (showScene) {
        Place(x + left, y, width - left - right, height - bottom);
        if (ImGui::Begin("Scene", &showScene, panelFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
            const char* operations[] = {"Move (W)", "Rotate (E)", "Scale (R)"};
            ImGui::BeginDisabled(ImGuizmo::IsUsing());
            for (int i = 0; i < 3; ++i) {
                if (i) ImGui::SameLine();
                if (ImGui::RadioButton(operations[i], gizmoOperation == i)) gizmoOperation = i;
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(gizmoOperation == 2);
            if (ImGui::RadioButton("Local", gizmoLocal || gizmoOperation == 2)) gizmoLocal = true;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", !gizmoLocal && gizmoOperation != 2)) gizmoLocal = false;
            ImGui::EndDisabled();
            ImGui::EndDisabled();
            if (transformRejected) ImGui::TextWrapped("This parent transform cannot represent the edit. Select the model's parent instead.");
            const ImVec2 size = ImGui::GetContentRegionAvail();
            if (size.x >= 1 && size.y >= 1 && ResizeTarget(static_cast<int>(size.x), static_cast<int>(size.y))) {
                const ImVec2 imageStart = ImGui::GetCursorScreenPos();
                ImGui::Image(static_cast<ImTextureID>(colorTexture), size, ImVec2(0, 1), ImVec2(1, 0));
                const bool hovered = ImGui::IsItemHovered();
                const auto& io = ImGui::GetIO();
                const bool keyboard = (hovered || ImGui::IsWindowFocused()) && !io.WantTextInput
                    && !ImGui::IsAnyItemActive() && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId);
                const bool navigating = io.KeyAlt || ImGui::IsMouseDown(ImGuiMouseButton_Right);
                if (keyboard && !navigating && !ImGuizmo::IsUsing()) {
                    if (ImGui::IsKeyPressed(ImGuiKey_W, false)) gizmoOperation = 0;
                    if (ImGui::IsKeyPressed(ImGuiKey_E, false)) gizmoOperation = 1;
                    if (ImGui::IsKeyPressed(ImGuiKey_R, false)) gizmoOperation = 2;
                    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) deleteRequest = scene.GetSelection();
                }
                // A gizmo drag owns the mouse until release; camera shortcuts remain independent.
                const bool usingGizmo = ImGuizmo::IsUsing();
                camera.Update(std::clamp(dt / 1000.0f, 0.0f, 0.1f), scene.GetSelection(), hovered && !usingGizmo, keyboard && !usingGizmo);
                const float aspect = static_cast<float>(targetWidth) / targetHeight;
                const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect,
                    std::max(0.01f, camera.distance / 10000.0f), std::max(100.0f, camera.distance * 4.0f));
                const glm::mat4 view = camera.GetViewMatrix();
                bool overGizmo = false;
                if (auto* selected = scene.GetSelection(); selected && selected != &scene.root) {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
                    ImGuizmo::SetRect(imageStart.x, imageStart.y, size.x, size.y);
                    ImGuizmo::Enable(!navigating && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId));
                    glm::mat4 model = selected->GetGlobalMatrix();
                    const ImGuizmo::OPERATION operation = gizmoOperation == 0 ? ImGuizmo::TRANSLATE :
                        gizmoOperation == 1 ? ImGuizmo::ROTATE : ImGuizmo::SCALE;
                    ImGuizmo::PushID(selected);
                    ImGui::GetWindowDrawList()->PushClipRect(imageStart, ImVec2(imageStart.x + size.x, imageStart.y + size.y), true);
                    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), operation,
                        (gizmoLocal || gizmoOperation == 2) ? ImGuizmo::LOCAL : ImGuizmo::WORLD, glm::value_ptr(model))) {
                        transformRejected = !selected->SetGlobalMatrix(model);
                    }
                    ImGui::GetWindowDrawList()->PopClipRect();
                    overGizmo = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
                    ImGuizmo::PopID();
                }
                if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !navigating && !overGizmo && !usingGizmo) {
                    const float nx = 2.0f * (io.MousePos.x - imageStart.x) / size.x - 1.0f;
                    const float ny = 1.0f - 2.0f * (io.MousePos.y - imageStart.y) / size.y;
                    const auto inverse = glm::inverse(projection * view);
                    const glm::vec4 nearPoint = inverse * glm::vec4(nx, ny, -1, 1);
                    const glm::vec4 farPoint = inverse * glm::vec4(nx, ny, 1, 1);
                    const glm::vec3 origin = glm::vec3(nearPoint) / nearPoint.w;
                    const glm::vec3 end = glm::vec3(farPoint) / farPoint.w;
                    scene.Select(scene.GetModelRoot(scene.Raycast(origin, end - origin, glm::length(end - origin))));
                    revealSelection = true;
                    transformRejected = false;
                }
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); glViewport(0, 0, targetWidth, targetHeight);
                scene.RenderScene(view, projection, dt);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }
        ImGui::End();
    }
    if (showConfiguration) {
        ImGui::SetNextWindowSize(ImVec2(470, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Configuration", &showConfiguration)) Configuration(scene);
        ImGui::End();
    }
    if (showAbout) {
        ImGui::SetNextWindowSize(ImVec2(580, 520), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("About", &showAbout)) About();
        ImGui::End();
    }
    DeleteDialog(scene);
    ImGui::Render();
    int w = 0, h = 0; SDL_GetWindowSizeInPixels(Engine::GetInstance().window->window, &w, &h);
    glViewport(0, 0, w, h); glClearColor(0.08f, 0.09f, 0.10f, 1); glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(Engine::GetInstance().window->window);
}
void Editor::CleanUp() {
    glDeleteRenderbuffers(1, &depthBuffer); glDeleteTextures(1, &colorTexture); glDeleteFramebuffers(1, &framebuffer);
    depthBuffer = colorTexture = framebuffer = 0;
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplSDL3_Shutdown(); ImGui::DestroyContext();
}


void Editor::DeleteDialog(Scene& scene) {
    if (deleteRequest) {
        if (deleteRequest != &scene.root) {
            if (deleteRequest->children.empty()) scene.DeleteObject(deleteRequest);
            else { deleteTarget = deleteRequest; ImGui::OpenPopup("Delete object?"); }
        }
        deleteRequest = nullptr;
    }
    if (ImGui::BeginPopupModal("Delete object?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (deleteTarget) {
            ImGui::Text("Delete '%s' and all its children?", deleteTarget->name.c_str());
            ImGui::TextUnformatted("Only the scene instance is removed. Source files are kept.");
            if (ImGui::Button("Delete")) {
                scene.DeleteObject(deleteTarget); deleteTarget = nullptr;
                transformRejected = false; ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                deleteTarget = nullptr; ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}
