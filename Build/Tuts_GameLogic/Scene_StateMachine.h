#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\OBJMesh.h>
#include "HungryDinosaur.h"

class Scene_StateMachine : public Scene
{
protected:
	HungryDinosaur* dinosaur;
	GameObject* food;
	GameObject* drink;
	OBJMesh* mesh_raptor;
public:
	Scene_StateMachine(const std::string& friendly_name)
		: Scene(friendly_name)
		, dinosaur(NULL)
		, mesh_raptor(new OBJMesh(MESHDIR"raptor.obj"))
	{
	}

	virtual ~Scene_StateMachine()
	{
		SAFE_DELETE(mesh_raptor);
	}


	virtual void OnInitializeScene() override
	{
		//There is always too much ground to cover...
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.5f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));


		RenderNode* mesh = new RenderNode();
		mesh->SetMesh(mesh_raptor);
		dinosaur = new HungryDinosaur();
		dinosaur->SetRender(mesh);
		this->AddGameObject(dinosaur);

		food = CommonUtils::BuildCuboidObject(
			"",
			Vector3(-4.0f, 0.0f, -5.0f),
			Vector3(0.5f, 1.f, 0.5f),
			false,
			0.0f,
			false,
			true,
			Vector4(1.0f, 0.8f, 0.5f, 0.5f));
		this->AddGameObject(food);

		drink = CommonUtils::BuildCuboidObject(
			"",
			Vector3(4.0f, 0.0f, -5.0f),
			Vector3(0.5f, 1.f, 0.5f),
			false,
			0.0f,
			false,
			true,
			Vector4(0.2f, 0.4f, 1.0f, 0.5f));
		this->AddGameObject(drink);
	}

	virtual void OnUpdateScene(float dt) override
	{
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   The AI is too strong for puny human controls.");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "----------------");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Simple example of a Finite State Machine to govern");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "the various actions of a dinosaur. In this example, ");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "he has 4 states (Eating, Drinking, Idle and Travelling)");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "and will automatically switch between states based on his");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "current needs. e.g. thirsty -> travel to water -> drink");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Current State ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Thirst: %2.0f", dinosaur->GetThirst());
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Hunger: %2.0f", dinosaur->GetHunger());
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   State: %s", dinosaur->GetStateName().c_str());

		Vector3 food_pos = food->Render()->GetTransform().GetPositionVector();
		Vector3 drink_pos = drink->Render()->GetTransform().GetPositionVector();
		Vector3 dino_pos = dinosaur->Render()->GetTransform().GetPositionVector();

		dinosaur->SetFoodAndDrinkLocations( food_pos, drink_pos );

		NCLDebug::DrawTextWs(food_pos + Vector3(0.f, 1.2f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "Food");
		NCLDebug::DrawTextWs(drink_pos + Vector3(0.f, 1.2f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "Water");
		NCLDebug::DrawTextWs(dino_pos + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "Thirst: %2.0f", dinosaur->GetThirst());
		NCLDebug::DrawTextWs(dino_pos + Vector3(0.f, 0.4f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "Hunger: %2.0f", dinosaur->GetHunger());
	}

};