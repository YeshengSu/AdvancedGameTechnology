
#include "Scene4.h"
using namespace std;
Scene4::Scene4(const std::string& friendly_name)
	: Scene(friendly_name)
{}

void Scene4::OnInitializeScene()
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

	CreateCloth();

}

void Scene4::OnUpdateScene(float dt)
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
		m_Sphere->Physics()->SetInverseMass(0.05f);
		m_Sphere->Physics()->SetPosition(cam->GetPosition());
		m_Sphere->Physics()->SetLinearVelocity(cam->GetDirection()*30.0f);
		m_Sphere->Physics()->SetOrientation(Quaternion());
		m_Sphere->Physics()->SetAngularVelocity(Vector3(0.f, 0.f, -2.0f * PI));
		CollisionShape* pColshape = new SphereCollisionShape(radius.x);
		m_Sphere->Physics()->SetCollisionShape(pColshape);
		m_Sphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(0.05f));
		this->AddGameObject(m_Sphere);

	}

}

void Scene4::CreateCloth()
{
	float transparent = 0.5f;
	float height = 10.0f;
	float heightDis = 0.5f;
	float widthDis = 0.5f;
	float CollisionRadius = 0.25f;
	int rawNum = 12;
	int clothLong = 12;

	GameObject *handle, *ball;
	vector<vector<GameObject *>> balls;
	for (int i = 0; i < clothLong; i++)
	{
		balls.push_back(vector<GameObject *>());
		for (int j = 0; j < rawNum; j++)
		{

			RenderNode* sphereRender = new RenderNode();
			sphereRender->SetMesh(CommonMeshes::Sphere());
			sphereRender->SetTransform(Matrix4::Scale(Vector3(0.1f,0.1f,0.1f))); //No position! That is now all handled in PhysicsNode
			sphereRender->SetColor(Vector4(1.0f,1.0f,1.0f,1.0f));
			sphereRender->SetBoundingRadius(5.0f);

			handle = new GameObject("projectile");
			handle->SetRender(new RenderNode());
			handle->Render()->AddChild(sphereRender);
			handle->SetPhysics(new PhysicsNode());
			handle->Physics()->SetInverseMass(i == 0 ? 0.0f : 0.1f);//mass
			handle->Physics()->SetPosition(Vector3(widthDis*j, height - heightDis*i, 0.0f));//pos
			CollisionShape* pColshape = new SphereCollisionShape(CollisionRadius);
			handle->Physics()->SetCollisionShape(pColshape);
			handle->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(5.0f));

			ScreenPicker::Instance()->RegisterNodeForMouseCallback(
				sphereRender, //Dummy is the rendernode that actually contains the drawable mesh
				std::bind(&CommonUtils::DragableObjectCallback, handle, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
			);

			this->AddGameObject(handle);
			balls[i].push_back(handle);
		}
	}

	for (int i = 0; i < clothLong; i++)
	{
		for (int j = 0; j < rawNum-1; j++)
		{
			//Add distance constraint between the two objects
			DistanceConstraint* constraint = new DistanceConstraint(
				balls[i][j]->Physics(),						//Physics Object A
				balls[i][j+1]->Physics(),					//Physics Object B
				balls[i][j]->Physics()->GetPosition(),		//Attachment Position on Object A	-> Currently the centre
				balls[i][j+1]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			PhysicsEngine::Instance()->AddConstraint(constraint);

		}
	}
	for (int j = 0; j < rawNum ; j++)
	{
		for (int i = 0; i < clothLong-1; i++)
		{
			//Add distance constraint between the two objects
			DistanceConstraint* constraint = new DistanceConstraint(
				balls[i][j]->Physics(),						//Physics Object A
				balls[i+1][j]->Physics(),					//Physics Object B
				balls[i][j]->Physics()->GetPosition(),		//Attachment Position on Object A	-> Currently the centre
				balls[i+1][j]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			PhysicsEngine::Instance()->AddConstraint(constraint);
		}
	}

	for (int j = 0; j < rawNum-1; j++)
	{
		for (int i = 0; i < clothLong-1; i++)
		{
			//Add distance constraint between the two objects
			DistanceConstraint* constraint = new DistanceConstraint(
				balls[i][j]->Physics(),						//Physics Object A
				balls[i+1][j+1]->Physics(),					//Physics Object B
				balls[i][j]->Physics()->GetPosition(),		//Attachment Position on Object A	-> Currently the centre
				balls[i+1][j+1]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			PhysicsEngine::Instance()->AddConstraint(constraint);
		}
	}

	for (int j = 1; j < rawNum; j++)
	{
		for (int i = 0; i < clothLong-1; i++)
		{
			//Add distance constraint between the two objects
			DistanceConstraint* constraint = new DistanceConstraint(
				balls[i][j]->Physics(),						//Physics Object A
				balls[i+1][j-1]->Physics(),					//Physics Object B
				balls[i][j]->Physics()->GetPosition(),		//Attachment Position on Object A	-> Currently the centre
				balls[i+1][j-1]->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			PhysicsEngine::Instance()->AddConstraint(constraint);
		}
	}
}
