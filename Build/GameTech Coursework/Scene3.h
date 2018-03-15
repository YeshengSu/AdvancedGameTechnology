#pragma once

#include <nclgl\NCLDebug.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\CommonMeshes.h>
#include <nclgl\OBJMesh.h>

class Scene3 : public Scene
{
public:
	Scene3(const std::string& friendly_name);

	virtual ~Scene3();

	virtual void OnInitializeScene() override;

	virtual void OnUpdateScene(float dt) override;

private :
	OBJMesh* dinosaurMesh;

	void CreateBarrier();

	GameObject* CreateDinosaur(Vector3 pos);

	void ShootDinosaur(float velocityScalar);


};