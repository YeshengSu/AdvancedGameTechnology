#include "Octree.h"
#include <nclgl\NCLDebug.h>
#include <Queue>
using namespace std;

Octree::Octree(void)
{
	nLevel = 0;
	nLevels = 4;
	pRoot = nullptr;
}

Octree::~Octree(void)
{
	queue<TreeNode*> q;
	q.push(pRoot);
	while (!q.empty())
	{
		TreeNode *pParent = q.front();
		q.pop();
		if (pParent->hasChildren)
		{
			for (int i = 0; i < 8; i++)
			{
				q.push(pParent->pChildren[i]);
			}
		}
		delete pParent;
	}

}

void Octree::CreateTree(Vector3 originPos, Vector3 halfDims, int level)
{
	if (pRoot != nullptr)
		return;
	nLevels = level;
	pRoot = new TreeNode();
	pRoot->cube.halfDims = halfDims;
	pRoot->cube.pos = originPos;
	_CreateTree(pRoot);
}

bool Octree::AddItem(PhysicsNode * physicsNode)
{
	return _AddItem(pRoot, physicsNode);
}

bool Octree::RemoveItem(PhysicsNode * physicsNode)
{
	return _RemoveItem(pRoot, physicsNode);
}

void Octree::RemoveAllItem()
{
	_RemoveAllItem(pRoot);
	allPhysicsNodes.clear();
}

void Octree::_CreateTree(TreeNode * pRoot)
{
	if (pRoot == nullptr)
		return;
	this->pRoot = pRoot;
	nLevel = 1;
	nCount = 0;
	queue<TreeNode*> q;
	q.push(pRoot);

	while (!q.empty())
	{

		TreeNode *pParent = q.front();
		q.pop();

		TreeCube cube = pParent->cube;
		int _x[] = { -1,-1, 1, 1,-1,-1, 1, 1 };
		int _y[] = {  1, 1, 1, 1,-1,-1,-1,-1 };
		int _z[] = { -1, 1, 1,-1,-1, 1, 1,-1 };
		for (int i = 0; i < 8; i++)
		{
			TreeNode *pNode = new TreeNode;

			TreeCube cube;
			cube.pos.x = pParent->cube.pos.x + _x[i] * pParent->cube.halfDims.x * 0.5f;
			cube.pos.y = pParent->cube.pos.y + _y[i] * pParent->cube.halfDims.y * 0.5f;
			cube.pos.z = pParent->cube.pos.z + _z[i] * pParent->cube.halfDims.z * 0.5f;
			cube.halfDims = Vector3(pParent->cube.halfDims.x * 0.5f, pParent->cube.halfDims.y * 0.5f, pParent->cube.halfDims.z * 0.5f);

			pNode->cube = cube;
			pNode->nLevel = nLevel;
			pNode->pParent = pParent;
			pParent->pChildren[i] = pNode;

			if (nLevel < nLevels)
			{
				q.push(pNode);
			}
		}

		nCount += 8;
		if (nCount >= pow(8.0, nLevel))
		{
			nLevel++;
			nCount = 0;
		}
		pParent->hasChildren = true;
	}
}

