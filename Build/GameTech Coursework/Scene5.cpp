
#include "Scene5.h"


Scene5::Scene5(const std::string& friendly_name)
	: Scene(friendly_name)
{}

void Scene5::OnInitializeScene()
{
	height = 5.0f;
	width = 0.5f;
	size = 10.0f;
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

	CreateBalls();



}

void Scene5::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	// You can print text using 'printf' formatting
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
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
		m_Sphere->Physics()->SetElasticity(0.4f);
		m_Sphere->Physics()->SetPosition(cam->GetPosition());
		m_Sphere->Physics()->SetLinearVelocity(cam->GetDirection()*30.0f);
		m_Sphere->Physics()->SetOrientation(Quaternion());
		m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));
		CollisionShape* pColshape = new SphereCollisionShape(radius.x);
		m_Sphere->Physics()->SetCollisionShape(pColshape);
		m_Sphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(1.f));

		ScreenPicker::Instance()->RegisterNodeForMouseCallback(
			sphereRender, //Dummy is the rendernode that actually contains the drawable mesh
			std::bind(&CommonUtils::DragableObjectCallback, m_Sphere, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

		this->AddGameObject(m_Sphere);

	}

}

void Scene5::CreateBalls()
{
	for (int i = 0; i < 100; i++)
	{
		int x = (size-width) - (rand() / (float)RAND_MAX)* (size - width) * 2;
		int z = (size - width) - (rand() / (float)RAND_MAX)* (size - width) * 2;
		int y = (height)*(rand() / (float)RAND_MAX) + 1.3f;
		Vector3 pos = Vector3(x, y, z);
		CreateBall(pos);
	}
}

void Scene5::CreateBall(Vector3 pos)
{
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
	m_Sphere->Physics()->SetElasticity(0.4f);
	m_Sphere->Physics()->SetPosition(pos);
	m_Sphere->Physics()->SetLinearVelocity(Vector3(rand() / (float)RAND_MAX, 0.0f, rand() / (float)RAND_MAX)*(rand() / (float)RAND_MAX)*15);
	m_Sphere->Physics()->SetOrientation(Quaternion());
	CollisionShape* pColshape = new SphereCollisionShape(radius.x);
	m_Sphere->Physics()->SetCollisionShape(pColshape);
	m_Sphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(1.f));

	ScreenPicker::Instance()->RegisterNodeForMouseCallback(
		sphereRender, //Dummy is the rendernode that actually contains the drawable mesh
		std::bind(&CommonUtils::DragableObjectCallback, m_Sphere, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
	);

	this->AddGameObject(m_Sphere);
}

void Scene5::CreateBarrier()
{
	Vector4 color = Vector4(0.2f, 0.2f, 0.2f, 0.5f);

	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(0.0f, 1.0f, size),
		Vector3(size, height, width),
		true,
		0.0f,
		true,
		false,
		color));
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(0.0f, 1.0f, -size),
		Vector3(size, height, width),
		true,
		0.0f,
		true,
		false,
		color));
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(-size, 1.0f, 0.0f),
		Vector3(width, height, size),
		true,
		0.0f,
		true,
		false,
		color));
	//Create barrier
	this->AddGameObject(CommonUtils::BuildCuboidObject(
		"Barrier",
		Vector3(size, 1.0f, 0.0f),
		Vector3(width, height, size),
		true,
		0.0f,
		true,
		false,
		color));
}
