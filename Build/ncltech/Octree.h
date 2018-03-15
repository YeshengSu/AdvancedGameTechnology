#pragma once
#include <list>
#include <map>
#include <vector>
#include "PhysicsNode.h"

struct CollisionPair	//Forms the output of the broadphase collision detection
{
	PhysicsNode* pObjectA;
	PhysicsNode* pObjectB;
};

class Octree
{
private:
	struct TreeCube
	{
		Vector3 pos;
		Vector3 halfDims;
	};

	struct TreeNode
	{
		int nLevel;
		std::vector<PhysicsNode *> listPhysicsNode;
		TreeNode *pChildren[8];
		bool hasChildren;
		TreeNode *pParent;
		TreeCube cube;
		TreeNode() {
			nLevel = 0;
			for (int i = 0; i < 8; i++)
				pChildren[i] = nullptr;
			hasChildren = false;
			pParent = nullptr;
		}
	};

public:
	Octree(void);
	~Octree(void);

	// has tree?
	bool IsHasTree()const { return pRoot == nullptr ? false : true; }

	// Create an octree
	void CreateTree(Vector3 originPos, Vector3 halfDims, int level = 4);

	// add physics node in tree
	bool AddItem(PhysicsNode *physicsNode);

	// remove physics node in tree
	bool RemoveItem(PhysicsNode *physicsNode);

	// remove All Item();
	void RemoveAllItem();

	// perform collision test
	int CollisionDetection(std::vector<CollisionPair>& broadphaseColPairs);

	// update tree 
	void Update();

	// Draw octree
	void DebugDrawTree()const;

private:

	// collision test with all physics nodes
	int _CollisionInANode(std::vector<PhysicsNode*> &ParentPhysicsNodes,std::vector<PhysicsNode*> &vecCmpPhysicsNodes, std::vector<CollisionPair>& broadphaseColPairs);

	// recursive function used to detect collision
	int _CollisionDetection(TreeNode *pNode, std::vector<PhysicsNode*> &vecCmpPhysicsNodes, std::vector<CollisionPair>& broadphaseColPairs);

	// recursive function used to create tree
	void _CreateTree(TreeNode *pRoot);

	// recursive function used to add physics node
	bool _AddItem(TreeNode *pNode, PhysicsNode *physicsNode);

	// recursive function used to remote node
	bool _RemoveItem(TreeNode *pNode, PhysicsNode *physicsNode);

	// remove All Item();
	void _RemoveAllItem(TreeNode *pNode);

	// determine is physics node in tree cube
	bool _IsInTreeCube(PhysicsNode* PhysicsNode, TreeCube &cube);

	int nLevel;
	int nLevels;
	TreeNode *pRoot;
	int nCount;
	std::list<TreeNode*> listLeaves;
	std::map<PhysicsNode*, TreeNode*> allPhysicsNodes;
};
