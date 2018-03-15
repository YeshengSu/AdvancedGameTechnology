
#include "Scene2.h"

Scene2::Scene2(const std::string& friendly_name)
	: Scene(friendly_name)
{
	m_Score = 0;

	GLuint tex = SOIL_load_OGL_texture(
		TEXTUREDIR"target.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	Mesh* cube = CommonMeshes::Cube();
	m_TargetMesh = new Mesh(*cube);
	m_TargetMesh->SetTexture(tex);
}

Scene2::~Scene2()
{
	SAFE_DELETE(m_TargetMesh);
}

void Scene2::OnInitializeScene()
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

	CreateTarget("GoodTarget", Vector3(0.0f+15.0f, 2.0f, 10.0f), Vector3(2.0f, 2.0f, 0.01f), Vector4(1.0f, 0.8f, 0.8f, 1.0f), 
		std::bind(&Scene2::IncreaseScore,this,	std::placeholders::_1,std::placeholders::_2));
	CreateTarget("BadTarget", Vector3(0.0f + 00.0f, 2.0f, 10.0f), Vector3(2.0f, 2.0f, 0.01f), Vector4(0.8f, 0.8f, 1.0f, 1.0f),
		std::bind(&Scene2::DecreaseScore, this, std::placeholders::_1, std::placeholders::_2));
	CreateTarget("GoodTarget", Vector3(0.0f - 15.0f, 2.0f, 10.0f), Vector3(2.0f, 2.0f, 0.01f), Vector4(1.0f, 0.8f, 0.8f, 1.0f),
		std::bind(&Scene2::IncreaseScore, this, std::placeholders::_1, std::placeholders::_2));
	CreateTarget("BadTarget", Vector3(0.0f + 15.0f, 2.0f, -10.0f), Vector3(2.0f, 2.0f, 0.01f), Vector4(0.8f, 0.8f, 1.0f, 1.0f),
		std::bind(&Scene2::DecreaseScore, this, std::placeholders::_1, std::placeholders::_2));
	CreateTarget("GoodTarget", Vector3(0.0f + 00.0f, 2.0f, -10.0f), Vector3(2.0f, 2.0f, 0.01f), Vector4(1.0f, 0.8f, 0.8f, 1.0f),
		std::bind(&Scene2::IncreaseScore, this, std::placeholders::_1, std::placeholders::_2));
	CreateTarget("BadTarget", Vector3(0.0f - 15.0f, 2.0f, -10.0f), Vector3(2.0f, 2.0f, 0.01f), Vector4(0.8f, 0.8f, 1.0f, 1.0f),
		std::bind(&Scene2::DecreaseScore, this, std::placeholders::_1, std::placeholders::_2));

}

void Scene2::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	// You can print text using 'printf' formatting
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Controls ---");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "	  - [J] shoot ball");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Left click to move");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Right click to rotate");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Information ---");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "   - Current Score : %d", m_Score);

	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
	{
		Camera* cam = GraphicsPipeline::Instance()->GetCamera();
		Vector3 radius = Vector3(0.3f, 0.3f, 0.3f);

		RenderNode* sphereRender = new RenderNode();
		sphereRender->SetMesh(CommonMeshes::Sphere());
		sphereRender->SetTransform(Matrix4::Scale(radius)); //No position! That is now all handled in PhysicsNode
		sphereRender->SetColor(Vector4(rand()/(float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1.0f));
		sphereRender->SetBoundingRadius(1.0f);

		GameObject* m_Sphere = new GameObject("projectile");
		m_Sphere->SetRender(new RenderNode());
		m_Sphere->Render()->AddChild(sphereRender);
		m_Sphere->SetPhysics(new PhysicsNode());
		m_Sphere->Physics()->SetInverseMass(0.2f);
		m_Sphere->Physics()->SetPosition(cam->GetPosition());
		m_Sphere->Physics()->SetLinearVelocity(cam->GetDirection()*20.0f);
		m_Sphere->Physics()->SetOrientation(Quaternion());
		m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));
		CollisionShape* pColshape = new SphereCollisionShape(radius.x);
		m_Sphere->Physics()->SetCollisionShape(pColshape);
		m_Sphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(1.f));
		this->AddGameObject(m_Sphere);

	}

}

void Scene2::CreateTarget(string name, Vector3 Pos, Vector3 Scale,Vector4 color, PhysicsCollisionCallback callback)
{
	GameObject *target = nullptr;
	RenderNode* renderNode = nullptr;
	RenderNode* dummy = nullptr;
	PhysicsNode* physicsNode = nullptr;
	CollisionShape* pColshape = nullptr;

	dummy = new RenderNode();
	dummy->SetMesh(m_TargetMesh);
	dummy->SetTransform(Matrix4::Scale(Scale));
	dummy->SetColor(color);
	renderNode = new RenderNode();
	renderNode->AddChild(dummy);
	renderNode->SetTransform(Matrix4::Translation(Pos));
	renderNode->SetColor(color);
	renderNode->SetBoundingRadius(4.0f);

	physicsNode = new PhysicsNode();
	physicsNode->SetInverseMass(0.0f);
	pColshape = new EdgeCollisionShape(Vector2(Scale.x, Scale.y));
	physicsNode->SetCollisionShape(pColshape);
	physicsNode->SetInverseInertia(pColshape->BuildInverseInertia(0.0f));
	physicsNode->SetPosition(Pos);
	physicsNode->SetOnCollisionCallback(callback);
	target = new GameObject(name, renderNode, physicsNode);
	this->AddGameObject(target);
}

