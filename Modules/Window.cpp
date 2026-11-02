#include "Window.h"
#include "Log.h"
#include "Engine.h"
#include <glad/glad.h>
#include <algorithm>

Window::Window() : Module()
{
	window = NULL;
	name = "window";
}

// Destructor
Window::~Window()
{
}

// Called before render is available
bool Window::Awake()
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if (SDL_Init(SDL_INIT_VIDEO) != true)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		// Create window
		//L05 TODO 6: Get the values from the config file
		Uint32 flags = 0;
		bool fullscreen = configParameters.child("fullscreen").attribute("value").as_bool();
		bool borderless = configParameters.child("borderless").attribute("value").as_bool();
		bool resizable = configParameters.child("resizable").attribute("value").as_bool();
		bool fullscreen_window = configParameters.child("fullscreen_window").attribute("value").as_bool();

		//TODO Get the values from the config file
		width = configParameters.child("resolution").attribute("width").as_int(1280);
		height = configParameters.child("resolution").attribute("height").as_int(720);
		scale = configParameters.child("resolution").attribute("scale").as_int(1);
        if (scale < 1) scale = 1;
        SDL_Rect usable{};
        if (SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable)) {
            width = std::min(std::max(width, 640), usable.w);
            height = std::min(std::max(height, 480), std::max(480, usable.h - 40));
        }

		if (fullscreen == true)        flags |= SDL_WINDOW_FULLSCREEN;
		if (borderless == true)        flags |= SDL_WINDOW_BORDERLESS;
		if (resizable == true)         flags |= SDL_WINDOW_RESIZABLE;

		flags |= SDL_WINDOW_OPENGL;

		window = SDL_CreateWindow("Game Engine", width, height, flags);

		if (window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			context = SDL_GL_CreateContext(window);
			if (!context) {
				LOG("OpenGL context creation failed: %s", SDL_GetError());
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GameEngine - OpenGL 3.3 required",
                    "Your graphics driver could not create an OpenGL 3.3 Core context.\nInstall the graphics driver from Intel, AMD or NVIDIA and try again.\nThis version requires Windows 10/11 x64 and an OpenGL 3.3 compatible GPU.", window);
				return false;
			}

			if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
				LOG("OpenGL loader initialization failed");
				return false;
			}
			if (!GLAD_GL_VERSION_3_3) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GameEngine", "OpenGL 3.3 or newer is required.", window);
                return false;
            }
            LOG("OpenGL: %s; GPU: %s", glGetString(GL_VERSION), glGetString(GL_RENDERER));

			if (fullscreen_window == true)
			{
				SDL_SetWindowFullscreenMode(window, nullptr); // use desktop resolution
				SDL_SetWindowFullscreen(window, true);
			}
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			SDL_ShowWindow(window);
		}
	}

	return ret;
}

// Called before quitting
bool Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	if (context != NULL)
	{
		SDL_GL_DestroyContext(context);
		context = nullptr;
	}
	if (window != NULL)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	// Quit SDL subsystems
	SDL_Quit();
	return true;
}

// Set new window title
void Window::SetTitle(const char* new_title)
{
	SDL_SetWindowTitle(window, new_title);
}

void Window::GetWindowSize(int& width, int& height) const
{
	width = this->width;
	height = this->height;
}

int Window::GetScale() const
{
	return scale;
}
