#include "HungryDinosaur.h"
#include <ncltech\Scene.h>

HungryDinosaur::HungryDinosaur()
	: GameObject("player")
	, location_drink(0.0f, 0.0f, 0.0f)
	, location_food(0.0f, 0.0f, 0.0f)
	, thirst(50.f)
	, hunger(100.f)
{
	currentState = std::bind(&HungryDinosaur::OnIdleState, this, std::placeholders::_1);
	state_name = "Idle";
}

HungryDinosaur::~HungryDinosaur()
{
}

void HungryDinosaur::OnAttachedToScene()
{
	//This function binds the "this" parameter to the callback function that should only take
	// a single float value. Allows us to have callbacks to member functions or specify fixed parameters
	// that will always be parsed when the callback is fired.
	SceneUpdateCallback callback = std::bind(&HungryDinosaur::OnGlobalUpdate, this, std::placeholders::_1);

	//Register a new update callback
	scene->RegisterOnUpdateCallback(this, callback);
}

void HungryDinosaur::OnDetachedFromScene()
{
	scene->UnregisterOnUpdateCallback(this);
}

void HungryDinosaur::SetFoodAndDrinkLocations(const Vector3& food, const Vector3& drink)
{
	location_food = food;
	location_drink = drink;
}

void HungryDinosaur::OnGlobalUpdate(float dt)
{
	if (!this->Render())
		return;


	//Dinosaurs are constantly hungry and thirsty...
	thirst -= 20.0f * dt;
	hunger -= 5.0f * dt;


	//All we do here is pass the update on
	if (currentState)
		currentState(dt);
	else
		NCLERROR("State machine error! - No update state is currently bound")
}


void HungryDinosaur::OnIdleState(float dt)
{
	state_name = "Idle";

	//Idle State
	this->Render()->SetTransform(
		this->Render()->GetTransform() * Matrix4::Rotation(45.f * dt, Vector3(0, 1, 0)) * Matrix4::Translation(Vector3(0, 0, -2.f * dt)));


	//Possible State Transitions
	if (thirst < 50.0f)
	{
		state_name = "Travelling to Water";

		OnStateUpdateCallback target_state = std::bind(&HungryDinosaur::OnDrinkingState, this, std::placeholders::_1);

		currentState = std::bind(&HungryDinosaur::OnTravellingState, this,
			std::placeholders::_1,
			location_drink, target_state);
	}

	if (hunger < 50.0f)
	{
		state_name = "Travelling to Food";

		OnStateUpdateCallback target_state = std::bind(&HungryDinosaur::OnEatingState, this, std::placeholders::_1);

		currentState = std::bind(&HungryDinosaur::OnTravellingState, this,
			std::placeholders::_1,
			location_food, target_state);
	}
}

void HungryDinosaur::OnEatingState(float dt)
{
	state_name = "Eating";

	//Eating State
	hunger += 50.0f * dt;

	//Possible State Transitions
	if (hunger >= 100.f)
	{
		currentState = std::bind(&HungryDinosaur::OnIdleState, this, std::placeholders::_1);
	}
}

void HungryDinosaur::OnDrinkingState(float dt)
{
	state_name = "Drinking";

	//Eating State
	thirst += 50.0f * dt;

	//Possible State Transitions
	if (thirst >= 100.f)
	{
		currentState = std::bind(&HungryDinosaur::OnIdleState, this, std::placeholders::_1);
	}
}

void HungryDinosaur::OnTravellingState(float dt, const Vector3& target_pos, OnStateUpdateCallback next_state)
{	
	//Travelling State
	Vector3 current_pos = this->Render()->GetTransform().GetPositionVector();
	Vector3 diff = target_pos - current_pos;
	float dist = diff.Length();

	const float max_speed = 5.f;

	if (max_speed * dt < dist)
	{
		diff = diff / dist; //Normalise()
		diff = diff * max_speed * dt;
	}

	Vector3 target_pos2D = target_pos; target_pos2D.y = 0.0f;
	Vector3 current_pos2D = current_pos; current_pos2D.y = 0.0f;

	Quaternion rot = Quaternion::LookAt(target_pos, current_pos);
	current_pos = current_pos + diff;
	
	this->Render()->SetTransform(Matrix4::Translation(current_pos) * rot.ToMatrix4());


	//Possible State Transitions

	if (dist < 0.3f)
	{
		currentState = next_state;
	}
}