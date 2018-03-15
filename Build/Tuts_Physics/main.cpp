#include <nclgl\Window.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>

#include "Phy2_Integration.h"
#include "Phy3_Constraints.h"
#include "Phy4_ColDetection.h"
#include "Phy4_AiCallbacks.h"
#include "Phy5_ColManifolds.h"
#include "Phy6_ColResponseElasticity.h"
#include "Phy6_ColResponseFriction.h"
#include "Phy7_Solver.h"

PerfTimer timer_total, timer_physics;

bool draw_debug = true;
bool draw_performance = false;

void Quit(bool error = false, const string &reason = "");

void Initialize()
{
	//Initialise the Window
	if (!Window::Initialise("Game Technologies - Collision Resolution", 1280, 800, false))
		Quit(true, "Window failed to initialise!");

	//Initialise the PhysicsEngine
	PhysicsEngine::Instance();

	//Initialize Renderer
	GraphicsPipeline::Instance();
	SceneManager::Instance();	//Loads CommonMeshes in here (So everything after this can use them globally e.g. our scenes)

								//Enqueue All Scenes
								// - Add any new scenes you want here =D
	SceneManager::Instance()->EnqueueScene(new Phy2_Integration("Physics Tut #2 - Integration"));
	SceneManager::Instance()->EnqueueScene(new Phy3_Constraints("Physics Tut #3 - Distance Constraints"));
	SceneManager::Instance()->EnqueueScene(new Phy4_ColDetection("Physics Tut #4 - Collision Detection"));
	SceneManager::Instance()->EnqueueScene(new Phy4_AiCallbacks("Physics Tut #4 - Collision Detection [Bonus]"));
	SceneManager::Instance()->EnqueueScene(new Phy5_ColManifolds("Physics Tut #5 - Collision Manifolds"));
	SceneManager::Instance()->EnqueueScene(new Phy6_ColResponseElasticity("Physics Tut #6 - Collision Response [Elasticity]"));
	SceneManager::Instance()->EnqueueScene(new Phy6_ColResponseFriction("Physics Tut #6 - Collision Response [Friction]"));
	SceneManager::Instance()->EnqueueScene(new Phy7_Solver("Physics Tut #7 - Global Solver"));
}









//------------------------------------
//---------Default main loop----------
//------------------------------------
// With GameTech, everything is put into 
// little "Scene" class's which are self contained
// programs with their own game objects/logic.
//
// So everything you want to do in renderer/main.cpp
// should now be able to be done inside a class object.
//
// For an example on how to set up your test Scene's,
// see one of the PhyX_xxxx tutorial scenes. =]



void Quit(bool error, const string &reason) {
	//Release Singletons
	SceneManager::Release();
	GraphicsPipeline::Release();
	PhysicsEngine::Release();
	Window::Destroy();

	//Show console reason before exit
	if (error) {
		std::cout << reason << std::endl;
		system("PAUSE");
		exit(-1);
	}
}

