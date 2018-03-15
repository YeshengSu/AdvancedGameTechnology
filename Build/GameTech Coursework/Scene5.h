#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>
#include "ncltech\SphereCollisionShape.h"
#include "ncltech\CuboidCollisionShape.h"
#include <ncltech\CommonMeshes.h>
#include <ncltech\ScreenPicker.h>
#include <functional>
class Scene5 : public Scene
{
public:
	Scene5(const std::string& friendly_name);

	virtual void OnInitializeScene() override;

	virtual void OnUpdateScene(float dt) override;

private:
	void CreateBalls();
	void CreateBall(Vector3 pos);
	void CreateBarrier();

	float height;
	float width;
	float size;
};