
#pragma once

#include <nclgl\OBJMesh.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\GameObject.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\CommonMeshes.h>
#include "ObjectPlayer.h"


class Phy4_AiCallbacks: public Scene
{
public:
	Phy4_AiCallbacks(const std::string& friendly_name)
		: Scene(friendly_name)
		, m_MeshPlayer(new OBJMesh(MESHDIR"raptor.obj"))
	{
	}

	virtual ~Phy4_AiCallbacks()
	{

		if (m_MeshPlayer)
		{
			delete m_MeshPlayer;
			m_MeshPlayer = NULL;
		}

	}

	virtual void OnInitializeScene() override
	{
		//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.001f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));


		//Create Player
		ObjectPlayer* player = new ObjectPlayer("Player1");
		player->SetRender(new RenderNode(m_MeshPlayer));
		player->Render()->SetBoundingRadius(1.0f);
		player->Render()->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

		player->SetPhysics(new PhysicsNode());
		player->Physics()->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
		player->Physics()->SetCollisionShape(new CuboidCollisionShape(Vector3(0.5f, 0.5f, 1.0f)));

		this->AddGameObject(player);


		//Yellow City	
		const Vector3 col_size1 = Vector3(2.0f, 0.2f, 2.f);

		RenderNode* citymesh_yellow = BuildDistrictThing(10, 0.05f, 0.2f);
		citymesh_yellow->SetTransform(Matrix4::Scale(col_size1));

		GameObject* city1 = new GameObject("City1");
		city1->SetRender(new RenderNode());
		city1->Render()->AddChild(citymesh_yellow);
		city1->SetPhysics(new PhysicsNode());
		city1->Physics()->SetPosition(Vector3(-5.0f, col_size1.y, -5.0f));
		city1->Physics()->SetCollisionShape(new CuboidCollisionShape(col_size1));
		city1->Physics()->SetOnCollisionCallback(&Phy4_AiCallbacks::ExampleCallbackFunction1);

		this->AddGameObject(city1);

		//Green City
		const Vector3 col_size2 = Vector3(2.0f, 0.5f, 2.f);
		RenderNode* citymesh_green = BuildDistrictThing(10, 0.3f, 0.45f);
		citymesh_green->SetTransform(Matrix4::Scale(col_size2));

		GameObject* city2 = new GameObject("City2");
		city2->SetRender(new RenderNode());
		city2->Render()->AddChild(citymesh_green);

		city2->SetPhysics(new PhysicsNode());
		city2->Physics()->SetPosition(Vector3(5.0f, col_size2.y, -5.0f));
		city2->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), 45.f));
		city2->Physics()->SetCollisionShape(new CuboidCollisionShape(col_size2));
		city2->Physics()->SetOnCollisionCallback(
			std::bind(&Phy4_AiCallbacks::ExampleCallbackFunction2,
				this,							//Any non-placeholder param will be passed into the function each time it is called
				std::placeholders::_1,			//The placeholders correlate to the expected parameters being passed to the callback
				std::placeholders::_2
			)
		);

		this->AddGameObject(city2);


		//'Hidden' Physics Node (no visual mesh or renderable object attached)	
		PhysicsNode* nde = new PhysicsNode();
		nde->SetPosition(Vector3(5.0f, 1.0f, 0.0f));
		nde->SetCollisionShape(new SphereCollisionShape(1.0f));
		nde->SetOnCollisionCallback([nde](PhysicsNode* self, PhysicsNode* collidingObject) {

			NCLDebug::Log(Vector3(1.0f, 0.0f, 0.0f), "You found the secret! - Inline callback");

			float r_x = 5.f * ((rand() % 200) / 100.f - 1.0f);
			float r_z = 3.f * ((rand() % 200) / 100.f - 1.0f);
			nde->SetPosition(Vector3(r_x, 1.0f, r_z + 3.0f));
			return false;
		});
		PhysicsEngine::Instance()->AddPhysicsObject(nde);

	}


	//Example of static callback (cannot use 'this' parameter)
	static bool ExampleCallbackFunction1(PhysicsNode* self, PhysicsNode* collidingObject)
	{
		NCLDebug::Log(Vector3(0.3f, 1.0f, 0.3f), "Player is inside the Yellow City! - Static callback");

		//Return true to enable collision resolution, for AI test's just return false so we can drop the collision pair from the system
		return false; 
	}

	//Example of member callback ('this' parameter is bound at bind time)
	bool ExampleCallbackFunction2(PhysicsNode* self, PhysicsNode* collidingObject)
	{
		NCLDebug::Log(Vector3(0.3f, 1.0f, 0.3f), "Player is inside the Green City! - Member callback");

		//Return true to enable collision resolution, for AI test's just return false so we can drop the collision pair from the system
		return false;
	}









	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    Arrow Keys to Move Player");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    -> See Log in bottom left for collision detection output <-");
	}

	RenderNode* BuildDistrictThing(int grid_size, float col_range_start, float col_range_end)
	{
		const float spacing = 2.0f / float(grid_size * grid_size);

		const float divisor = 2.0f / (float(grid_size) + 1.0f);
		const float hdivisor = divisor * 0.5f;

		RenderNode* root = new RenderNode();
		for (int x = 0; x < grid_size; ++x)
		{
			for (int z = 0; z < grid_size; ++z)
			{
				float building_scale = (rand() % 100) / 110.0f + 0.1f; //Range 0.1x - 1x
				float color_hue = col_range_start + (rand() % 100) / 100.0f * (col_range_end - col_range_start);

				RenderNode* node = new RenderNode();
				node->SetMesh(CommonMeshes::Cube());
				node->SetTransform(
					Matrix4::Translation(Vector3(
						-1.f + x*spacing + x * divisor + hdivisor,
						-1.f + building_scale,
						-1.f + z*spacing + z * divisor + hdivisor)) *
					Matrix4::Scale(Vector3(hdivisor, building_scale, hdivisor))
				);
				node->SetColor(CommonUtils::GenColor(color_hue));
				root->AddChild(node);
			}
		}

		return root;
	}

private:
	OBJMesh* m_MeshPlayer;
};