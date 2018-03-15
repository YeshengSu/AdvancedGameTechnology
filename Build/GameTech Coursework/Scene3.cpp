
#include "Scene3.h"

Scene3::Scene3(const std::string& friendly_name)
	: Scene(friendly_name)
{}

Scene3::~Scene3()
{
	delete dinosaurMesh;
}

void Scene3::OnInitializeScene()
{

	dinosaurMesh = new OBJMesh(MESHDIR"raptor.obj");

	//Create Ground
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(20.0f, 1.0f, 20.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

	CreateBarrier();

	//Create object
	CreateDinosaur(Vector3(0.0f,0.5f,-10.0f));

}

void Scene3::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	// You can print text using 'printf' formatting
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "	  - [H] shoot dinosaur");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "	  - [J] shoot ball");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Left click to move");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Right click to rotate");

	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
	{
		Camera* cam = GraphicsPipeline::Instance()->GetCamera();
		Vector3 radius = Vector3(0.3f, 0.3f, 0.3f);

		RenderNode* sphereRender = new RenderNode();
		sphereRender->SetMesh(CommonMeshes::Sphere());
		sphereRender->SetTransform(Matrix4::Scale(radius)); //No position! That is now all handled in PhysicsNode
		sphereRender->SetColor(Vector4(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1.0f));
		sphereRender->SetBoundingRadius(1.0f);

		GameObject* m_Sphere = new GameObject("projectile");
		m_Sphere->SetRender(new RenderNode());
		m_Sphere->Render()->AddChild(sphereRender);
		m_Sphere->SetPhysics(new PhysicsNode());
		m_Sphere->Physics()->SetInverseMass(0.2f);
		m_Sphere->Physics()->SetPosition(cam->GetPosition());
		m_Sphere->Physics()->SetLinearVelocity(cam->GetDirection()*30.0f);
		m_Sphere->Physics()->SetOrientation(Quaternion());
		m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));
		CollisionShape* pColshape = new SphereCollisionShape(radius.x);
		m_Sphere->Physics()->SetCollisionShape(pColshape);
		m_Sphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(1.f));
		this->AddGameObject(m_Sphere);

	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_H))
	{
		ShootDinosaur(20.0f);
	}

}

void Scene3::CreateBarrier()
{
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(0.0f, 1.0f, 20.0f),
		Vector3(20.0f, 2.0f, 1.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(0.0f, 1.0f, -20.0f),
		Vector3(20.0f, 2.0f, 1.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(-20.0f, 1.0f, 0.0f),
		Vector3(1.0f, 2.0f, 20.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(20.0f, 1.0f, 0.0f),
		Vector3(1.0f, 2.0f, 20.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

	const float width_scalar = 0.5f;
	const float height_scalar = 0.5f;
	const int stackHeight = 6;
	for (int y = 0; y < stackHeight; ++y)
	{
		for (int x = 0; x <= y; ++x)
		{
			Vector4 color = CommonUtils::GenColor(y * 0.2f, 1.0f);
			GameObject* cube = CommonUtils::BuildCuboidObject(
				"",
				Vector3((x * 1.1f - y * 0.5f) * width_scalar, (0.5f + float(stackHeight - 1) - y) * height_scalar, -0.5f),
				Vector3(width_scalar * 0.5f, height_scalar * 0.5f, 0.5f),
				true,
				1.f,
				true,
				true,
				color);
			cube->Physics()->SetElasticity(0.0f); //No elasticity (Little cheaty)
			cube->Physics()->SetFriction(1.0f);

			this->AddGameObject(cube);
		}
	}
}

GameObject* Scene3::CreateDinosaur(Vector3 pos)
{
	GameObject* dinosaur = new GameObject("Player1");
	dinosaur->SetRender(new RenderNode(dinosaurMesh));
	dinosaur->Render()->SetBoundingRadius(1.0f);
	dinosaur->Render()->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	dinosaur->SetPhysics(new PhysicsNode());
	dinosaur->Physics()->SetInverseMass(0.1f);
	dinosaur->Physics()->SetPosition(pos);
	dinosaur->Physics()->SetOrientation(Quaternion());

	CollisionShape* pColshape = new CuboidCollisionShape(Vector3(0.4f, 0.4f, 0.35f));
	pColshape->SetOffset(Vector3(0.0,-0.1f,-0.15f));
	dinosaur->Physics()->SetCollisionShape(pColshape);
	dinosaur->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(1.f));

	pColshape = new CuboidCollisionShape(Vector3(0.2f, 0.2f, 0.7f));
	pColshape->SetOffset(Vector3(0.0, 0.1f, 0.9f));
	dinosaur->Physics()->SetCollisionShape(pColshape);

	pColshape = new SphereCollisionShape(0.25f);
	pColshape->SetOffset(Vector3(0.0, 0.35f, -0.6f));
	dinosaur->Physics()->SetCollisionShape(pColshape);

	this->AddGameObject(dinosaur);

	return dinosaur;
}

void Scene3::ShootDinosaur(float velocityScalar)
{
	Camera* cam = GraphicsPipeline::Instance()->GetCamera();
	GameObject* obj = CreateDinosaur(cam->GetPosition());
	obj->Physics()->SetLinearVelocity(cam->GetDirection()*velocityScalar);
	obj->Physics()->SetAngularVelocity(Vector3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX)*3.0f);
}
