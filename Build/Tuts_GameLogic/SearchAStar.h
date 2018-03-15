#pragma once

#include "SearchAlgorithm.h"
#include <nclgl\common.h>
#include <deque>
#include <algorithm>
#include <map>

class SearchAStar : public SearchAlgorithm
{
protected:
	float g_weighting;
	float h_weighting; //NEW!

public:
	SearchAStar()
		: SearchAlgorithm()
		, g_weighting(1.0f)
		, h_weighting(1.0f)
	{
	}
	virtual ~SearchAStar() {}

	void SetWeightings(float g, float h)
	{
		g_weighting = g;
		h_weighting = h;
	}

	float GetWeighting_G() const { return g_weighting; }
	float GetWeighting_H() const { return h_weighting; }



	float DistanceHueristic(const GraphNode* node, const GraphNode* goal)
	{
		//Two different distance measurements (Try both!!)
		Vector3 diff = goal->_pos - node->_pos;

		//return diff.Length();								//True omni directional system  (Euclidean distance)
		return fabs(diff.x) + fabs(diff.y) + fabs(diff.z);	//Grid based system				(Manhatten distance)
	};

	virtual bool FindBestPath(const GraphNode* start, const GraphNode* goal) override
	{
		//See Handout: AI 2: Path-Finding for more details.
		//https://en.wikipedia.org/wiki/A*_search_algorithm

		searchHistory.clear();
		finalPath.clear();

		std::list<const GraphNode*> open_set;
		std::list<const GraphNode*> closed_set;

		//If you know the size of search area, don't leave it to chance!!
		// - Use an unordered_map or direct mem allocation and get those sweet
		//   O(1) map lookup times! Alot of the AStar algorithm is searching
		//   back through our history, so being able to turn a search into a
		//   direct memory access will make the world of difference. For the
		//   maze, we could pass the size in as a parameter and use
		//   [size x size] (number of nodes) as our lookups.
		typedef std::map<const GraphNode*, float> gmap;
		typedef std::pair<const GraphNode*, float> gkeyvalue;

		ParentMap came_from; //For working out final 'best' path
		gmap g_score; //Distance from any node to the start node
		gmap f_score; //Distance from any node to the end node (partly known.. mostly estimation)

		g_score[start] = 0.0f;
		f_score[start] = DistanceHueristic(start, goal);
		open_set.push_back(start);

		const GraphNode *current = NULL;
		while (open_set.size() > 0)
		{
			//Find the node in the open_set with the lowest f_score
			auto min = min_element(open_set.begin(), open_set.end(), [&](const GraphNode* left, const GraphNode* right) {
				return f_score.at(left) < f_score.at(right);
			});

			current = *min;
			open_set.erase(min);

			if (came_from[current] != NULL) searchHistory.push_back({ current, came_from[current] });

			//Have we made it to the goal yet?
			if (current == goal)
			{
				BuildPathFromParentMap(start, goal, came_from);
				return true;
			}

			//Go down the node with the lowest f_score and explore all it's adjacent neighbours
			for (GraphEdge* e : current->_neighbours)
			{
				//Get the 'other' node that this edge connects to	
				GraphNode* otherNode = (e->_a == current) ? e->_b : e->_a;

				//Make sure we haven't visted the neighbor already
				if (std::find(closed_set.begin(), closed_set.end(), otherNode) != closed_set.end())
					continue;

				//Make sure we haven't already listed the neighbour previously
				if (std::find(open_set.begin(), open_set.end(), otherNode) == open_set.end())
				{
					open_set.push_back(otherNode);
				}

				//Distance from start to neighbour node
				float gscore = g_score[current] + e->weighting;

				//Check to see if the node has already been visited previously (and if our new path to it is more expensive)
				if (g_score.find(otherNode) != g_score.end() && gscore >= g_score[otherNode])
					continue; //More expensive route found - just leave it with the previous routes f_score

				came_from[otherNode] = current;
				g_score[otherNode] = gscore;

				// Different weightings have vastly different results, the default 1x1 is the traditional A* algorithm
				f_score[otherNode] = gscore * g_weighting + DistanceHueristic(otherNode, goal) * h_weighting;
			}

			closed_set.push_back(current);
		}

		return false;
	}

};