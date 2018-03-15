#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>
#include "ncltech\SphereCollisionShape.h"
#include "ncltech\CuboidCollisionShape.h"
#include <ncltech\CommonMeshes.h>
class Scene1 : public Scene
{
public:
	Scene1(const std::string& friendly_name);
	
	virtual void OnInitializeScene() override;

	virtual void OnUpdateScene(float dt) override;

private :
	int m_StackHeight;
};