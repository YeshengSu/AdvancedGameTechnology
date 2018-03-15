
//Very simple example of a finite state machine.
//The dinosaurMesh has a hunger and thirst requirements, when
// one is low he will change state and go eat/drink



#pragma once
#include <ncltech\GameObject.h>
#include <functional>
#include <algorithm>

//Callback function for state update
//Params:
//	float dt - timestep
//Return:
typedef std::function<void(float dt)> OnStateUpdateCallback;

class HungryDinosaur : public GameObject
{
public:
	HungryDinosaur();
	~HungryDinosaur();

	virtual void OnAttachedToScene() override;
	virtual void OnDetachedFromScene() override;

	void OnGlobalUpdate(float dt);

	void SetFoodAndDrinkLocations(const Vector3& food, const Vector3& drink);
	float GetHunger() { return hunger; }
	float GetThirst() { return thirst; }
	std::string GetStateName() { return state_name; }
protected:

	void OnIdleState(float dt);
	void OnEatingState(float dt);
	void OnDrinkingState(float dt);
	void OnTravellingState(float dt, const Vector3& target_pos, OnStateUpdateCallback next_state);

protected:
	//Global
	Vector3 location_drink;
	Vector3 location_food;

	float thirst;
	float hunger;

	std::string state_name;
	OnStateUpdateCallback currentState;
};