bool Octree::_AddItem(TreeNode *pNode, PhysicsNode *physicsNode)
{
	TreeNode *pChild = 0;

	if (physicsNode->GetCollisionShape().size() == 0)
		return false;

	if (_IsInTreeCube(physicsNode, pNode->cube))
	{
		if (pNode->hasChildren)
		{
			vector<TreeNode*> vecChildren;
			for (int i = 0; i < 8; i++)
			{
				if (_IsInTreeCube(physicsNode, pNode->pChildren[i]->cube))
				{
					vecChildren.push_back(pNode->pChildren[i]);
					pChild = pNode->pChildren[i];
				}
			}

			int nCount = 0;
			nCount = vecChildren.size();

			if (vecChildren.size() > 1)
			{
				allPhysicsNodes[physicsNode] = pNode;
				pNode->listPhysicsNode.push_back(physicsNode);
				return true;
			}
			else
			{
				if (pChild == nullptr)
				{
					allPhysicsNodes[physicsNode] = nullptr;
					return false;
				}
				else if (_AddItem(pChild, physicsNode))
				{
					return true;
				}
			}
		}
		else
		{
			allPhysicsNodes[physicsNode] = pNode;
			pNode->listPhysicsNode.push_back(physicsNode);
			return true;
		}
	}
	else
	{
		allPhysicsNodes[physicsNode] = nullptr;
	}
	return false;


}
bool Octree::_RemoveItem(TreeNode *pNode, PhysicsNode *physicsNode)
{
	if (physicsNode->GetCollisionShape().size() == 0)
		return false;

	if (_IsInTreeCube(physicsNode, pNode->cube))
	{
		if (pNode->listPhysicsNode.size() > 0)
		{
			for (vector<PhysicsNode*>::iterator ite = pNode->listPhysicsNode.begin(); ite != pNode->listPhysicsNode.end(); ++ite)
			{

				if ((*ite) == physicsNode)
				{
					vector<PhysicsNode*>::iterator iter = pNode->listPhysicsNode.erase(ite);

					map<PhysicsNode*, TreeNode*>::iterator it = allPhysicsNodes.find(physicsNode);
					if (it != allPhysicsNodes.end())
						allPhysicsNodes.erase(it);
					else
					{
						cout << 11 << endl;
					}
					return true;
				}
			}
		}
		

		if (pNode->hasChildren)
		{
			for (int i = 0; i < 8; i++)
			{
				if (_RemoveItem(pNode->pChildren[i], physicsNode))
				{
					return true;
				}
			}
		}
	}
	return false;

}

void Octree::_RemoveAllItem(TreeNode *pNode)
{
	if (pNode->hasChildren)
	{
		for (int i = 0; i < 8; i++)
		{
			_RemoveAllItem(pNode->pChildren[i]);
		}
	}
	pNode->listPhysicsNode.clear();
}

bool Octree::_IsInTreeCube(PhysicsNode * physicsNode, TreeCube &cube)
{
	float radius = physicsNode->GetBoundingRadius();
	Vector3 len = (cube.pos - physicsNode->GetPosition());
	Vector3 halfDims = cube.halfDims;
	if (abs(len.x) <= abs(halfDims.x + radius) &&
		abs(len.y) <= abs(halfDims.y + radius) &&
		abs(len.z) <= abs(halfDims.z + radius) )
	{
		return true;
	}
	return false;
	
}

int Octree::CollisionDetection(vector<CollisionPair>& broadphaseColPairs)
{
	nCount = 0;

	nCount += _CollisionInANode(vector<PhysicsNode*>(), pRoot->listPhysicsNode, broadphaseColPairs);
	for (int i = 0; i < 8; i++)
	{
		nCount += _CollisionDetection(pRoot->pChildren[i], pRoot->listPhysicsNode, broadphaseColPairs);
	}

	//nCount = _CollisionDetection(pRoot, pRoot->listPhysicsNode, broadphaseColPairs);
	return nCount;
}

int Octree::_CollisionDetection(TreeNode *pNode, vector<PhysicsNode*> &vecCmpPhysicsNodes, vector<CollisionPair>& broadphaseColPairs)
{
	int nNum = 0;
	if (pNode->listPhysicsNode.size() > 0)
		nNum += _CollisionInANode(vecCmpPhysicsNodes, pNode->listPhysicsNode, broadphaseColPairs);

	if (pNode->hasChildren)
	{
		//nNum += _CollisionInANode(pNode->listPhysicsNode);
		vector<PhysicsNode*> vecPhysicsNodes[8];

		// push physics nodes of parent node
		for (int i = 0; i < (int)vecCmpPhysicsNodes.size(); i++)
		{
			PhysicsNode *physicsNode = vecCmpPhysicsNodes[i];
			for (int j = 0; j < 8; j++)
			{
				if (_IsInTreeCube(physicsNode, pNode->pChildren[j]->cube))
				{
					vecPhysicsNodes[j].push_back(physicsNode);
				}
			}
		}

		// push physics nodes of current node
		for (int i = 0; i < (int)pNode->listPhysicsNode.size(); i++)
		{
			PhysicsNode *physicsNode = pNode->listPhysicsNode[i];
			for (int j = 0; j < 8; j++)
			{
				if (_IsInTreeCube(physicsNode, pNode->pChildren[j]->cube))
				{
					vecPhysicsNodes[j].push_back(physicsNode);
				}
			}
		}

		for (int i = 0; i < 8; i++)
		{
			nNum += _CollisionDetection(pNode->pChildren[i], vecPhysicsNodes[i], broadphaseColPairs);
		}

	}
	return nNum;

}

