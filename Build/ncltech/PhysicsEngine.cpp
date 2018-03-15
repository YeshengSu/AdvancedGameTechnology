#include "PhysicsEngine.h"
#include "GameObject.h"
#include "CollisionDetectionSAT.h"
#include <nclgl\NCLDebug.h>
#include <nclgl\Window.h>
#include <omp.h>
#include <algorithm>


void PhysicsEngine::SetDefaults()
{
	//Variables set here /will/ be reset with each scene
	updateTimestep = 1.0f / 60.f;
	updateRealTimeAccum = 0.0f;
	gravity = Vector3(0.0f, -9.81f, 0.0f);
	dampingFactor = 0.999f;
	if (!octree.IsHasTree())
		octree.CreateTree(Vector3(0.0f, 0.0f, 0.0f), Vector3(20.0f, 20.0f, 20.0f), 3);
}

PhysicsEngine::PhysicsEngine()
{
	//Variables set here will /not/ be reset with each scene
	isPaused = false;  
	isUpdateTree = true;
	debugDrawFlags = 0;

	SetDefaults();
}

PhysicsEngine::~PhysicsEngine()
{
	RemoveAllPhysicsObjects();
}

void PhysicsEngine::AddPhysicsObject(PhysicsNode* obj)
{
	physicsNodes.push_back(obj);
	octree.AddItem(obj);
}

void PhysicsEngine::RemovePhysicsObject(PhysicsNode* obj)
{
	//Lookup the object in question
	auto found_loc = std::find(physicsNodes.begin(), physicsNodes.end(), obj);

	//If found, remove it from the list
	if (found_loc != physicsNodes.end())
	{
		physicsNodes.erase(found_loc);
		octree.RemoveItem(obj);
	}
}

void PhysicsEngine::RemoveAllPhysicsObjects()
{
	//Delete and remove all constraints/collision manifolds
	for (Constraint* c : constraints)
	{
		delete c;
	}
	constraints.clear();

	for (Manifold* m : manifolds)
	{
		delete m;
	}
	manifolds.clear();


	//Delete and remove all physics objects
	// - we also need to inform the (possibly) associated game-object
	//   that the physics object no longer exists
	for (PhysicsNode* obj : physicsNodes)
	{
		if (obj->GetParent()) obj->GetParent()->SetPhysics(NULL);
		delete obj;
	}
	physicsNodes.clear();

	octree.RemoveAllItem();
}


void PhysicsEngine::Update(float deltaTime)
{
	//The physics engine should run independantly to the renderer
	// - As our codebase is currently single threaded we just need
	//   a way of calling "UpdatePhysics()" at regular intervals
	//   or multiple times a frame if the physics timestep is higher
	//   than the renderers.
	const int max_updates_per_frame = 5;

	if (!isPaused)
	{
		updateRealTimeAccum += deltaTime;
		for (int i = 0; (updateRealTimeAccum >= updateTimestep) && i < max_updates_per_frame; ++i)
		{
			updateRealTimeAccum -= updateTimestep;

			//Additional IsPaused check here incase physics was paused inside one of it's components for debugging or otherwise
			if (!isPaused) UpdatePhysics(); 
		}

		if (updateRealTimeAccum >= updateTimestep)
		{
			NCLDebug::Log("Physics too slow to run in real time!");
			//Drop Time in the hope that it can continue to run faster the next frame
			updateRealTimeAccum = 0.0f;
		}
	}
}


void PhysicsEngine::UpdatePhysics()
{
	for (Manifold* m : manifolds)
	{
		delete m;
	}
	manifolds.clear();

	perfUpdate.UpdateRealElapsedTime(updateTimestep);
	perfBroadphase.UpdateRealElapsedTime(updateTimestep);
	perfNarrowphase.UpdateRealElapsedTime(updateTimestep);
	perfPreSolver.UpdateRealElapsedTime(updateTimestep);
	perfVelociy.UpdateRealElapsedTime(updateTimestep);
	perfSolver.UpdateRealElapsedTime(updateTimestep);
	perfPosition.UpdateRealElapsedTime(updateTimestep);

	//A whole physics engine in 6 simple steps =D
	
	//-- Using positions from last frame --
	perfUpdate.BeginTimingSection();

//1. Broadphase Collision Detection (Fast and dirty)
	perfBroadphase.BeginTimingSection();
	BroadPhaseCollisions();
	perfBroadphase.EndTimingSection();

//2. Narrowphase Collision Detection (Accurate but slow)
	perfNarrowphase.BeginTimingSection();
	NarrowPhaseCollisions();
	perfNarrowphase.EndTimingSection();


//3. Initialize Constraint Params (precompute elasticity/baumgarte factor etc)
	//Optional step to allow constraints to 
	// precompute values based off current velocities 
	// before they are updated loop below.
	perfPreSolver.BeginTimingSection();
	for (Manifold* m : manifolds) m->PreSolverStep(updateTimestep);
	for (Constraint* c : constraints) c->PreSolverStep(updateTimestep);
	perfPreSolver.EndTimingSection();

//4. Update Velocities
	perfVelociy.BeginTimingSection();
	for (PhysicsNode* obj : physicsNodes) obj->IntegrateForVelocity(updateTimestep);
	perfVelociy.EndTimingSection();

//5. Constraint Solver
	perfSolver.BeginTimingSection();
	for (size_t i = 0; i < SOLVER_ITERATIONS; ++i)
	{
		for (Manifold* m : manifolds) m->ApplyImpulse();
		for (Constraint* c : constraints) c->ApplyImpulse();
	}
	perfSolver.EndTimingSection();

//6. Update Positions (with final 'real' velocities)
	perfPosition.BeginTimingSection();
	for (PhysicsNode* obj : physicsNodes) obj->IntegrateForPosition(updateTimestep);
	perfPosition.EndTimingSection();

	perfUpdate.EndTimingSection();
}

