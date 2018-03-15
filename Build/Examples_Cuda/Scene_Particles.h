#include <ncltech\Scene.h>
#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

#include "RenderNodeParticles.h"
#include "CudaParticles.cuh"

using namespace CommonUtils;

class Scene_Particles : public Scene
{
public:
	Scene_Particles(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual ~Scene_Particles()
	{

	}


	virtual void OnInitializeScene() override
	{
		//<--- SCENE CREATION --->
		//Create Ground
		this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		//TODO: Make this changable at runtime
		num_particles = 30000;

		RenderNodeParticles* rnode = new RenderNodeParticles();
		rnode->SetParticleRadius(0.05f);
		rnode->SetColor(Vector4(1.f, 0.f, 1.f, 1.f));
		rnode->GeneratePositionBuffer(num_particles, NULL);

		//We don't need any game logic, or model matrices, just a means to render our
		// particles to the screen.. so this is just a wrapper  to our actual
		// vertex buffer that holds each particles world position.
		this->AddGameObject(new GameObject("", rnode, NULL));


		cudaParticleProg = new CudaParticles();
		cudaParticleProg->InitializeArrays(num_particles, rnode->GetGLVertexBuffer());
		num_particles_updating = 0;
	}

	virtual void OnCleanupScene() override
	{
		Scene::OnCleanupScene();
		delete cudaParticleProg;
	}

	virtual void OnUpdateScene(float dt) override
	{
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Info ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "    No. Particles: %d", num_particles_updating);


		//We don't want an instant 'splurge' of particles, so were going to add them to the 
		// system gradually over the course of a few frames.
		num_particles_updating = min(num_particles_updating+30, num_particles);


		cudaParticleProg->UpdateParticles(num_particles_updating, dt);
	}


protected:
	uint num_particles;
	uint num_particles_updating;
	CudaParticles* cudaParticleProg;
};