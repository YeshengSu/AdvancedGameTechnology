#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>


class Scene_Angles : public Scene
{
protected:
	GameObject* player;

	float		yaw; //Life is easier in 2D ;)
	Vector3     vel; // All of this is easier with physics....
	GameObject* target;
public:

	//This function isn't actually called (the code is duplicated in DebugDraw function below)
	// - Just wanted to keep code for actually calculating the angle nice and easy to read.
	float GetAngle()
	{
		//Bleh...
		Matrix3 player_rot = Matrix3(player->Render()->GetTransform());
		Vector3 forward_dir = player_rot * Vector3(0, 0, -1);

		Vector3 goal_pos = target->Render()->GetTransform().GetPositionVector();
		Vector3 player_pos = player->Render()->GetTransform().GetPositionVector();


		//Get the direction from the player to goal
		Vector3 goal_dir = (goal_pos - player_pos).Normalise();
		
		
		//Calculate the absolute angle between the two vectors
		float costheta = Vector3::Dot(forward_dir, goal_dir);
		float angle = (float)RadToDeg(acosf(costheta));


		//Optional: As the above is an absolute angle, to work out if it's negative rotation
		//          or positive we also need to work out if it's on the left or right of the player.
		Vector3 right_dir = player_rot * Vector3(1, 0, 0);
		angle = _copysignf(angle, Vector3::Dot(right_dir, goal_dir));

		return angle;
	}





	Scene_Angles(const std::string& friendly_name)
		: Scene(friendly_name)
		, yaw(0.0f)
		, vel(0.0f, 0.0f, 0.0f)
	{
	}

	virtual ~Scene_Angles()
	{

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

		player = CommonUtils::BuildCuboidObject(
			"",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(0.2f, 0.2f, 0.5f),
			true,
			0.0f,
			false,
			true,
			Vector4(0.5f, 1.0f, 0.8f, 1.0f));
		player->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), 45.f));
		this->AddGameObject(player);

		target = CommonUtils::BuildSphereObject(
			"",
			Vector3(0.0f, 0.0f, -5.0f),
			0.3f,
			false,
			0.0f,
			false,
			true,
			Vector4(1.0f, 0.0f, 0.0f, 0.5f));
		this->AddGameObject(target);
	}

	virtual void OnUpdateScene(float dt) override
	{
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   [G] To rotate target");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Right click and drag to rotate 'player'");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "----------------");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "A visual representation on how to compute angles with");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "dot products. Very simple but always useful :)");

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_G))
		{
			const Vector3 rotation_axis = Vector3(0.f, 1.f, 0.f);
			target->Render()->SetTransform(Matrix4::Rotation(dt * 45.0f, rotation_axis) * target->Render()->GetTransform());
		}
		DebugDraw();
	}

	void DebugDraw()
	{
		//Just recompute this here and keep the top explanation code neat and tidy <3
		Vector3 goal_pos = target->Render()->GetTransform().GetPositionVector();
		Vector3 player_pos = player->Render()->GetTransform().GetPositionVector();

		Matrix3 player_rot = Matrix3(player->Render()->GetTransform());

		Vector3 goal_dir = (goal_pos - player_pos).Normalise();
		Vector3 forward_dir = player_rot * Vector3(0, 0, -1);
		Vector3 right_dir = player_rot * Vector3(1, 0, 0);

		float costheta = Vector3::Dot(forward_dir, goal_dir);
		float angle = (float)RadToDeg(acosf(costheta));

		angle = _copysignf(angle, Vector3::Dot(right_dir, goal_dir));

		const float line_length = 5.0f;
		const float text_length = 4.0f;
		const float angle_radius = 3.5f;

		Vector3 up_axis = Vector3::Cross(goal_dir, forward_dir);
		Matrix3 rot = Matrix3::Rotation(forward_dir, up_axis);

		NCLDebug::DrawTextWs(
			rot * (Vector3(sinf((float)DegToRad(fabs(angle)*0.5f)), 0.0f, -cosf((float)DegToRad(fabs(angle)*0.5f))) * text_length),
			STATUS_TEXT_SIZE, TEXTALIGN_CENTRE,
			Vector4(0.f, 0.f, 0.f, 1.0f),
			"%5.2f degrees", angle);

		NCLDebug::DrawThickLine(player_pos, player_pos + forward_dir * line_length, 0.02f);
		NCLDebug::DrawThickLine(player_pos, player_pos + goal_dir * line_length, 0.02f);




		Vector3 start, end;
		bool first = true;
		for (float f = 0.0f; f <= fabs(angle); f += 3.0f)
		{
			end = rot * (Vector3(sinf((float)DegToRad(f)), 0.0f, -cosf((float)DegToRad(f))) * angle_radius);

			if (!first)	NCLDebug::DrawThickLine(start, end, 0.02f);

			first = false;
			start = end;
		}

		end = rot * (Vector3(sinf((float)DegToRad(fabs(angle))), 0.0f, -cosf((float)DegToRad(fabs(angle)))) * angle_radius);
		NCLDebug::DrawThickLine(start, end, 0.02f);
	}
};