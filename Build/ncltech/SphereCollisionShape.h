/******************************************************************************
Class: SphereCollisionShape
Implements: CollisionShape
Author: 
	Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

	Extends CollisionShape to represent a sphere.

	This is an example of a implicit collision shape, where all of the output data can be computed algorithmetrically. For instance,
	getting the closest point on a sphere only needs a radius to clamp the length of the vector. Implicit collision shapes are generally
	faster and more effecient, and allow the representation of perfect shapes that would require too many vertices/faces to represent otherwise
	e.g. curved surfaces.

	A sphere object is one of the easiest 3D shapes to model, as for collision purposes it can only ever have one axis of itersection,
	it also can only ever have one contact point. This makes alot of the returns quite trival compared to the cuboid that requires alot of
	setting up and adjanency information.

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "CollisionShape.h"

class SphereCollisionShape : public CollisionShape
{
public:
	SphereCollisionShape();
	SphereCollisionShape(float radius);
	virtual ~SphereCollisionShape();


	// Get/Set Sphere Radius
	void	SetRadius(float radius) { 
		m_Radius = radius;
		ConstructBoundingRadius();
	}
	float	GetRadius() const { return m_Radius; }

	// Debug Collision Shape
	virtual void DebugDraw() const override;

	// Draw bounding radius to the debug renderer
	virtual void DebugDrawBounding()const override;

	// Get Bounding Radius for broad phase
	virtual float GetBoundingRadius()const override;

	// generate bounding radius
	virtual void ConstructBoundingRadius()override;

	// Build Inertia Matrix for rotational mass
	virtual Matrix3 BuildInverseInertia(float invMass) const override;


	// Generic Collision Detection Routines
	//  - Used in CollisionDetectionSAT to identify if two shapes overlap
	virtual void GetCollisionAxes(
		const PhysicsNode* otherObject,
		std::vector<Vector3>& out_axes) const override;

	virtual Vector3 GetClosestPoint(const Vector3& point) const override;

	virtual void GetMinMaxVertexOnAxis(
		const Vector3& axis,
		Vector3& out_min,
		Vector3& out_max) const override;
	
	virtual void GetIncidentReferencePolygon(
		const Vector3& axis,
		std::list<Vector3>& out_face,
		Vector3& out_normal,
		std::vector<Plane>& out_adjacent_planes) const override;

protected:
	float	m_Radius;
};

