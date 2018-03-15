#include "PhysicsNode.h"
#include "PhysicsEngine.h"


void PhysicsNode::IntegrateForVelocity(float dt)
{
	// Apply Gravity
	// Technically gravity here is calculated by formula
	// (gravity / invMass * invMass * dt)
	// So even though the divide and multiply cancel out , we still need to handle the possibility of divide by zero

	//Runge-Kutta ¡±Midpoint¡± Method
	//halfLinVelocity = linVelocity;
	//halfAngVelocity = angVelocity;

	if (invMass > 0.0f)
	{
		linVelocity += PhysicsEngine::Instance()->GetGravity() * dt;
		//halfLinVelocity += PhysicsEngine::Instance()->GetGravity() * (dt*0.5f);
	}

	// Semi-Implicity Euler Intergration
	// - see "Update Position" below
	linVelocity += force *invMass * dt;
	//halfLinVelocity += force *invMass * (dt*0.5f);

	// apply velocity damping
	// - This removes a tiny bit of energy from the simulation each update from nowhere.
	// - In it's present form this can be seen as a rough approximation of air resistance ,
	//	albeit(wrongly?) making the assumption that all objects have the same surface area.
	linVelocity = linVelocity * PhysicsEngine::Instance()->GetDampingFactor();
	//halfLinVelocity = halfLinVelocity * PhysicsEngine::Instance()->GetDampingFactor();

	//Angular rotation 
	//- these are the exact same calculations as the three lines above, except for rotations rather than positions
	//	-Mass		-> Torque
	//  -Velocity	-> Rotational Velocity
	//	-Position	-> Orientation
	angVelocity += invInertia * torque * dt;
	//halfAngVelocity += invInertia * torque * (dt*0.5f);

	//Apply velocity Damping
	angVelocity = angVelocity * PhysicsEngine::Instance()->GetDampingFactor();
	//halfAngVelocity = halfAngVelocity * PhysicsEngine::Instance()->GetDampingFactor();

}

/* Between these two functions the physics engine will solve for velocity
   based on collisions/constraints etc. So we need to integrate velocity, solve 
   constraints, then use final velocity to update position. 
*/

void PhysicsNode::IntegrateForPosition(float dt)
{
	// Update Position
	// - Euler intergration : works on the assumption that linearvelocity 
	//	does not change over time (or changes so lightly it doesn't make a difference)
	// - In this scenario, gravity / will be increasing velocity over time. 
	//	The in-accuracy of not taking into account of these changes over time can be visibly seen in tutorial 1.. and 
	//	thus how better intergration schemes lead to better approximations by taking into account of curvature

	//Semi-Implicit Euler Integration
	position += linVelocity * dt;
	//Runge-Kutta ¡±Midpoint¡± Method
	//position += halfLinVelocity * dt;

	// Update Orientation
	// - this is slightly different calculation due to the wierdness
	//	 of quaternions. It does the same thing as position update
	//	(with a slight error )but from i hvae seen , is generally the best wat to update orientation.
	//Semi-Implicit Euler Integration
	orientation = orientation + Quaternion(angVelocity * dt * 0.5f, 0.0f) * orientation;
	//Runge-Kutta ¡±Midpoint¡± Method
	//orientation = orientation + Quaternion(halfAngVelocity * dt * 0.5f, 0.0f) * orientation;

	//invInertia = invInertia * (Quaternion(angvelocity * dt * 0.5f, 0.0f)).ToMatrix3();
	// as the above formulation has slight approximation error ,
	// we need to normalize our orientation here to stop them accumulation
	orientation.Normalise();

	//Finally: Notify any listener's that this PhysicsNode has a new world transform.
	// - This is used by GameObject to set the worldTransform of any RenderNode's. 
	//   Please don't delete this!!!!!
	FireOnUpdateCallback();
}

float PhysicsNode::GetBoundingRadius() const
{
	return boundingRadius;
}

void PhysicsNode::DrawBoundingRadius() const
{
	NCLDebug::DrawSphere(position,boundingRadius, Vector4(0.3f, 1.0f, 1.0f, 1.0f));
}

void PhysicsNode::ConstructBoundingRadius()
{
	float maxRadius = 0.0f;
	for (std::vector<CollisionShape*>::const_iterator i = collisionShapes.cbegin(); i != collisionShapes.cend(); i++)
	{
		if (maxRadius < (*i)->GetBoundingRadius())
		{
			maxRadius = (*i)->GetBoundingRadius();
		}
	}
	boundingRadius = maxRadius;
}

void PhysicsNode::FireOnUpdateCallback()
{
	//Build world transform
	worldTransform = orientation.ToMatrix4();
	worldTransform.SetPositionVector(position);

	//Fire the OnUpdateCallback, notifying GameObject's and other potential
	// listeners that this PhysicsNode has a new world transform.
	if (onUpdateCallback) onUpdateCallback(worldTransform);
	PhysicsEngine::Instance()->EnbaleUpdateTree();
}
