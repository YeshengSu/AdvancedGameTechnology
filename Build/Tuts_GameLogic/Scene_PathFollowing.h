#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>

const float time_between_nodes = 0.5f;
const int num_control_points = 10;
const float total_circuit_time = float(num_control_points) * time_between_nodes;

class Scene_PathFollowing : public Scene
{
protected:
	GameObject* player;
	std::vector<GameObject*> path_nodes;
	std::vector<Quaternion> path_rotations; //These can matrices, or stored in the rendernodes themselves etc
	int current_node;
	float elapsed_time;
	
	float cubic_tangent_weighting;

	int interpolation_type_pos;
	int interpolation_type_rot;
	
public:
	//As all the logic for this example is self contained so i've tried
	// to put all the boring scene initialization chaff at the end.
	// Apologies if the class layout is too painful..
	void GetNodeIdxAndFactor(float elapsed, int& out_idx, float& out_factor)
	{
		elapsed = fmod(elapsed, total_circuit_time);

		//Get the current node in elapsed time 
		float node_time = elapsed / time_between_nodes;

		//Get the current node index - floor(node_time)
		// and the factor - decimal part
		float node_time_floored;

		out_factor = modf(node_time, &node_time_floored);
		out_idx = (int)node_time_floored;
	}

	virtual void OnUpdateScene(float dt) override
	{
		//Build rotations for path based on their positions
		UpdatePathForwardVectors();


		const uint num_nodes = path_nodes.size();
		elapsed_time += dt;
		elapsed_time = fmod(elapsed_time, total_circuit_time);

		
		int current_node;
		float factor;
		GetNodeIdxAndFactor(elapsed_time, current_node, factor);
		
//Position
		Vector3 player_pos;
		switch (interpolation_type_pos)
		{
		default:
		case 0:		
			//Linear
			player_pos = InterpolatePositionLinear(
				current_node,
				(current_node + 1) % num_nodes,
				factor);
			
			break;
		case 1:
			//Cubic/Hermite spline
			//Requires nodes previous and ahead of current segment
			player_pos = InterpolatePositionCubic(
				(current_node + num_nodes - 1) % num_nodes,
				current_node,
				(current_node + 1) % num_nodes,
				(current_node + 2) % num_nodes,
				factor);		
			break;
		}

//Rotation
		Quaternion player_rot;
		switch (interpolation_type_rot)
		{
		default:
		case 0:
		{
			//Direct-Linear
			// - This should never be used for reasons shown here.
			player_rot = InterpolateRotationLinear(
				current_node,
				(current_node + 1) % num_nodes,
				factor, false);
		}
		break;
		case 1:
		{
			//Spherical Linear Interpolation
			player_rot = InterpolateRotationLinear(
				current_node,
				(current_node + 1) % num_nodes,
				factor, true);
		}
		break;
		case 2:
		{
			//Look-Ahead
			// - Fast and simple.. the better the positional interpolation, the better the rotation =]
			GetNodeIdxAndFactor(elapsed_time + dt, current_node, factor);
			Vector3 ahead;
			if (interpolation_type_pos == 0)
			{
				player_pos = InterpolatePositionLinear(
					current_node,
					(current_node + 1) % num_nodes,
					factor);
			}
			else
			{
				ahead = InterpolatePositionCubic(
					(current_node + num_nodes - 1) % num_nodes,
					current_node,
					(current_node + 1) % num_nodes,
					(current_node + 2) % num_nodes,
					factor);		
			}
			player_rot = Quaternion::LookAt(player_pos, ahead);
		}
		break;
		}

		player->Render()->SetTransform(Matrix4::Translation(player_pos) * player_rot.ToMatrix4());
		DebugDrawPath();



		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Interpolation Method:-");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "       [H] Rotation: %s", 
			(interpolation_type_rot == 0) ? "Direct-Linear (and why you should never lerp a quaternion!)" : (interpolation_type_rot == 1) ? "Spherical-Linear" : "Look Ahead");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "       [G] Position: %s", (interpolation_type_pos == 0) ? "Linear" : "Cubic/Hermite");

		if (interpolation_type_pos == 1)
		{
			NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "       Tangent Factor: %5.2f [1/2]", cubic_tangent_weighting);

			if (Window::GetKeyboard()->KeyDown(KEYBOARD_1))
			{
				cubic_tangent_weighting -= dt * 0.25f;
			}
			if (Window::GetKeyboard()->KeyDown(KEYBOARD_2))
			{
				cubic_tangent_weighting += dt * 0.25f;
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G))
		{
			interpolation_type_pos = (interpolation_type_pos + 1) % 2;
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_H))
		{
			interpolation_type_rot = (interpolation_type_rot + 1) % 3;
		}


		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Move the control points around to change the path");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "----------------");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "A little demo to explain and compare linear");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "and cubic interpolation methods.");


	}

	//Linearly interpolates between a-b
	Vector3 InterpolatePositionLinear(int node_a, int node_b, float factor)
	{
		//With factor between 0-1, this is defined as:
		// LerpA-B(factor) = (1 - factor) * A + factor * B
		Vector3 posA = path_nodes[node_a]->Render()->GetTransform().GetPositionVector();
		Vector3 posB = path_nodes[node_b]->Render()->GetTransform().GetPositionVector();

		return posA * (1.0f - factor) + posB * factor;
	}

	Quaternion InterpolateRotationLinear(int node_a, int node_b, float factor, bool use_proper_slerp)
	{
		Quaternion rotA = path_rotations[node_a];
		Quaternion rotB = path_rotations[node_b];

		if (!use_proper_slerp)
		{
			return Quaternion::Lerp(rotA, rotB, factor);
		}
		else
		{
			//For quaternions we must do spherical interpolation! The interpolation
			// is the same, but we interpolate the minimal angle of rotation instead of the
			// axis of rotation. 
			return Quaternion::Slerp(rotA, rotB, factor);
		}
	}

	//Cubicly interpolates between b-c (with a and d being the nodes after/before the current segment)
	Vector3 InterpolatePositionCubic(int node_a, int node_b, int node_c, int node_d, float f)
	{
		//With factor between 0-1, this is defined as:
		// - See wiki Hermite Spline
		Vector3 posA = path_nodes[node_a]->Render()->GetTransform().GetPositionVector();
		Vector3 posB = path_nodes[node_b]->Render()->GetTransform().GetPositionVector();
		Vector3 posC = path_nodes[node_c]->Render()->GetTransform().GetPositionVector();
		Vector3 posD = path_nodes[node_d]->Render()->GetTransform().GetPositionVector();

		//Get the tangents at points B and C
		// - These are the exit velocities of the two points, so if you change the 0.5f factor
		//   here you will either expand or shrink the amount of curvature. The half default
		//   means that the direction of the curve entering/leaving will be half way towards the
		//   the direction needed to hit the next point.
		Vector3 tanB = (posC - posA) * cubic_tangent_weighting;
		Vector3 tanC = (posD - posB) * cubic_tangent_weighting;

		float f2 = f * f;
		float f3 = f2 * f;

		return posB * (f3 * 2.0f - 3.0f * f2 + 1.0f)
			+ tanB * (f3 - 2.0f * f2 + f)
			+ posC * (-2.0f * f3 + 3.0f * f2)
			+ tanC * (f3 - f2);
	}
		
	void UpdatePathForwardVectors()
	{
		//To do rotational interpolation we need to generate some rotations for the path nodes.
		// - For the node example we can just do a LookAt from each control point to the next
		//   node in the path.
		const uint size = path_nodes.size();

		Vector3 centre = Vector3(0, 0, 0);
		for (uint i = 0; i < size; ++i)
		{
			centre = centre + path_nodes[i]->Render()->GetTransform().GetPositionVector();
		}
		centre = centre / (float)size;

		path_rotations.resize(size);
		for (uint i = 0; i < size; ++i)
		{
			Vector3 cur_node_pos = path_nodes[i % size]->Render()->GetTransform().GetPositionVector();
			Vector3 nxt_node_pos = path_nodes[(i + 1) % size]->Render()->GetTransform().GetPositionVector();
		
			path_rotations[i] = Quaternion::LookAt(cur_node_pos, nxt_node_pos, Vector3(0, 1, 0));
		}
	}


	Scene_PathFollowing(const std::string& friendly_name)
		: Scene(friendly_name)
		, elapsed_time(0.0f)
		, interpolation_type_pos(1)
		, interpolation_type_rot(1)
		, cubic_tangent_weighting(0.5f)
	{
	}

	virtual ~Scene_PathFollowing()
	{
	}


	virtual void OnInitializeScene() override
	{
		elapsed_time = 0.0f;
		interpolation_type_pos = 1;
		interpolation_type_rot = 1;
		cubic_tangent_weighting = 0.5f;

		//There is always too much ground to cover...
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));


		//Create some balls to drag around and form our path
		const Vector4 ball_col = Vector4(1.0f, 0.f, 0.f, 1.0f);
		GameObject* obj;
		for (int i = 0; i < num_control_points; ++i)
		{
			float angle = (i / (float)num_control_points) * 2.0f * PI;
			float x = cosf(angle) * 6.0f;
			float z = sinf(angle) * 6.0f;

			obj = CommonUtils::BuildSphereObject(
				"",
				Vector3(x, (i%3 == 0) ? 4.0f : 1.0f, z),
				0.3f,
				false,
				0.0f,
				false,
				true,
				ball_col);
			path_nodes.push_back(obj);
			this->AddGameObject(obj);
		}


		player = CommonUtils::BuildCuboidObject(
			"",
			Vector3(0.0f, 0.5f, 0.0f),
			Vector3(0.2f, 0.2f, 0.5f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.5f, 1.0f, 0.8f, 1.0f));
		this->AddGameObject(player);
	}


	virtual void OnCleanupScene() override
	{
		Scene::OnCleanupScene();

		player = NULL;
		path_nodes.clear();
	}

	void DebugDrawPath()
	{
		const uint num_nodes = path_nodes.size();
		const uint num_samples = 50;
		const Vector4 line_col = Vector4(1.0f, 0.0f, 1.0f, 0.5f);
		for (uint nde = 0; nde < num_nodes; ++nde)
		{
			Vector3 end, start = path_nodes[nde]->Render()->GetTransform().GetPositionVector();

			for (uint i = 1; i <= num_samples; ++i)
			{
				float factor = (i / (float)num_samples);
				switch (interpolation_type_pos)
				{
				default:
				case 0: //Linear
				{
					end = InterpolatePositionLinear(
						nde,
						(nde + 1) % num_nodes,
						factor);
				}
				break;
				case 1: //Cubic
				{
					end = InterpolatePositionCubic(
						(nde + num_nodes - 1) % num_nodes,
						nde,
						(nde + 1) % num_nodes,
						(nde + 2) % num_nodes,
						factor);
				}
				break;
				}

				NCLDebug::DrawThickLineNDT(start, end, 0.05f, line_col);
				start = end;
			}
		}
	}
};