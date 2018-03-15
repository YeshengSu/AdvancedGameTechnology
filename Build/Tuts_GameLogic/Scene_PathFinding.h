#pragma once

#include <ncltech\Scene.h>
#include "MazeGenerator.h"
#include "MazeRenderer.h"
#include "SearchAStar.h"
#include "SearchBreadthFirst.h"
#include "SearchDepthFirst.h"
#include <ncltech\CommonUtils.h>
#include <nclgl\OBJMesh.h>
#include <deque>
#include <list>
#include <unordered_map>

const Vector3 pos_maze1 = Vector3(-3.f, 0.f, -3.f);
const Vector3 pos_maze2 = Vector3(3.f, 0.f, -3.f);
const Vector3 pos_maze3 = Vector3(0.f, 0.f, 3.f);



//This class is mostly just debug/rendering chaff.
//The Maze graph is created via the MazeGenerator class, and is then
// traversed by one of the corresponding search algorithms. See SearchAStar.h
// of SearchBreadthFrist.h etc for actual implementation examples.
class Scene_PathFinding : public Scene
{
protected:
	MazeGenerator*	generator;
	MazeRenderer*	mazes[3];

	SearchDepthFirst*	search_df;
	SearchBreadthFirst* search_bf;
	SearchAStar*		search_as;

	int grid_size;
	float density;
	int astar_preset_idx;
	std::string astar_preset_text;

	Mesh* wallmesh;
public:
	void UpdateAStarPreset()
	{
		//Example presets taken from:
		// http://movingai.com/astar-var.html
		float weightingG, weightingH;
		switch (astar_preset_idx)
		{
		default:
		case 0:
			//Only distance from the start node matters - fans out from start node
			weightingG = 1.0f;
			weightingH = 0.0f;
			astar_preset_text = "Dijkstra - None heuristic search";
			break;
		case 1:
			//Only distance to the end node matters
			weightingG = 0.0f;
			weightingH = 1.0f;
			astar_preset_text = "Pure Hueristic search";
			break;
		case 2:
			//Equal weighting
			weightingG = 1.0f;
			weightingH = 1.0f;
			astar_preset_text = "Traditional A-Star";
			break;
		case 3:
			//Greedily goes towards the goal node where possible, but still cares about distance travelled a little bit
			weightingG = 1.0f;
			weightingH = 2.0f;
			astar_preset_text = "Weighted Greedy A-Star";
			break;
		}
		search_as->SetWeightings(weightingG, weightingH);

		GraphNode* start = generator->GetStartNode();
		GraphNode* end = generator->GetGoalNode();
		search_as->FindBestPath(start, end);
	}



	Scene_PathFinding(const std::string& friendly_name)
		: Scene(friendly_name)
		, grid_size(16)
		, density(0.5f)
		, astar_preset_idx(2)
		, astar_preset_text("")
		, search_df(new SearchDepthFirst())
		, search_bf(new SearchBreadthFirst())
		, search_as(new SearchAStar())
		, generator(new MazeGenerator())
	{
		wallmesh = new OBJMesh(MESHDIR"cube.obj");

		GLuint whitetex;
		glGenTextures(1, &whitetex);
		glBindTexture(GL_TEXTURE_2D, whitetex);
		unsigned int pixel = 0xFFFFFFFF;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
		glBindTexture(GL_TEXTURE_2D, 0);

		wallmesh->SetTexture(whitetex);

		srand(93225); //Set the maze seed to a nice consistent example :)
	}

	virtual ~Scene_PathFinding()
	{
		SAFE_DELETE(wallmesh);
	}



	virtual void OnInitializeScene() override
	{
		GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(-1.5, 25, 1));
		GraphicsPipeline::Instance()->GetCamera()->SetPitch(-80);
		GraphicsPipeline::Instance()->GetCamera()->SetYaw(0);
		
		GenerateNewMaze();
	}


	void GenerateNewMaze()
	{
		this->DeleteAllGameObjects(); //Cleanup old mazes

		generator->Generate(grid_size, density);

		//The maze is returned in a [0,0,0] - [1,1,1] cube (with edge walls outside) regardless of grid_size,
		// so we need to scale it to whatever size we want
		Matrix4 maze_scalar = Matrix4::Scale(Vector3(5.f, 5.0f / float(grid_size), 5.f)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));

		mazes[0] = new MazeRenderer(generator, wallmesh);
		mazes[0]->Render()->SetTransform(Matrix4::Translation(pos_maze1) * maze_scalar);
		this->AddGameObject(mazes[0]);

		mazes[1] = new MazeRenderer(generator, wallmesh);
		mazes[1]->Render()->SetTransform(Matrix4::Translation(pos_maze2) * maze_scalar);
		this->AddGameObject(mazes[1]);

		mazes[2] = new MazeRenderer(generator, wallmesh);
		mazes[2]->Render()->SetTransform(Matrix4::Translation(pos_maze3) * maze_scalar);
		this->AddGameObject(mazes[2]);

		//Create Ground (..we still have some common ground to work off)
		GameObject* ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));

		this->AddGameObject(ground);

		GraphNode* start = generator->GetStartNode();
		GraphNode* end = generator->GetGoalNode();

		search_df->FindBestPath(start, end);
		search_bf->FindBestPath(start, end);
		UpdateAStarPreset();
	}

	

	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   [G] To generate a new maze", grid_size);
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Grid Size : %2d ([1]/[2] to change)", grid_size);
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Density : %2.0f percent ([3]/[4] to change)", density * 100.f);
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   A-Star Type: \"%s\" ([C] to cycle)", astar_preset_text.c_str());
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "----------------");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Demonstration of path finding algorithms, and hopefully why A* is");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "so good at the job. See if you can figure out what the pros and cons");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "are of the various A* presets.");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G))
		{
			GenerateNewMaze();
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
		{
			grid_size++;
			GenerateNewMaze();
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
		{
			grid_size = max(grid_size - 1, 2);
			GenerateNewMaze();
		}
		
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))
		{
			density = min(density + 0.1f, 1.0f);
			GenerateNewMaze();
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4))
		{
			density = max(density - 0.1f, 0.0f);
			GenerateNewMaze();
		}
		
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_C))
		{
			astar_preset_idx = (astar_preset_idx + 1) % 4;
			UpdateAStarPreset();
		}

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Number of nodes searched ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Depth-First  : %4d nodes", search_df->GetSearchHistory().size());
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Breadth-First: %4d nodes", search_bf->GetSearchHistory().size());
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   A-Star       : %4d nodes", search_as->GetSearchHistory().size());

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Final path length ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Depth-First  : %3d", search_df->GetFinalPath().size() - 1);
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Breadth-First: %3d", search_bf->GetFinalPath().size() - 1);
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   A-Star       : %3d", search_as->GetFinalPath().size() - 1);

		NCLDebug::DrawTextWsNDT(pos_maze1 - Vector3(0, 0, 3.1f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "Depth First");
		NCLDebug::DrawTextWsNDT(pos_maze2 - Vector3(0, 0, 3.1f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "Breadth First");
		NCLDebug::DrawTextWsNDT(pos_maze3 + Vector3(0, 0, 3.1f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0, 0, 0, 1), "A-Star");

		mazes[0]->DrawSearchHistory(search_df->GetSearchHistory(), 2.5f / float(grid_size));
		mazes[1]->DrawSearchHistory(search_bf->GetSearchHistory(), 2.5f / float(grid_size));
		mazes[2]->DrawSearchHistory(search_as->GetSearchHistory(), 2.5f / float(grid_size));
	}
};