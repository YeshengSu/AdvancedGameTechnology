#include "SceneManager.h"
#include "PhysicsEngine.h"
#include "CommonMeshes.h"
#include <nclgl\NCLDebug.h>
#include "GraphicsPipeline.h"

SceneManager::SceneManager() 
	: m_SceneIdx(NULL)
{
	CommonMeshes::InitializeMeshes();
}

SceneManager::~SceneManager()
{
	NCLLOG("[SceneManager] Closing scene manager");
	m_SceneIdx = 0;
	for (Scene* scene : m_vpAllScenes)
	{
		if (scene != scene)
		{
			scene->OnCleanupScene();
			delete scene;
		}
	}
	m_vpAllScenes.clear();

	CommonMeshes::ReleaseMeshes();
}


void SceneManager::EnqueueScene(Scene* scene)
{
	if (scene == NULL)
	{
		NCLERROR("Attempting to enqueue NULL scene");
		return;
	}

	m_vpAllScenes.push_back(scene);
	NCLLOG("[SceneManager] - Enqueued scene: \"%s\"", scene->GetSceneName().c_str());

	//If this was the first scene, activate it immediately
	if (m_vpAllScenes.size() == 1)
		JumpToScene(0);
	else
		Window::GetWindow().SetWindowTitle("NCLTech - [%d/%d] %s", m_SceneIdx + 1, m_vpAllScenes.size(), scene->GetSceneName().c_str());
}

void SceneManager::JumpToScene()
{
	JumpToScene((m_SceneIdx + 1) % m_vpAllScenes.size());
}

void SceneManager::JumpToScene(int idx)
{
	if (idx < 0 || idx >= (int)m_vpAllScenes.size())
	{
		NCLERROR("Invalid Scene Index: %d", idx);
		return;
	}

	//Clear up old scene
	if (scene)
	{
		NCLLOG("[SceneManager] - Exiting scene -");
		scene->OnCleanupScene();
		PhysicsEngine::Instance()->RemoveAllPhysicsObjects();	
	}

	m_SceneIdx = idx;
	scene = m_vpAllScenes[idx];
	NCLLOG("");

	//Initialize new scene
	PhysicsEngine::Instance()->SetDefaults();
	GraphicsPipeline::Instance()->InitializeDefaults();
	scene->OnInitializeScene();
	Window::GetWindow().SetWindowTitle("NCLTech - [%d/%d] %s", idx + 1, m_vpAllScenes.size(), scene->GetSceneName().c_str());
	NCLLOG("[SceneManager] - Scene switched to: \"%s\"", scene->GetSceneName().c_str());
}

void SceneManager::JumpToScene(const std::string& friendly_name)
{
	bool found = false;
	uint idx = 0;
	for (uint i = 0; found == false && i < m_vpAllScenes.size(); ++i)
	{
		if (m_vpAllScenes[i]->GetSceneName() == friendly_name)
		{
			found = true;
			idx = i;
			break;
		}
	}

	if (found)
	{
		JumpToScene(idx);
	}
	else
	{
		NCLERROR("Unknown Scene Alias: \"%s\"", friendly_name.c_str());
	}
}