void PhysicsEngine::BroadPhaseCollisions()
{
	broadphaseColPairs.clear();
	
	//	The broadphase needs to build a list of all potentially colliding objects in the world,
	//	which then get accurately assesed in narrowphase. If this is too coarse then the system slows down with
	//	the complexity of narrowphase collision checking, if this is too fine then collisions may be missed.

	//	Brute force approach.
	//  - For every object A, assume it could collide with every other object.. 
	//    even if they are on the opposite sides of the world.

	// update octree based on position and bounding raidus
	if (isUpdateTree)
	{
		octree.Update();
		isUpdateTree = false;
	}

	// perform broad collision detection and push them into container
	octree.CollisionDetection(broadphaseColPairs);

}


void PhysicsEngine::NarrowPhaseCollisions()
{
	if (broadphaseColPairs.size() > 0)
	{
		//Collision data to pass between detection and manifold generation stages.
		CollisionData colData;				

		//Collision Detection Algorithm to use
		CollisionDetectionSAT colDetect;	

		// Iterate over all possible collision pairs and perform accurate collision detection
		for (size_t i = 0; i < broadphaseColPairs.size(); ++i)
		{
			CollisionPair& cp = broadphaseColPairs[i];

			std::vector<CollisionShape*>& shapesA = cp.pObjectA->GetCollisionShape();
			std::vector<CollisionShape*>& shapesB = cp.pObjectB->GetCollisionShape();

			for (std::vector<CollisionShape*>::iterator i = shapesA.begin(); i != shapesA.end(); i++)
			{
				for (std::vector<CollisionShape*>::iterator j = shapesB.begin(); j != shapesB.end(); j++)
				{
					colDetect.BeginNewPair(
						cp.pObjectA,
						cp.pObjectB,
						(*i),
						(*j));

					//--TUTORIAL 4 CODE--
					// Detects if the objects are colliding
					if (colDetect.AreColliding(&colData))
					{
						//Note: As at the end of tutorial 4 we have very little to do, this is a bit messier
						//      than it should be. We now fire oncollision events for the two objects so they
						//      can handle AI and also optionally draw the collision normals to see roughly
						//      where and how the objects are colliding.

						//Draw collision data to the window if requested
						// - Have to do this here as colData is only temporary. 
						if (debugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS)
						{
							NCLDebug::DrawPointNDT(colData._pointOnPlane, 0.1f, Vector4(0.5f, 0.5f, 1.0f, 1.0f));
							NCLDebug::DrawThickLineNDT(colData._pointOnPlane, colData._pointOnPlane - colData._normal * colData._penetration, 0.05f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
						}

						//Check to see if any of the objects have a OnCollision callback that dont want the objects to physically collide
						bool okA = cp.pObjectA->FireOnCollisionEvent(cp.pObjectA, cp.pObjectB);
						bool okB = cp.pObjectB->FireOnCollisionEvent(cp.pObjectB, cp.pObjectA);

						if (okA && okB)
						{
							/* TUTORIAL 5 CODE */
							Manifold* manifold = new Manifold();

							manifold->Initiate(cp.pObjectA, cp.pObjectB);

							// Construct contact points that form the perimeter of the collision manifold

							colDetect.GenContactPoints(manifold);

							if (manifold->contactPoints.size() > 0)
							{
								// Add to list of manifolds that need solving
								manifolds.push_back(manifold);
							}
							else
								delete manifold;

						}
					}
				}
			}
			
		}

	}
	std::random_shuffle(manifolds.begin(),manifolds.end());
	std::random_shuffle(constraints.begin(), constraints.end());
}


void PhysicsEngine::DebugRender()
{
	// Draw all collision manifolds
	if (debugDrawFlags & DEBUGDRAW_FLAGS_MANIFOLD)
	{
		for (Manifold* m : manifolds)
		{
			m->DebugDraw();
		}
	}

	// Draw all constraints
	if (debugDrawFlags & DEBUGDRAW_FLAGS_CONSTRAINT)
	{
		for (Constraint* c : constraints)
		{
			c->DebugDraw();
		}
	}

	// Draw all associated collision shapes
	if (debugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES)
	{
		for (PhysicsNode* obj : physicsNodes)
		{
			std::vector<CollisionShape*>& shapes = obj->GetCollisionShape();
			for (std::vector<CollisionShape*>::iterator i = shapes.begin(); i != shapes.end(); i++)
			{
				(*i)->DebugDraw();
			}
		}
	}

	// Draw all associated collision shapes
	if (debugDrawFlags & DEBUGDRAW_FLAGS_BOUNDINGRADIUS)
	{
		for (PhysicsNode* obj : physicsNodes)
		{
			obj->DrawBoundingRadius();
		}
		octree.DebugDrawTree();
	}
}