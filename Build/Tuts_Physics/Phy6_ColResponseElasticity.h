
#pragma once
#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>

class Phy6_ColResponseElasticity : public Scene
{
public:
	Phy6_ColResponseElasticity(const std::string& friendly_name)
		: Scene(friendly_name)
	{}


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

		//ELASTICITY EXAMPLE -> Balls bouncing on a pad
		// Note: The last ball has an elasticity of 1, so no energy will every be lost
		//       from the system after collision. So normally it would bounce (at the same height) forever,
		//       though considering we also add additional energy to compensate for
		//       positional errors.. the ball actually bounces higher and higher.
		{
			//Sphere Bounce-Pad
			GameObject* obj = CommonUtils::BuildCuboidObject(
				"BouncePad",
				Vector3(-2.5f, 0.5f, -5.0f),
				Vector3(5.0f, 0.5f, 2.0f),
				true,
				0.0f,
				true,
				false,
				Vector4(0.2f, 0.5f, 1.0f, 1.0f));
			obj->Physics()->SetFriction(1.0f);
			obj->Physics()->SetElasticity(1.0f);
			this->AddGameObject(obj);

			//Create Bouncing Spheres
			for (int i = 0; i <= 5; ++i)
			{
				Vector4 color = CommonUtils::GenColor(0.7f + i * 0.05f, 1.0f);
				GameObject* obj = CommonUtils::BuildSphereObject(
					"",
					Vector3(-6.0f + i * 1.25f, 5.5f, -5.0f),
					0.5f,
					true,
					1.0f,
					true,
					true,
					color);
				obj->Physics()->SetFriction(0.5f);
				obj->Physics()->SetElasticity(i * 0.2f);
				this->AddGameObject(obj);
			}
		}



	}

};