int Octree::_CollisionInANode(std::vector<PhysicsNode*> &ParentPhysicsNodes, std::vector<PhysicsNode*> &vecCmpPhysicsNodes, std::vector<CollisionPair>& broadphaseColPairs)
{
	// two Individual broad phase
	PhysicsNode *pnodeA, *pnodeB;
	Vector3		posA, posB;
	float		boundA, boundB;
		
	int num = 0;
	if (ParentPhysicsNodes.size() > 0)
	{
		for (int i = 0; i < (int)ParentPhysicsNodes.size(); i++)
		{
			for (int j = 0; j < (int)vecCmpPhysicsNodes.size(); j++)
			{
				pnodeA = ParentPhysicsNodes[i];
				pnodeB = vecCmpPhysicsNodes[j];

				//Check they both atleast have collision shapes
				if (pnodeA->GetCollisionShape().size() != 0
					&& pnodeB->GetCollisionShape().size() != 0)
				{
					posA = pnodeA->GetPosition();
					posB = pnodeB->GetPosition();
					boundA = pnodeA->GetBoundingRadius();
					boundB = pnodeB->GetBoundingRadius();
					float length = (posA - posB).Length();
					float closestDis = (boundA + boundB);
					if (length <= closestDis)
					{
						CollisionPair cp;
						cp.pObjectA = pnodeA;
						cp.pObjectB = pnodeB;
						broadphaseColPairs.push_back(cp);
						num++;
					}
				}
			}
		}
	}

	for (int i = 0; i < (int)vecCmpPhysicsNodes.size()-1; i++)
	{
		for (int j = i+1; j < (int)vecCmpPhysicsNodes.size(); j++)
		{
			pnodeA = vecCmpPhysicsNodes[i];
			pnodeB = vecCmpPhysicsNodes[j];

			//Check they both atleast have collision shapes
			if (pnodeA->GetCollisionShape().size() != 0
				&& pnodeB->GetCollisionShape().size() != 0)
			{
				posA = pnodeA->GetPosition();
				posB = pnodeB->GetPosition();
				boundA = pnodeA->GetBoundingRadius();
				boundB = pnodeB->GetBoundingRadius();
				float length = (posA - posB).Length();
				float closestDis = (boundA + boundB);
				if (length <= closestDis)
				{
					CollisionPair cp;
					cp.pObjectA = pnodeA;
					cp.pObjectB = pnodeB;
					broadphaseColPairs.push_back(cp);
					num++;
				}
			}
		}
	}

	return num;
}

void Octree::Update()
{
	//for (map<PhysicsNode*, TreeNode*>::iterator iter = allPhysicsNodes.begin(); iter != allPhysicsNodes.end(); ++iter)
	//{
	//	TreeNode *pNode = (TreeNode*)iter->second;
	//	PhysicsNode *physicsNode = (PhysicsNode*)iter->first;
	//
	//	if (pNode != nullptr)
	//		pNode->listPhysicsNode.clear();
	//
	//}
	_RemoveAllItem(pRoot);

	for (map<PhysicsNode*, TreeNode*>::iterator ite = allPhysicsNodes.begin(); ite != allPhysicsNodes.end(); ++ite)
	{
		bool ret = AddItem(ite->first);

	}
}

void Octree::DebugDrawTree() const
{
	queue<TreeNode*> q;
	q.push(pRoot);
	while (!q.empty())
	{
		TreeNode *pParent = q.front();
		q.pop();
		if (pParent->hasChildren)
		{
			for (int i = 0; i < 8; i++)
			{
				q.push(pParent->pChildren[i]);
			}
		}
		NCLDebug::DrawBox(pParent->cube.pos,pParent->cube.halfDims,Vector4(0.6f,0.3f,0.6f,1.0f));
	}
}
