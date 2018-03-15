
#pragma once
#include <ncltech\GameObject.h>
#include <nclgl\NCLDebug.h>

class ObjectPlayer : public GameObject
{
public:
	ObjectPlayer(const std::string& name) 
		: GameObject(name) 
	{
		
	}

	virtual void OnAttachedToScene() override
	{
		//This function binds the "this" parameter to the callback function that should only take
		// a single float value. Allows us to have callbacks to member functions or specify fixed parameters
		// that will always be parsed when the callback is fired.
		SceneUpdateCallback callback = std::bind(&ObjectPlayer::OnUpdateCallback, this, std::placeholders::_1);

		//Register a new update callback
		scene->RegisterOnUpdateCallback(this, callback);
	}

	virtual void OnDetachedFromScene() override
	{
		scene->UnregisterOnUpdateCallback(this);
	}

protected:
	void	OnUpdateCallback(float dt)
	{
		if (!this->HasPhysics())
		{
			NCLERROR("Player object does not have a valid physics node to manipulate!");
		}
		else
		{
			const float mv_speed = 5.f * dt;			//Meters per second
			const float rot_speed = 90.f * dt;			//Rotational degrees per second

			if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
			{
				Physics()->SetPosition(Physics()->GetPosition() +
					Physics()->GetOrientation().ToMatrix3() * Vector3(0.0f, 0.0f, -mv_speed));
			}

			if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
			{
				Physics()->SetPosition(Physics()->GetPosition() +
					Physics()->GetOrientation().ToMatrix3()* Vector3(0.0f, 0.0f, mv_speed));
			}

			if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
			{
				Physics()->SetOrientation(Physics()->GetOrientation() *
					Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), rot_speed));
			}

			if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
			{
				Physics()->SetOrientation(Physics()->GetOrientation() *
					Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), -rot_speed));
			}
		}
	}
};