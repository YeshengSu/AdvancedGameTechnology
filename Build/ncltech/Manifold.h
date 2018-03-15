/******************************************************************************
Class: Manifold
Implements:
Author: 
	Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

	A manifold is the surface area of the collision between two objects, which 
	for the purpose of this physics engine is also used to solve all the contact
	constraints between the two colliding objects.

	This is done by applying a distance constraint at each of the corners of the manifold,
	constraining the shapes to seperate in the next frame. This is also coupled with
	additional constraints of friction and also elasticity in the form of a bias term to add
	additional energy to the system (elasticity) or negate existing velocity (friction).

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "PhysicsNode.h"
#include <nclgl\Vector3.h>

/* A contact constraint is actually the summation of a distance constraint to handle the main collision (normal)
   along with two friction constraints going along the axes perpendicular to the collision
   normal.
*/
struct ContactPoint
{
	//Amount of additional energy to add to the system
	// -- Summation of Baumgarte error correction and elasticity
	float	b_term;

	//Collision Normal and Penetration depth
	Vector3 colNormal;
	float	colPenetration;

	Vector3 relPosA;			//Position relative to objectA
	Vector3 relPosB;			//Position relative to objectB

	//Solver - Total force added this frame
	// - Used to clamp contact constraint over the course of the entire solver
	//   to expected bounds.
	float   sumImpulseContact;
	Vector3 sumImpulseFriction;
};



class Manifold
{
public:
	Manifold();
	~Manifold();

	//Initiate for collision pair
	void Initiate(PhysicsNode* nodeA, PhysicsNode* nodeB);

	//Called whenever a new collision contact between A & B are found
	void AddContact(const Vector3& globalOnA, const Vector3& globalOnB, const Vector3& _normal, const float& _penetration);	

	//Sequentially solves each contact constraint
	void ApplyImpulse();
	void PreSolverStep(float dt);
	

	//Debug draws the manifold surface area
	void DebugDraw() const;

	//Get the physics objects
	PhysicsNode* NodeA() { return pnodeA; }
	PhysicsNode* NodeB() { return pnodeB; }

protected:
	void SolveContactPoint(ContactPoint& c);
	void UpdateConstraint(ContactPoint& c);

public:
	PhysicsNode*				pnodeA;
	PhysicsNode*				pnodeB;
	std::vector<ContactPoint>	contactPoints;
};