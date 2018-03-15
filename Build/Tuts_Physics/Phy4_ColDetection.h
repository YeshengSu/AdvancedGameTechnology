
#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
#include "ncltech\SphereCollisionShape.h"
#include "ncltech\CuboidCollisionShape.h"
#include <ncltech\EdgeCollisionShape.h>

class Phy4_ColDetection : public Scene
{
public:
	Phy4_ColDetection(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	const Vector3 ss_pos = Vector3(-5.5f, 1.5f, -5.0f);
	const Vector3 sc_pos = Vector3(4.5f, 1.5f, -5.0f);
	const Vector3 cc_pos = Vector3(-0.5f, 1.5f, 5.0f);

	virtual void OnInitializeScene() override
	{
		//Create Ground (..everybody loves finding some common ground)
		GameObject* ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));

		this->AddGameObject(ground);


		//Create Sphere-Sphere Manifold Test
		{

			this->AddGameObject(CommonUtils::BuildSphereObject("orbiting_sphere1",
				ss_pos + Vector3(0.75f, 0.0f, 0.0f),	//Position leading to 0.25 meter overlap between spheres
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(0.45f, 0.5f)));	//Color

			this->AddGameObject(CommonUtils::BuildSphereObject("",
				ss_pos,									//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(0.5f, 1.0f)));	//Color

		}

		//Create Sphere-Cuboid Manifold Test
		{
			this->AddGameObject(CommonUtils::BuildCuboidObject("",
				sc_pos,									//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(0.5f, 1.0f)));	//Color

			this->AddGameObject(CommonUtils::BuildSphereObject("orbiting_sphere2",
				sc_pos + Vector3(0.9f, 0.0f, 0.0f),		//Position leading to 0.1 meter overlap on faces, and more on diagonals
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(0.45f, 0.5f)));	//Color
		}

		//Create Cuboid-Cuboid Manifold Test
		{

			this->AddGameObject(CommonUtils::BuildCuboidObject("rotating_cuboid1",
				cc_pos + Vector3(0.75f, 0.0f, 0.0f),	//Position leading to 0.25 meter overlap on faces, and more on diagonals
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(0.45f, 0.5f)));	//Color

			this->AddGameObject(CommonUtils::BuildCuboidObject("",
				cc_pos,									//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColor(0.5f, 1.0f)));	//Color
		}

	}

	float m_AccumTime;
	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		//Update Rotating Objects!
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_1))
		{
			m_AccumTime += dt;

			float cost = cosf((float)DegToRad(m_AccumTime * 45.0f));
			float sint = sinf((float)DegToRad(m_AccumTime * 45.0f));

			GameObject* orbiting_sphere1 = this->FindGameObject("orbiting_sphere1");
			if (orbiting_sphere1 != NULL)
			{
				//Move orbiting sphere1 around centre object at 45 degrees per second with an orbiting radius of 75cm
				orbiting_sphere1->Physics()->SetPosition(Vector3(
					ss_pos.x + cost * 0.75f,
					ss_pos.y,
					ss_pos.z + sint * 0.75f));
			}

			GameObject* orbiting_sphere2 = this->FindGameObject("orbiting_sphere2");
			if (orbiting_sphere2 != NULL)
			{
				//Move orbiting sphere2 around centre object at 45 degrees per second with an orbiting radius of 90cm
				orbiting_sphere2->Physics()->SetPosition(Vector3(
					sc_pos.x + cost * 0.9f,
					sc_pos.y,
					sc_pos.z + sint * 0.9f));
			}

			GameObject* rotating_cuboid1 = this->FindGameObject("rotating_cuboid1");
			if (rotating_cuboid1 != NULL)
			{
				//Move orbiting sphere2 around centre object at 45 degrees per second with an orbiting radius of 90cm
				rotating_cuboid1->Physics()->SetPosition(Vector3(
					cc_pos.x + cost * 0.9f,
					cc_pos.y,
					cc_pos.z + sint * 0.9f));
			}
		}


		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_F))
		{
			Camera* cam = GraphicsPipeline::Instance()->GetCamera();
			Vector3 radius = Vector3(0.5f, 0.5f, 0.5f);

			RenderNode* sphereRender = new RenderNode();
			sphereRender->SetMesh(CommonMeshes::Sphere());
			sphereRender->SetTransform(Matrix4::Scale(radius)); //No position! That is now all handled in PhysicsNode
			sphereRender->SetColor(Vector4(1.0f, 0.2f, 0.5f, 1.0f));
			sphereRender->SetBoundingRadius(1.0f);

			GameObject* m_Sphere = new GameObject("projectile");
			m_Sphere->SetRender(new RenderNode());
			m_Sphere->Render()->AddChild(sphereRender);
			m_Sphere->SetPhysics(new PhysicsNode());
			m_Sphere->Physics()->SetInverseMass(1.f);
			m_Sphere->Physics()->SetPosition(cam->GetPosition());
			m_Sphere->Physics()->SetLinearVelocity(cam->GetDirection()*20.0f);
			m_Sphere->Physics()->SetOrientation(Quaternion());
			m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));
			
			CollisionShape* pShereColShape = new SphereCollisionShape(radius.x);
			pShereColShape->SetOffset(Vector3(1.0f,0.0f,0.0f));
			m_Sphere->Physics()->SetCollisionShape(pShereColShape);
			m_Sphere->Physics()->SetInverseInertia(pShereColShape->BuildInverseInertia(1.f));

			CollisionShape* pCubeColShape = new CuboidCollisionShape(radius);
			m_Sphere->Physics()->SetCollisionShape(pCubeColShape);
			m_Sphere->Physics()->SetInverseInertia(pCubeColShape->BuildInverseInertia(1.f));

			this->AddGameObject(m_Sphere);
		}

		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();


		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    Hold [1] to rotate objects");
	}
};