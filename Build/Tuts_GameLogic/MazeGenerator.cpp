#include "MazeGenerator.h"
#include <nclgl\NCLDebug.h>

#include <list>
#include <algorithm>

uint RandomGridCell(uint size)
{
	uint x = rand() % size;
	uint y = rand() % size;
	return y * size + x;
}

MazeGenerator::MazeGenerator()
	: size(0)
	, start(NULL)
	, end(NULL)
	, allNodes(NULL)
	, allEdges(NULL)
{
}

MazeGenerator::~MazeGenerator()
{
	if (allNodes)
	{
		delete[] allNodes;
		delete[] allEdges;
		allNodes = NULL;
		allEdges = NULL;
	}
}

void MazeGenerator::Generate(int grid_size, float maze_density)
{
	if (allNodes)
	{
		delete[] allNodes;
		delete[] allEdges;
	}

	size = grid_size;
	Initiate_Arrays();

	Generate_Sparse(maze_density);

	//Final step is de-randomise the neighbours list
	// - This isn't normally needed, but for the sake of the maze demonstration it is nice to see that
	//   breadth first always does '+x, -x, +y, -y' and depth first always searches all '+x'then all '-x' etc
	// Order of neighbours: 
	//   0: -x
	//   1: +x
	//   2: -y
	//   3: +y
#pragma omp parallel for
	for (int y = 0; y < (int)size; ++y)
	{
		GraphEdge* lookup[4];
		for (int x = 0; x < (int)size; ++x)
		{
			GraphNode* on = &allNodes[y * size + x];

			memset(lookup, 0, 4 * sizeof(GraphEdge*));

			for (GraphEdge* e : on->_neighbours)
			{
				GraphNode* nn = (e->_a == on) ? e->_b : e->_a;

				int xOffset = on->_pos.x > nn->_pos.x;
				int yOffset = (on->_pos.y > nn->_pos.y);
				if (on->_pos.x != nn->_pos.x)
					lookup[xOffset] = e;	 //0 or 1
				else
					lookup[yOffset + 2] = e; //2 or 3
			}

			on->_neighbours.clear();

			for (int i = 0; i < 4; ++i)
			{
				if (lookup[i])
					on->_neighbours.push_back(lookup[i]);
			}

		}
	}

	GetRandomStartEndNodes();
}

void MazeGenerator::GetRandomStartEndNodes()
{
	//Traditional Maze one side to the other
	int edge = rand() % 2;
	int idxS = rand() % size;
	int idxE = rand() % size;
	switch (edge)
	{
	case 0: //x
		start = &allNodes[idxS * size];
		end = &allNodes[(idxE + 1) * size - 1];
		break;
	case 1: //y
		start = &allNodes[idxS];
		end = &allNodes[size * (size - 1) + idxE];
		break;
	}
}


void MazeGenerator::Initiate_Arrays()
{
	allNodes = new GraphNode[size * size];
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			allNodes[y * size + x]._pos = Vector3((float)x, (float)y, 0.0f);
			allNodes[y * size + x]._visited = false;
		}
	}

	uint base_offset = size * (size - 1);
	allEdges = new GraphEdge[base_offset * 2];

	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size - 1; ++x)
		{
			GraphEdge& edgeX = allEdges[(y * (size - 1) + x)];
			edgeX._a = &allNodes[y * size + x];
			edgeX._b = &allNodes[y * size + x + 1];
			edgeX.weighting = 1.0f;
			edgeX._connected = false;
			edgeX._iswall = true;
		}
	}
	for (uint y = 0; y < size - 1; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			GraphEdge& edgeY = allEdges[base_offset + (x * (size - 1) + y)];
			edgeY._a = &allNodes[y * size + x];
			edgeY._b = &allNodes[(y + 1) * size + x];
			edgeY.weighting = 1.0f;
			edgeY._connected = false;
			edgeY._iswall = true;
		}
	}
}

//Takes in a size parameter and generates a random square maze of dimensions: [sizexsize] 
// Uses Prims algorithm to generate the maze: https://en.wikipedia.org/wiki/Prim%27s_algorithm
void  MazeGenerator::Generate_Prims()
{
	uint startIdx = RandomGridCell(size);
	GraphNode* start = &allNodes[startIdx];
	start->_visited = true;

	uint base_offset = size * (size - 1);

	std::list<GraphEdge*> walls;


	auto add_potential_wall = [&](uint x1, uint y1, uint x2, uint y2)
	{
		GraphEdge* edge;

		if (x1 != x2)
		{
			//It's edge on the x-axis!
			edge = &allEdges[y1 * (size - 1) + min(x1, x2)];
		}
		else
		{
			//It's edge on the y-axis!	
			edge = &allEdges[base_offset + x1 * (size - 1) + min(y1, y2)];
		}

		if (!edge->_connected)
		{
			edge->_connected = true;
			walls.push_back(edge);
		}
	};

	auto add_walls = [&](uint node_idx)
	{
		uint x = node_idx % size;
		uint y = node_idx / size;

		if (y > 0) add_potential_wall(x, y, x, y - 1);	
		if (x > 0) add_potential_wall(x, y, x - 1, y);

		if (y < size - 1) add_potential_wall(x, y, x, y + 1);				
		if (x < size - 1) add_potential_wall(x, y, x + 1, y);
	};


	add_walls(startIdx);

	while (walls.size() > 0)
	{
		//Pick random wall in list
		uint idx = rand() % walls.size();
		auto itr = walls.begin();
		std::advance(itr, idx);
		GraphEdge* edge = *itr; 
		walls.erase(itr);

		if (edge->_a->_visited ^ edge->_b->_visited)
		{
			edge->_a->_neighbours.push_back(edge);
			edge->_b->_neighbours.push_back(edge);
			edge->_iswall = false;

			if (!edge->_a->_visited)
			{
				edge->_a->_visited = true;
				add_walls((edge->_a - allNodes));
			}
			else
			{
				edge->_b->_visited = true;
				add_walls((edge->_b - allNodes));
			}
		}
	}
}

void MazeGenerator::Generate_Sparse(float density)
{
	//Making a sparse maze is not so easy, as we still need to ensure
	// any node in the graph /can/ reach any other node. So to make
	// everything simpler, this just generates a complete maze and 
	// knocks out some walls at the end.
	Generate_Prims();

	//Build list of un-used edges
	std::deque<GraphEdge*> edges;

	uint base_offset = size * (size - 1);
	for (uint y = 0; y < size; ++y)
	{
		for (uint x = 0; x < size - 1; ++x)
		{
			GraphEdge* edgeX = &allEdges[(y * (size - 1) + x)];
			if (edgeX->_iswall) edges.push_back(edgeX);
		}
	}
	for (uint y = 0; y < size - 1; ++y)
	{
		for (uint x = 0; x < size; ++x)
		{
			GraphEdge* edgeY = &allEdges[base_offset + (x * (size - 1) + y)];
			if (edgeY->_iswall) edges.push_back(edgeY);
		}
	}


	std::random_shuffle(edges.begin(), edges.end());

	//Remove half the walls/edges in the maze
	int total_to_remove = (int)(floor((float)(edges.size() * (1.0f - density))));
	for (int i = 0; i < total_to_remove; ++i)
	{
		GraphEdge* e = edges.back();
		e->_a->_neighbours.push_back(e);
		e->_b->_neighbours.push_back(e);

		e->_iswall = false;
		edges.pop_back();
	}
}


