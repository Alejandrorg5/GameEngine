#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <filesystem>

#include "Engine.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
//#include "Textures.h"
//#include "Audio.h"
#include "Scene.h"
//#include "EntityManager.h"
//#include "Map.h"
//#include "Physics.h"
#include "Log.h"

// Constructor
Engine::Engine() {

    LOG("Constructor Engine::Engine");

    // L2: TODO 3: Measure the amount of ms that takes to execute the Engine constructor and LOG the result
    Timer timer = Timer();
    startupTime = Timer();
    frameTime = PerfTimer();
    lastSecFrameTime = PerfTimer();
    frames = 0;

    // L4: TODO 1: Add the EntityManager Module to the Engine

    // Modules
    window = std::make_shared<Window>();
    input = std::make_shared<Input>();
    render = std::make_shared<Render>();
    scene = std::make_shared<Scene>();

    // Ordered for awake / Start / Update
    // Reverse order of CleanUp
    AddModule(std::static_pointer_cast<Module>(window));
    AddModule(std::static_pointer_cast<Module>(input));
    AddModule(std::static_pointer_cast<Module>(scene));

    // Render last 
    AddModule(std::static_pointer_cast<Module>(render));

    // L2: TODO 3: Log the result of the timer
    LOG("Timer App Constructor: %f", timer.ReadMSec());
}

// Static method to get the instance of the Engine class, following the singleton pattern
Engine& Engine::GetInstance() {
    static Engine instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

void Engine::AddModule(std::shared_ptr<Module> module) {
    module->Init();
    moduleList.push_back(module);
}

// Called before render is available
bool Engine::Awake() {

    // L2: TODO 3: Measure the amount of ms that takes to execute the Awake and LOG the result
    Timer timer = Timer();

    LOG("Engine::Awake");

    //L05 TODO 2: Add the LoadConfig() method here
    if (!LoadConfig()) return false;
    // L05: TODO 3: Read the title from the config file and set the variable gameTitle, read targetFrameRate and set the variables
    gameTitle = configFile.child("config").child("engine").child("title").child_value();
    targetFrameRate = configFile.child("config").child("engine").child("targetFrameRate").attribute("value").as_int();

    //Iterates the module list and calls Awake on each module
    bool result = true;
    for (const auto& module : moduleList) {
        // L05: TODO 4: Call the LoadParameters function for each module
        module->LoadParameters(configFile.child("config").child(module.get()->name.c_str()));
        result = module->Awake();

        if (!result) {
            break;
        }
    }

    // L2: TODO 3: Log the result of the timer
    LOG("Timer App Awake(): %f", timer.ReadMSec());

    return result;
}

// Called before the first frame
bool Engine::Start() {

    // L2: TODO 3: Measure the amount of ms that takes to execute the Start() and LOG the result
    Timer timer = Timer();

    LOG("Engine::Start");

    //Iterates the module list and calls Start on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Start();
        if (!result) {
            break;
        }
    }

    // L2: TODO 3: Log the result of the timer
    LOG("Engine Start: %.2f ms", timer.ReadMSec());

    return result;
}

// Called each loop iteration
bool Engine::Update() {

    bool ret = true;
    PrepareUpdate();

    if (input->GetWindowEvent(WE_QUIT) == true)
        ret = false;

    if (ret == true)
        ret = PreUpdate();

    if (ret == true)
        ret = DoUpdate();

    if (ret == true)
        ret = PostUpdate();

    FinishUpdate();
    return ret;
}

// Called before quitting
bool Engine::CleanUp() {

    // L2: TODO 3: Measure the amount of ms that takes to execute the Start() and LOG the result
    const auto cleanupStart = std::chrono::steady_clock::now();

    LOG("Engine::CleanUp");

    //Iterates the module list and calls CleanUp on each module
    bool result = true;
    for (auto it = moduleList.rbegin(); it != moduleList.rend(); ++it) {
        result = (*it)->CleanUp() && result;
    }

    // L2: TODO 3: Log the result of the timer
    LOG("Engine cleanup: %.2f ms", std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cleanupStart).count());

    return result;
}

// ---------------------------------------------
void Engine::PrepareUpdate()
{
    frameTime.Start();
}

// ---------------------------------------------
void Engine::FinishUpdate()
{
    // L03: TODO 1: Cap the framerate of the gameloop
    double currentDt = frameTime.ReadMs();
    float maxFrameDuration = targetFrameRate > 0 ? 1000.0f / targetFrameRate : 0.0f;
    if (targetFrameRate > 0 && currentDt < maxFrameDuration) {
        const auto delay = static_cast<Uint64>((maxFrameDuration - currentDt) * 1000000.0);
        SDL_DelayPrecise(delay);
    }

    // L2: TODO 4: Calculate:

    // Amount of frames since startup
    frameCount++;

    // Amount of time since game start (use a low resolution timer)
    secondsSinceStartup = startupTime.ReadSec();

    // Amount of ms took the last update (dt)
    dt = (float)frameTime.ReadMs();

    // Amount of frames during the last second
    lastSecFrameCount++;

    // Average FPS for the whole game life
    if (lastSecFrameTime.ReadMs() > 1000) {
        lastSecFrameTime.Start();
        averageFps = (averageFps + lastSecFrameCount) / 2;
        framesPerSecond = lastSecFrameCount;
        lastSecFrameCount = 0;
    }

    // Shows the time measurements in the window title
    // check sprintf formats here https://cplusplus.com/reference/cstdio/printf/
    std::stringstream ss;
    ss << gameTitle << ": Av.FPS: " << std::fixed << std::setprecision(2) << averageFps
        << " Last sec frames: " << framesPerSecond
        << " Last dt: " << std::fixed << std::setprecision(3) << dt
        << " Time since startup: " << secondsSinceStartup
        << " Frame Count: " << frameCount;

    std::string titleStr = ss.str();

    window.get()->SetTitle(titleStr.c_str());
}


// Call modules before each loop iteration
bool Engine::PreUpdate()
{
    //Iterates the module list and calls PreUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PreUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules on each loop iteration
bool Engine::DoUpdate()
{
    //Iterates the module list and calls Update on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->Update(dt);
        if (!result) {
            break;
        }
    }

    return result;
}

// Call modules after each loop iteration
bool Engine::PostUpdate()
{
    //Iterates the module list and calls PostUpdate on each module
    bool result = true;
    for (const auto& module : moduleList) {
        result = module->PostUpdate();
        if (!result) {
            break;
        }
    }

    return result;
}

// Load config from XML file
bool Engine::LoadConfig()
{
    bool ret = true;

    // L05: TODO 2: Load config.xml file using load_file() method from the xml_document class
    // If the result is ok get the main node of the XML
    // else, log the error
    // check https://pugixml.org/docs/quickstart.html#loading

    pugi::xml_parse_result result = configFile.load_file(std::filesystem::u8path(Scene::AssetPath("config.xml")).c_str());
    if (result)
    {
        LOG("config.xml parsed without errors");
    }
    else
    {
        LOG("Error loading config.xml: %s", result.description());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GameEngine - Missing configuration",
            "Cannot read config.xml. Extract the complete ZIP before running GameEngine.exe; keep config.xml and Assets next to it.", nullptr);
        ret = false;
    }

    return ret;
}

