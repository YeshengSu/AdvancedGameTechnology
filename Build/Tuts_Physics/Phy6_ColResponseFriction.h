
#pragma once
#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>

class Phy6_ColResponseFriction : public Scene
{
public:
	Phy6_ColResponseFriction(const std::string& friendly_name)
		: Scene(friendly_name)
		, m_RampAngle(20.0f)
	{}


	float m_RampAngle;

	virtual void OnInitializeScene() override
	{
		PhysicsEngine::Instance()->SetDampingFactor(1.0f);

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

		//FRICTION EXAMPLE -> Cubes sliding down a ramp
		{
			//Create Ramp
			GameObject* ramp = CommonUtils::BuildCuboidObject(
				"Ramp",
				Vector3(5.0f, 5.5f, -5.0f),
				Vector3(5.0f, 0.5f, 4.0f),
				true,
				0.0f,
				true,
				false,
				Vector4(1.0f, 0.7f, 1.0f, 1.0f));
			ramp->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), m_RampAngle));
			ramp->Physics()->SetFriction(1.0f);
			this->AddGameObject(ramp);

			Vector3 start_off = ramp->Physics()->GetPosition() + Vector3(
				cosf((float)DegToRad(m_RampAngle)) * 4.0f - sinf((float)DegToRad(m_RampAngle)),
				sinf((float)DegToRad(m_RampAngle)) * 4.0f + cosf((float)DegToRad(m_RampAngle)),
				0.0f
			);

			//Create Cubes to roll on ramp
			for (int i = 0; i <= 5; ++i)
			{
				Vector4 color = Vector4(i * 0.25f, 0.7f, (2 - i) * 0.25f, 1.0f);
				GameObject* cube = CommonUtils::BuildCuboidObject(
					"",
					start_off + Vector3(0.0f, 0.0f, -2.5f + i * 1.1f),
					Vector3(0.5f, 0.5f, 0.5f),
					true,
					1.f,
					true,
					true,
					color);
				cube->Physics()->SetElasticity(0.0f);
				cube->Physics()->SetFriction(i * 0.2f);
				cube->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), m_RampAngle));

				//Initial push??
				//cube->Physics()->SetLinearVelocity(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 0.0f, 1.0f), 20.0f).ToMatrix3() * Vector3(-1.f, 0.f, 0.f));
				this->AddGameObject(cube);
			}
		}
	}


	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    Ramp Angle : %2.0f degrees ([1]/[2] to change)", m_RampAngle);

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
		{
			m_RampAngle = min(m_RampAngle + 10.0f, 70.0f);
			SceneManager::Instance()->JumpToScene(SceneManager::Instance()->GetCurrentSceneIndex());
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
		{
			m_RampAngle = max(m_RampAngle - 10.0f, 10.0f);
			SceneManager::Instance()->JumpToScene(SceneManager::Instance()->GetCurrentSceneIndex());
		}
	}
};