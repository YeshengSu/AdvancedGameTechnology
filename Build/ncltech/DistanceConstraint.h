/******************************************************************************
Class: DistanceConstraint
Implements:
Author: 
	Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

	Manages a distance constraint between two objects, ensuring the two objects never
	seperate. It works on the velocity level, enforcing the constraint:
		dot([(velocity of B) - (velocity of A)], normal) = zero
	
	Which is the same as saying, if the velocity of the two objects in the direction of the constraint is zero
	then we can assert that the two objects will not move further or closer together and thus satisfy the constraint.

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Constraint.h"
#include "PhysicsEngine.h"
#include <nclgl\NCLDebug.h>

class DistanceConstraint : public Constraint
{
public:
	DistanceConstraint(PhysicsNode* obj1, PhysicsNode* obj2,
		const Vector3& globalOnA, const Vector3& globalOnB)
	{
		pnodeA = obj1;
		pnodeB = obj2;

		//Set the preferred distance of the constraint to enforce 
		// (ONLY USED FOR BAUMGARTE)
		// - Ideally we only ever work at the velocity level, so satifying (velA-VelB = 0)
		//   is enough to ensure the distance never changes.
		Vector3 ab = globalOnB - globalOnA;
		targetLength = ab.Length();

		//Get the local points (un-rotated) on the two objects where the constraint should
		// be attached relative to the objects center. So when we rotate the objects
		// the constraint attachment point will rotate with it.
		Vector3 r1 = (globalOnA - pnodeA->GetPosition());
		Vector3 r2 = (globalOnB - pnodeB->GetPosition());
		relPosA = Matrix3::Transpose(pnodeA->GetOrientation().ToMatrix3()) * r1;
		relPosB = Matrix3::Transpose(pnodeB->GetOrientation().ToMatrix3()) * r2;
	}

	//Solves the constraint and applies a velocity impulse to the two
	// objects in order to satisfy the constraint.
	virtual void ApplyImpulse() override
	{
		// computer current constraint vars based on object A/B's
		// position/rotation

		Vector3 r1 = pnodeA->GetOrientation().ToMatrix3() * relPosA;
		Vector3 r2 = pnodeB->GetOrientation().ToMatrix3() * relPosB;

		// Get the global contact points in world space 
		Vector3 globalOnA = r1 + pnodeA->GetPosition();
		Vector3 globalOnB = r2 + pnodeB->GetPosition();

		// Get the vector between the two contact points
		Vector3 ab = globalOnB - globalOnA;
		Vector3 abn = ab;
		abn.Normalise();

		// Compute the velocity of objects A and B at the point of contract
		Vector3 v0 = pnodeA->GetLinearVelocity() + Vector3::Cross(pnodeA->GetAngularVelocity(), r1);
		Vector3 v1 = pnodeB->GetLinearVelocity() + Vector3::Cross(pnodeB->GetAngularVelocity(), r2);

		// relative velocity in constraint direction
		float abnVel = Vector3::Dot(v0 - v1, abn);

		// compute the "mass" of the constraint e.g 
		// How difficult it is to move the two objects in the direction of the constraint
		float invConstraintMassLin = pnodeA->GetInverseMass() + pnodeB->GetInverseMass();

		float invconstraintMassRot = Vector3::Dot(abn,
			Vector3::Cross(pnodeA->GetInverseInertia()*Vector3::Cross(r1, abn), r1) +
			Vector3::Cross(pnodeB->GetInverseInertia()*Vector3::Cross(r2, abn), r2));

		float constraintMass = invConstraintMassLin + invconstraintMassRot;

		if (constraintMass > 0.0f)
		{
			// Baumgarte offset (Adds energy to the system to counter 
			// slight solving errors that accumulate over time - known as 'constraint drift')

			// Experiment by commenting this out and
			// see how it affects the constraints over time and when you manually move the objects apart.

			// the key is to find a nice value that is samll enough not to cause objects to explode but also enough
			// to make sure all constraints / will / be satisfied.This value(0.1)will change based on your physics objects,
			// timestep etc., and also how many constraints you are chaining together.

			float b = 0.0f;

			// optional
			float distance_offset = ab.Length() - targetLength;
			float baumgarte_scalar = 0.1f;
			b = -(baumgarte_scalar / PhysicsEngine::Instance()->GetDeltaTime()) * distance_offset;

			// -Eof Optional-

			// compute velocity impulse(jn)
			// In order to satisfy the distance constraint we need to apply forces to ensure the relative velocity 
			// (abnVel) in the direction of the constraint is zero.So we take inverse of the current rel velocity 
			// and multiply it by how hard it will be to move the objects.

			// note : we also add in any extra energy to the system
			// here, e.g baumgarte( and later elasticity)
			float jn = -(abnVel + b) / constraintMass;

			// Apply linear velocity impulse
			pnodeA->SetLinearVelocity(pnodeA->GetLinearVelocity() + abn*(pnodeA->GetInverseMass() * jn));
			pnodeB->SetLinearVelocity(pnodeB->GetLinearVelocity() - abn*(pnodeB->GetInverseMass() * jn));

			//apply rotational velocity impulse
			pnodeA->SetAngularVelocity(pnodeA->GetAngularVelocity() + pnodeA->GetInverseInertia()*Vector3::Cross(r1, abn*jn));
			pnodeB->SetAngularVelocity(pnodeB->GetAngularVelocity() - pnodeB->GetInverseInertia()*Vector3::Cross(r2, abn*jn));

		}


	}

	//Draw the constraint visually to the screen for debugging
	virtual void DebugDraw() const
	{
		Vector3 globalOnA = pnodeA->GetOrientation().ToMatrix3() * relPosA + pnodeA->GetPosition();
		Vector3 globalOnB = pnodeB->GetOrientation().ToMatrix3() * relPosB + pnodeB->GetPosition();

		NCLDebug::DrawThickLine(globalOnA, globalOnB, 0.02f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		NCLDebug::DrawPointNDT(globalOnA, 0.05f, Vector4(1.0f, 0.8f, 1.0f, 1.0f));
		NCLDebug::DrawPointNDT(globalOnB, 0.05f, Vector4(1.0f, 0.8f, 1.0f, 1.0f));
	}

protected:
	PhysicsNode *pnodeA, *pnodeB;

	float   targetLength;

	Vector3 relPosA;
	Vector3 relPosB;
};