void PrintStatusEntries()
{
	const Vector4 status_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	const Vector4 status_color_debug = Vector4(1.0f, 0.6f, 1.0f, 1.0f);
	const Vector4 status_color_performance = Vector4(1.0f, 0.6f, 0.6f, 1.0f);

	//Print Current Scene Name
	NCLDebug::AddStatusEntry(status_color, "[%d/%d]: %s ([T]/[Y] to cycle or [R] to reload)",
		SceneManager::Instance()->GetCurrentSceneIndex() + 1,
		SceneManager::Instance()->SceneCount(),
		SceneManager::Instance()->GetCurrentScene()->GetSceneName().c_str()
	);

	//Print Engine Options
	NCLDebug::AddStatusEntry(Vector4(0.8f, 1.0f, 0.8f, 1.0f), "    Physics: %s [P]", PhysicsEngine::Instance()->IsPaused() ? "Paused  " : "Enabled ");

	//Physics Debug Drawing options
	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
	NCLDebug::AddStatusEntry(status_color_debug, "--- Debug Draw  [G] ---");
	if (draw_debug)
	{		
		NCLDebug::AddStatusEntry(status_color_debug, "Constraints       : %s [Z] - Tut 3+", (drawFlags & DEBUGDRAW_FLAGS_CONSTRAINT) ? "Enabled " : "Disabled");
		NCLDebug::AddStatusEntry(status_color_debug, "Collision Normals : %s [X] - Tut 4+", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS) ? "Enabled " : "Disabled");
		NCLDebug::AddStatusEntry(status_color_debug, "Collision Volumes : %s [C] - Tut 4+", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES) ? "Enabled " : "Disabled");
		NCLDebug::AddStatusEntry(status_color_debug, "Manifolds         : %s [V] - Tut 5+", (drawFlags & DEBUGDRAW_FLAGS_MANIFOLD) ? "Enabled " : "Disabled");
		NCLDebug::AddStatusEntry(status_color_debug, "Bounding Radius   : %s [B] - Tut 6+", (drawFlags & DEBUGDRAW_FLAGS_BOUNDINGRADIUS) ? "Enabled " : "Disabled");
		NCLDebug::AddStatusEntry(status_color_debug, "");
	}

	//Physics performance measurements
	NCLDebug::AddStatusEntry(status_color_performance, "--- Performance [H] ---");
	if (draw_performance)
	{
		timer_total.PrintOutputToStatusEntry(status_color_performance, "Frame Total     :");
		timer_physics.PrintOutputToStatusEntry(status_color_performance, "Physics Total   :");
		PhysicsEngine::Instance()->PrintPerformanceTimers(status_color_performance);
		NCLDebug::AddStatusEntry(status_color_performance, "");
	}
}


void HandleKeyboardInputs()
{
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P))
	{
		PhysicsEngine::Instance()->SetPaused(!PhysicsEngine::Instance()->IsPaused());
		NCLLOG("[MAIN] - Physics %s", PhysicsEngine::Instance()->IsPaused() ? "paused" : "resumed");
	}

	uint sceneIdx = SceneManager::Instance()->GetCurrentSceneIndex();
	uint sceneMax = SceneManager::Instance()->SceneCount();
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Y))
		SceneManager::Instance()->JumpToScene((sceneIdx + 1) % sceneMax);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
		SceneManager::Instance()->JumpToScene((sceneIdx == 0 ? sceneMax : sceneIdx) - 1);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R))
		SceneManager::Instance()->JumpToScene(sceneIdx);


	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G)) draw_debug = !draw_debug;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_H)) draw_performance = !draw_performance;



	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Z))
		drawFlags ^= DEBUGDRAW_FLAGS_CONSTRAINT;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_X))
		drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_C))
		drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_V))
		drawFlags ^= DEBUGDRAW_FLAGS_MANIFOLD;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_B))
		drawFlags ^= DEBUGDRAW_FLAGS_BOUNDINGRADIUS;

	PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

}



int main()
{
	//Initialize our Window, Physics, Scenes etc
	Initialize();

	Window::GetWindow().GetTimer()->GetTimedMS();

	//Create main game-loop
	while (Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		//Start Timing
		float dt = Window::GetWindow().GetTimer()->GetTimedMS() * 0.001f;	//How many milliseconds since last update?
		timer_total.BeginTimingSection();

		//Print Status Entries
		PrintStatusEntries();

		//Handle Keyboard Inputs
		HandleKeyboardInputs();

		//Update Performance Timers (Show results every second)
		timer_total.UpdateRealElapsedTime(dt);
		timer_physics.UpdateRealElapsedTime(dt);


		//Update Scene
		SceneManager::Instance()->GetCurrentScene()->FireOnSceneUpdate(dt);

		//Update Physics
		timer_physics.BeginTimingSection();
		PhysicsEngine::Instance()->Update(dt);
		timer_physics.EndTimingSection();
		PhysicsEngine::Instance()->DebugRender();

		//Render Scene

		GraphicsPipeline::Instance()->UpdateScene(dt);
		GraphicsPipeline::Instance()->RenderScene();				 //Finish Timing
		timer_total.EndTimingSection();
	}

	//Cleanup
	Quit();
	return 0;
}