#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>
#include "ncltech\SphereCollisionShape.h"
#include "ncltech\CuboidCollisionShape.h"
#include "ncltech\EdgeCollisionShape.h"
#include <ncltech\CommonMeshes.h>
class Scene2 : public Scene
{
public:
	Scene2(const std::string& friendly_name);
	virtual ~Scene2();
	virtual void OnInitializeScene() override;

	virtual void OnUpdateScene(float dt) override;

private:
	void CreateTarget(string name, Vector3 Pos, Vector3 Scele, Vector4 color, PhysicsCollisionCallback callback);

	bool IncreaseScore(PhysicsNode* self, PhysicsNode* collidingObject)
	{
		if (collidingObject->GetParent() != nullptr)
		{
			if (collidingObject->GetParent()->GetName() == "projectile")
				m_Score += 100;
		}
		return true;
	}
	bool DecreaseScore(PhysicsNode* self, PhysicsNode* collidingObject)
	{
		if (collidingObject->GetParent() != nullptr)
		{
			if (collidingObject->GetParent()->GetName() == "projectile")
				m_Score -= 50;
		}
		return true;
	}

	int		m_Score;
	Mesh*	m_TargetMesh;
};