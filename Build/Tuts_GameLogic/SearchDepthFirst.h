#pragma once

#include "SearchAlgorithm.h"
#include <nclgl\common.h>
#include <deque>
#include <algorithm>

typedef std::pair<const GraphNode*, uint> SearchMemory;

class SearchDepthFirst : public SearchAlgorithm
{
public:
	SearchDepthFirst() 
		: SearchAlgorithm()
	{

	}

	virtual ~SearchDepthFirst()
	{

	}

	virtual bool FindBestPath(const GraphNode* start, const GraphNode* goal) override
	{
		//This algorithm search the graph from the bottom up. By traversing down the 
		// graph as far as possible then backtracking to try other nodes after exhausting
		// all the leaves of the search space. This performs brilliantly if the goal is 
		// (by luck?) directly in the depth first's initial traversal path. Though without
		// any guidance (see A*) it is not very effecient. This is the opposite to the
		// BreadthFirst search algorithm.
		// The depth first algorithm will always find a path.. and it might get there
		// very quickly.
		searchHistory.clear();
		finalPath.clear();

		//Stores the graph node and the index of the neighbour we were currently searching last
		std::deque<SearchMemory> stack; 
		std::deque<const GraphNode*> closed_set;

		stack.push_back({ start, 0 });

		SearchMemory *current;
		while (stack.size() > 0)
		{
			current = &stack.back();

			const GraphNode* node = current->first;

			if (stack.size() > 1 && current->second == 0)
				searchHistory.push_back({ node, stack[stack.size() - 2].first });

			//Have we reached the goal yet?
			if (node == goal)
			{
				BuildPathFromStack(stack);
				return true;
			}

			//Grab the next child from the current node and traverse down as far as possible
			// (or until we loop around on ourselves)
			uint i = current->second;
			if (i < node->_neighbours.size())
			{
				GraphNode* nextNode = NULL;
				while (!nextNode && current->second < node->_neighbours.size())
				{
					GraphEdge* e = node->_neighbours[current->second];
					current->second++;

					//Get the 'other' node that this edge connects to	
					GraphNode* otherNode = (e->_a == node) ? e->_b : e->_a;

					//Make sure we haven't visted the neighbor already
					if (std::find(closed_set.begin(), closed_set.end(), otherNode) != closed_set.end())
						continue;

					//Make sure the node isn't already in the stack (e.g. the parent, or a loop)
					auto itr = std::find_if(stack.begin(), stack.end(), [otherNode](const SearchMemory& s) {return s.first == otherNode; });
					if (itr != stack.end())
						continue;

					nextNode = otherNode;
				}
				if (nextNode)
					stack.push_back({ nextNode, 0 });
			}
			else
			{
				closed_set.push_back(node);
				stack.pop_back();
			}
		}

		return false;
	}


	void BuildPathFromStack(const std::deque<SearchMemory>& stack)
	{
		//To build a final path for depth first we can just walk back up the stack
		for (const SearchMemory& mem : stack)
		{
			finalPath.push_front(mem.first);
		}
	}
};