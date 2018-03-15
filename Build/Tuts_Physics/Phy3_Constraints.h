
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\NCLDebug.h>

class Phy3_Constraints : public Scene
{
public:
	Phy3_Constraints(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual void OnInitializeScene() override
	{
		//Create Ground (..why not?)
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

		GameObject *handle, *ball;

		//Create Hanging Ball
			handle = CommonUtils::BuildSphereObject("",
				Vector3(-7.f, 7.f, -5.0f),				//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.45f, 0.5f));	//Color

			ball = CommonUtils::BuildSphereObject("",
				Vector3(-4.f, 7.f, -5.0f),				//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				1.0f,									// Inverse Mass = 1 / 1kg mass
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.5f, 1.0f));		//Color

			this->AddGameObject(handle);
			this->AddGameObject(ball);

			//Add distance constraint between the two objects
			DistanceConstraint* constraint = new DistanceConstraint(
				handle->Physics(),					//Physics Object A
				ball->Physics(),					//Physics Object B
				handle->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
				ball->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			PhysicsEngine::Instance()->AddConstraint(constraint);
		




		//Create Hanging Cube (Attached by corner)
			handle = CommonUtils::BuildSphereObject("",
				Vector3(4.f, 7.f, -5.0f),				//Position
				1.0f,									//Radius
				true,									//Has Physics Object
				0.1f,									//Inverse Mass = 1 / 10 kg mass (For creating rotational inertia tensor)
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.55f, 0.5f));	//Color

			//Set linear mass to be infinite, so it can rotate still but not move
			handle->Physics()->SetInverseMass(0.0f);

			ball = CommonUtils::BuildCuboidObject("",
				Vector3(7.f, 7.f, -5.0f),				//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half Dimensions
				true,									//Has Physics Object
				1.0f,									//Inverse Mass = 1 / 1kg mass
				false,									//No Collision Shape Yet
				true,									//Dragable by the user
				CommonUtils::GenColor(0.6f, 1.0f));		//Color

			this->AddGameObject(handle);
			this->AddGameObject(ball);

			PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
				handle->Physics(),													//Physics Object A
				ball->Physics(),													//Physics Object B
				handle->Physics()->GetPosition() + Vector3(1.0f, 0.0f, 0.0f),		//Attachment Position on Object A	-> Currently the far right edge
				ball->Physics()->GetPosition() + Vector3(-0.5f, -0.5f, -0.5f)));	//Attachment Position on Object B	-> Currently the far left edge 
		


	}

};