
#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>

class Phy7_Solver : public Scene
{
public:
	Phy7_Solver(const std::string& friendly_name)
		: Scene(friendly_name)
		, m_StackHeight(6)
	{}

	int m_StackHeight;
	virtual void OnInitializeScene() override
	{
		//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		//SOLVER EXAMPLE -> Pyramid of cubes stacked on top of eachother
		// Note: Also, i've just noticed we have only ever done cube's not cuboids.. 
		//       so, to prove to any non-believes, try turning the pyramid of cubes
		//       into rectangles :)
		const float width_scalar = 1.0f; 
		const float height_scalar = 1.0f; 

		for (int y = 0; y < m_StackHeight; ++y)
		{
			for (int x = 0; x <= y; ++x)
			{
				Vector4 color = CommonUtils::GenColor(y * 0.2f, 1.0f);
				GameObject* cube = CommonUtils::BuildCuboidObject(
					"",
					Vector3((x * 1.1f - y * 0.5f) * width_scalar, (0.5f + float(m_StackHeight - 1) - y) * height_scalar, -0.5f),
					Vector3(width_scalar * 0.5f, height_scalar * 0.5f, 0.5f),
					true,
					1.f,
					true,
					true,
					color);
				cube->Physics()->SetElasticity(0.0f); //No elasticity (Little cheaty)
				cube->Physics()->SetFriction(1.0f);

				this->AddGameObject(cube);
			}
		}
		

	}


	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    Stack Height : %2d ([1]/[2] to change)", m_StackHeight);

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
		{
			m_StackHeight++;
			SceneManager::Instance()->JumpToScene(SceneManager::Instance()->GetCurrentSceneIndex());
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
		{
			m_StackHeight = max(m_StackHeight - 1, 1);
			SceneManager::Instance()->JumpToScene(SceneManager::Instance()->GetCurrentSceneIndex());
		}
	}
};