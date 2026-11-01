#include <glad/glad.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ImGui e integraciones provistas por vcpkg
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

unsigned int wWidth = 1920;
unsigned int wHeight = 1080;
SDL_Window* wdn;
SDL_GLContext ctx;

struct RGB {
	float r;
	float g;
	float b;
};

void Init_ImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(wdn, ctx);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void CleanUp() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DestroyContext(ctx);
	SDL_DestroyWindow(wdn);
	SDL_Quit();
}

void Show_Editor_UI() {

}

int Init() {
	if (!SDL_Init(SDL_INIT_VIDEO))
		return -1;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	wdn = SDL_CreateWindow("Engine 3D + vcpkg", wWidth, wHeight, flags);
	if (wdn == nullptr)
		return -1;

	ctx = SDL_GL_CreateContext(wdn);
	if (!ctx)
		return -1;

	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0)
		return -1;

	Init_ImGui();
	
	return 0;
}

RGB* GenerateChecker(int width, int height) {
	RGB* checkerTexels = new RGB[width * height];
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			float c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0)));
			checkerTexels[width * i + j] = { c, c, c };
		}
	}
	return checkerTexels;
}

int main(int argc, char* argv[]) {
	if (Init() != 0)
		return -1;

	static constexpr unsigned int tWidth = 512;
	static constexpr unsigned int tHeight = 256;

	RGB* checkerTexels = GenerateChecker(tWidth, tHeight);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_FLOAT, checkerTexels);
	delete[] checkerTexels;

	glViewport(0, 0, wWidth, wHeight);
	glClearColor(0.1f, 0.25f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Vértices del cubo 3D
	float cubeVertices[] = {
		// Frontal
		-0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		// Trasera
		-0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,   0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,   0.5f, 0.5f, 0.5f,   1.0f, 1.0f
	};

	GLuint cVBuff;
	glGenBuffers(1, &cVBuff);
	glBindBuffer(GL_ARRAY_BUFFER, cVBuff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	unsigned int cubeIndices[] = {
		0, 1, 2,  2, 3, 0,
		5, 4, 7,  7, 6, 5,
		4, 0, 3,  3, 7, 4,
		1, 5, 6,  6, 2, 1,
		4, 5, 1,  1, 0, 4,
		3, 2, 6,  6, 7, 3
	};

	GLuint iBuff;
	glGenBuffers(1, &iBuff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	bool running = true;
	Uint64 lastTime = SDL_GetTicks();

	while (running)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			ImGui_ImplSDL3_ProcessEvent(&e);

			if (e.type == SDL_EVENT_QUIT)
				running = false;
			else if (e.type == SDL_EVENT_WINDOW_RESIZED)
			{
				wWidth = e.window.data1;
				wHeight = e.window.data2;
				glViewport(0, 0, wWidth, wHeight);
			}
		}

		Uint64 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;
		static float angle = 0.0f;
		angle += glm::radians(50.0f) * deltaTime;

		// Nuevos frames de ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Begin("Engine Control Panel");
		ImGui::Text("FPS: %.1f", 1.0f / deltaTime);
		ImGui::End();

		// Renderizado 3D de la escena
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
		glLoadMatrixf(glm::value_ptr(projection));

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.5f, 1.0f, 0.0f));
		glm::mat4 mvp = view * model;
		glLoadMatrixf(glm::value_ptr(mvp));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindBuffer(GL_ARRAY_BUFFER, cVBuff);
		glVertexPointer(3, GL_FLOAT, sizeof(float) * 8, reinterpret_cast<void*>(0));
		glColorPointer(3, GL_FLOAT, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 3));
		glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 8, reinterpret_cast<void*>(sizeof(float) * 6));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuff);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, NULL);

		// Renderizado de ImGui sobre la escena
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(wdn);
	}

	CleanUp();

	return 0;
}