#include "Manifold.h"
#include <nclgl\Matrix3.h>
#include <nclgl\NCLDebug.h>
#include "PhysicsEngine.h"
#include <algorithm>

Manifold::Manifold()
	: pnodeA(NULL)
	, pnodeB(NULL)
{
}

Manifold::~Manifold()
{

}

void Manifold::Initiate(PhysicsNode* nodeA, PhysicsNode* nodeB)
{
	contactPoints.clear();

	pnodeA = nodeA;
	pnodeB = nodeB;
}

void Manifold::ApplyImpulse()
{
	for (ContactPoint& contact : contactPoints)
	{
		SolveContactPoint(contact);
	}
}


void Manifold::SolveContactPoint(ContactPoint& c)
{
	/* TUTORIAL 6 CODE */
	Vector3 r1 = c.relPosA;
	Vector3 r2 = c.relPosB;

	Vector3 v0 = pnodeA->GetLinearVelocity() + Vector3::Cross(pnodeA->GetAngularVelocity(), r1);
	Vector3 v1 = pnodeB->GetLinearVelocity() + Vector3::Cross(pnodeB->GetAngularVelocity(), r2);

	Vector3 dv = v1 - v0;

	//Collision Resolution

	float constraintMass =
		(pnodeA->GetInverseMass() + pnodeB->GetInverseMass()) +
		Vector3::Dot(c.colNormal,
			Vector3::Cross(pnodeA->GetInverseInertia()
				*Vector3::Cross(r1, c.colNormal), r1) +
			Vector3::Cross(pnodeB->GetInverseInertia()
				*Vector3::Cross(r2, c.colNormal), r2));

	if (constraintMass > 0.0f)
	{
		// Changes here are needed to ensure that :
		// 1. The Collision never solves itself backwards (e.g. pushes itself
		// all the way though the other side of the other object).As now if
		// an object is colliding on both sides it could theoretically solve
		// it by doing the above scenario.
		// 2. The Friction force never exceeds the force applied by the main
		// collision force. As now the main collision force will change over
		// time
		float jn = max(-Vector3::Dot(dv, c.colNormal) + c.b_term, 0.0f);

		float oldSumImpulseContact = c.sumImpulseContact;
		c.sumImpulseContact = max(c.sumImpulseContact + jn, 0.0f);
		jn = c.sumImpulseContact - oldSumImpulseContact;

		jn = jn / constraintMass;

		pnodeA->SetLinearVelocity(pnodeA->GetLinearVelocity() - c.colNormal*(jn * pnodeA->GetInverseMass()));
		pnodeB->SetLinearVelocity(pnodeB->GetLinearVelocity() + c.colNormal*(jn * pnodeB->GetInverseMass()));

		pnodeA->SetAngularVelocity(pnodeA->GetAngularVelocity() - pnodeA->GetInverseInertia()*Vector3::Cross(r1, c.colNormal*jn));
		pnodeB->SetAngularVelocity(pnodeB->GetAngularVelocity() + pnodeB->GetInverseInertia()*Vector3::Cross(r2, c.colNormal*jn));
	}

	// Friction
	Vector3 tangent = dv - c.colNormal * Vector3::Dot(dv, c.colNormal);
	float tangent_len = tangent.Length();

	if (tangent_len > 1e-6f)
	{
		tangent = tangent / tangent_len;
		float frictionalMass = (pnodeA->GetInverseMass()
			+ pnodeB->GetInverseMass()) + Vector3::Dot(tangent,
				Vector3::Cross(pnodeA->GetInverseInertia()
					* Vector3::Cross(r1, tangent), r1) +
				Vector3::Cross(pnodeB->GetInverseInertia()
					* Vector3::Cross(r2, tangent), r2));

		if (frictionalMass > 0.0f)
		{
			// Stop Friction from ever being more than frictionCoef * normal
			// resolution impulse. Similar to above for SumImpulseContact, Except
			// for friction the direction of friction is potentially changing each
			// time we solve the contact (as other objects are bouncing this
			// object in different directions). So instead of a scalar, we need to 
			// clamp the total tangential impulse vector to the normal
			// resolution during the solving step without having to precompute and 
			// check a single tangent vector
			float frictionCoef = (pnodeA->GetFriction() * pnodeB->GetFriction());

			float jt = -Vector3::Dot(dv, tangent) * frictionCoef;

			Vector3 oldImpulseFriction = c.sumImpulseFriction;
			c.sumImpulseFriction = c.sumImpulseFriction + tangent * jt;
			float len = c.sumImpulseFriction.Length();
			if (len > 0.0f && len > c.sumImpulseContact)
			{
				c.sumImpulseFriction = c.sumImpulseFriction / len * c.sumImpulseContact;
			}
			tangent = c.sumImpulseFriction - oldImpulseFriction;
			jt = 1.0f;
			jt = jt / frictionalMass;

			pnodeA->SetLinearVelocity(pnodeA->GetLinearVelocity() - tangent*(jt*pnodeA->GetInverseMass()));
			pnodeB->SetLinearVelocity(pnodeB->GetLinearVelocity() + tangent*(jt*pnodeB->GetInverseMass()));

			pnodeA->SetAngularVelocity(pnodeA->GetAngularVelocity() - pnodeA->GetInverseInertia() * Vector3::Cross(r1, tangent*jt));
			pnodeB->SetAngularVelocity(pnodeB->GetAngularVelocity() + pnodeB->GetInverseInertia() * Vector3::Cross(r2, tangent*jt));

		}
	}

}

