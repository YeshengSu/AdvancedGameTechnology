#pragma once

#include "SearchAlgorithm.h"
#include <nclgl\common.h>
#include <deque>
#include <algorithm>
#include <map>

class SearchBreadthFirst : public SearchAlgorithm
{
public:
	SearchBreadthFirst()
		: SearchAlgorithm()
	{

	}

	virtual ~SearchBreadthFirst()
	{

	}

	virtual bool FindBestPath(const GraphNode* start, const GraphNode* goal) override
	{
		//This algorithm search the graph radially outwards from it's start node
		// If the goal node is nearby the start then this will be pretty effecient,
		// but the further away the goal is the number of nodes searched increases
		// dramatically. This is the exact opposite to the SearchDepthFirst algorithm.
		//The breadth first algorithm will always find the shortest path!
		searchHistory.clear();
		finalPath.clear();

		ParentMap came_from; //For working out final 'best' path

		std::deque<const GraphNode*> open_set;
		std::deque<const GraphNode*> closed_set;

		open_set.push_back(start);

		const GraphNode *parent = NULL;
		while (open_set.size() > 0)
		{
			parent = open_set.front();
			open_set.pop_front();

			if (came_from[parent] != NULL) searchHistory.push_back({ parent, came_from[parent] });

			//Have we reached our goal yet?
			if (parent == goal)
			{
				BuildPathFromParentMap(start, goal, came_from);
				return true;
			}

			for (GraphEdge* e : parent->_neighbours)
			{
				//Get the 'other' node that this edge connects to	
				GraphNode* otherNode = (e->_a == parent) ? e->_b : e->_a;

				//Make sure we haven't visted the neighbor already
				if (std::find(closed_set.begin(), closed_set.end(), otherNode) != closed_set.end())
					continue;

				//Make sure we haven't already listed the neighbour for visiting
				if (std::find(open_set.begin(), open_set.end(), otherNode) == open_set.end())
				{
					came_from[otherNode] = parent;
					open_set.push_back(otherNode);
				}

			}
			closed_set.push_back(parent);
		}

		return false;
	}

};