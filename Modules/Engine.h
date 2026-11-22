#pragma once

#include <memory>
#include <list>
class Module;
#include "Timer.h"
#include "PerfTimer.h"
#include "pugixml.hpp"

// Modules
class Window;
class Input;
class Render;
class Scene;

class Engine
{
public:

	// Public method to get the instance of the Singleton
	static Engine& GetInstance();

	//	
	void AddModule(std::shared_ptr<Module> module);

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	int GetFrameLimit() const { return targetFrameRate; }
    void SetFrameLimit(int value) { targetFrameRate = value < 0 ? 0 : value; }

	float GetDt() const {
		return dt;
	}

private:

	// Private constructor to prevent instantiation
	// Constructor
	Engine();

	// Delete copy constructor and assignment operator to prevent copying
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load config file
	bool LoadConfig();

	std::list<std::shared_ptr<Module>> moduleList;

public:

	enum EngineState
	{
		CREATE = 1,
		AWAKE,
		START,
		LOOP,
		CLEAN,
		FAIL,
		EXIT
	};

    Window& GetWindow() const { return *window; }
    Input& GetInput() const { return *input; }
    Render& GetRender() const { return *render; }
    Scene& GetScene() const { return *scene; }

private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Input> input;
    std::shared_ptr<Render> render;
    std::shared_ptr<Scene> scene;

	// Delta time
	float dt = 0.0f;
	//Frames since startup
	int frames;

	// Calculate timing measures
	// required variables are provided:
	Timer startupTime;
	PerfTimer frameTime;
	PerfTimer lastSecFrameTime;

	int frameCount = 0;
	int framesPerSecond = 0;
	int lastSecFrameCount = 0;

	float averageFps = 0.0f;
	int secondsSinceStartup = 0;

	//Maximun frame duration in miliseconds.
	int targetFrameRate = 60;

	std::string gameTitle = "Platformer Game";

	//L05 TODO 2: Declare a xml_document to load the config file
	pugi::xml_document configFile;

};