void Manifold::PreSolverStep(float dt)
{
	std::random_shuffle(contactPoints.begin(), contactPoints.end());
	for (ContactPoint& contact : contactPoints)
	{
		UpdateConstraint(contact);
	}
}

void Manifold::UpdateConstraint(ContactPoint& c)
{
	//Reset total impulse forces computed this physics timestep 
	c.sumImpulseContact = 0.0f;
	c.sumImpulseFriction = Vector3(0.0f, 0.0f, 0.0f);
	c.b_term = 0.0f;

	/* TUTORIAL 6 CODE
	// After c. sumImpulseFriction = ( This is used in the next tutorial
	// Baumgarte Offset ( Adds energy to the system to counter slight
	// solving errors that accumulate over time - known as ’constraint
	// drift ?
	// Very slightly different to the one we used to prevent the distance
	// constraints breaking , as we now allow a minimum allowed error .
	// In this case , we allow the objects to overlap by 1mm before adding
	// in correctional energy to counteract the error , this is a little
	// dirty trick that results in us always getting a manifold for
	// constantly colliding objects which normally we would get collide
	// one frame and not the next , endlessly jittering . If you ’re
	// interested , the _slop part (and this issue in general ) is usually
	// handled in physics engines by pretending the collision volume is
	// larger than it is during the narrowphase , so manifolds are
	// generated for colliding , and almost colliding pairs .
	*/

	const float baumgarte_scalar = 0.1f;
	const float baumgarte_slop = 0.001f;
	const float penetration_slop = min(c.colPenetration + baumgarte_slop, 0.0f);

	c.b_term += -(baumgarte_scalar / PhysicsEngine::Instance()->GetDeltaTime()) * penetration_slop;
	//Compute Elasticity Term

	// This is the total velocity going into the collision relative to the 
	// collision normal, as elasticity is 'adding' energy back into the 
	// system we can attach it to our 'b' term wich we already add to 
	// 'jt' when sloving the contact constraint
	const float elasticity = pnodeA->GetElasticity() * pnodeB->GetElasticity();
	const float elatisity_term = Vector3::Dot(c.colNormal, pnodeA->GetLinearVelocity() + Vector3::Cross(c.relPosA, pnodeA->GetAngularVelocity()) - pnodeB->GetLinearVelocity() - Vector3::Cross(c.relPosB, pnodeB->GetAngularVelocity()));

	c.b_term += (elasticity *  elatisity_term) / contactPoints.size();
}

void Manifold::AddContact(const Vector3& globalOnA, const Vector3& globalOnB, const Vector3& normal, const float& penetration)
{
	//Get relative offsets from each object centre of mass
	// Used to compute rotational velocity at the point of contact.
	Vector3 r1 = (globalOnA - pnodeA->GetPosition());
	Vector3 r2 = (globalOnB - pnodeB->GetPosition());

	//Create our new contact descriptor
	ContactPoint contact;
	contact.relPosA = r1;
	contact.relPosB = r2;
	contact.colNormal = normal;
	contact.colNormal.Normalise();
	contact.colPenetration = penetration;

	contactPoints.push_back(contact);


	//What a stupid function!
	// - Manifold's normally persist over multiple frames, as in two colliding objects
	//   (especially in the case of stacking) will likely be colliding in a similar 
	//   setup the following couple of frames. So the accuracy therefore can be increased
	//   by using persistent manifolds, which will only be deleted when the two objects
	//   fail a narrowphase check. This means the manifolds can be quite busy, with lots of
	//   new contacts per frame, but to solve any convex manifold in 3D you really only need
	//   3 contacts (4 max), so tldr; perhaps you want persistent manifolds.. perhaps
	//   you want to put some code here to sort contact points.. perhaps this comment is even 
	//   more pointless than a passthrough function.. perhaps I should just stop typ
}

void Manifold::DebugDraw() const
{
	if (contactPoints.size() > 0)
	{
		//Loop around all contact points and draw them all as a line-loop
		Vector3 globalOnA1 = pnodeA->GetPosition() + contactPoints.back().relPosA;
		for (const ContactPoint& contact : contactPoints)
		{
			Vector3 globalOnA2 = pnodeA->GetPosition() + contact.relPosA;
			Vector3 globalOnB = pnodeB->GetPosition() + contact.relPosB;

			//Draw line to form area given by all contact points
			NCLDebug::DrawThickLineNDT(globalOnA1, globalOnA2, 0.02f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));

			//Draw descriptors for indivdual contact point
			NCLDebug::DrawPointNDT(globalOnA2, 0.05f, Vector4(0.0f, 0.5f, 0.0f, 1.0f));
			NCLDebug::DrawThickLineNDT(globalOnB, globalOnA2, 0.01f, Vector4(1.0f, 0.0f, 1.0f, 1.0f));

			globalOnA1 = globalOnA2;
		}
	}
}