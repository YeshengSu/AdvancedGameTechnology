
#include "Scene1.h"

Scene1::Scene1(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_StackHeight(8)
{}

void Scene1::OnInitializeScene()
{
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

	//SOLVER EXAMPLE -> Pyramid of cubes stacked on top of eachother
	// Note: Also, i've just noticed we have only ever done cube's not cuboids.. 
	//       so, to prove to any non-believes, try turning the pyramid of cubes
	//       into rectangles :)
	const float width_scalar = 1.0f;
	const float height_scalar = 1.0f;

	for (int y = 0; y < m_StackHeight; ++y)
	{
		for (int x = 0; x <= y; ++x)
		{
			Vector4 color = CommonUtils::GenColor(y * 0.2f, 1.0f);
			GameObject* cube = CommonUtils::BuildCuboidObject(
				"",
				Vector3((x * 1.1f - y * 0.5f) * width_scalar, (0.5f + float(m_StackHeight - 1) - y) * height_scalar, -0.5f),
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

void Scene1::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	// You can print text using 'printf' formatting
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "	  - [J] shoot ball");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Stack Height : %2d ([1]/[2] to change)", m_StackHeight);
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Left click to move");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Right click to rotate");

	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
	{
		m_StackHeight++;
		SceneManager::Instance()->JumpToScene(SceneManager::Instance()->GetCurrentSceneIndex());
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
	{
		m_StackHeight = max(m_StackHeight - 1, 1);
		SceneManager::Instance()->JumpToScene(SceneManager::Instance()->GetCurrentSceneIndex());
	}

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

}