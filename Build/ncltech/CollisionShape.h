/******************************************************************************
Class: CollisionShape
Implements:
Author: 
	Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description: 

	A Generic template for all the functionality needed to represent a convex collision shape. 

	This will be the only thing in the physics engine that defines the geometric shape of the
	attached PhysicsNode. It provides a means for computing the interia tensor (rotational mass)
	and a means to calculate collisions with other unknown collision shapes via CollisionDetectionSAT.

	For example usage, see SphereCollisionShape (implicit shape defined by an algorithm - in this case bounding radius)
	and CuboidCollisionShape (physical shape defined by a set of vertices/faces)

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Hull.h"
#include "GeometryUtils.h"
#include <nclgl\Vector3.h>
#include <nclgl\Plane.h>
#include <nclgl\Matrix3.h>
#include <vector>
#include <list>

using namespace GeometryUtils;

class PhysicsNode;

struct CollisionEdge
{
	CollisionEdge(const Vector3& a, const Vector3& b) 
		: _v0(a), _v1(b) {}

	Vector3 _v0;
	Vector3 _v1;
};

class CollisionShape
{
public:
	CollisionShape() : m_Parent(NULL), m_Offset(0.0f,0.0f,0.0f){ boundingRadius = 0; }
	virtual ~CollisionShape()	{}

	// Constructs an inverse inertia matrix of the given collision volume. This is the equivilant of the inverse mass of an object for rotation,
	//   a good source for non-inverse inertia matricies can be found here: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
	virtual Matrix3 BuildInverseInertia(float invMass) const = 0;

	// Draws this collision shape to the debug renderer
	virtual void DebugDraw() const {};
	// Draws bounding radius to the debug renderer
	virtual void DebugDrawBounding()const {};

	void SetOffset(const Vector3& _offset) { m_Offset = _offset; ConstructBoundingRadius(); }
	const Vector3& GetOffset()const { return m_Offset; }

	inline void SetParent(PhysicsNode* node) { m_Parent = node; }
	inline		 PhysicsNode* Parent()		 { return m_Parent; }
	inline const PhysicsNode* Parent() const { return m_Parent; }

	// Get Bounding Radius for broad phase
	virtual float GetBoundingRadius()const = 0;

	// generate bounding radius
	virtual void ConstructBoundingRadius() {};

//<----- USED BY COLLISION DETECTION ----->
	// Get all collision axes between the current shape and the given
	// object to be tested. The axes returned are just for the shape in question
	// the other shape will also have it's own axes to test.
	virtual void GetCollisionAxes(
		const PhysicsNode* otherObject,
		std::vector<Vector3>& out_axes) const = 0;

	//Returns closest point on the collision shape to the given point
	virtual Vector3 GetClosestPoint(const Vector3& point) const = 0;

	// Get the min/max vertices along a given axis
	virtual void GetMinMaxVertexOnAxis(
		const Vector3& axis, 
		Vector3& out_min,
		Vector3& out_max) const = 0;


	// Get all data needed to build manifold
	//	- Computes the face that is closest to parallel to that of the given axis,
	//    returning the face (as a list of vertices), face normal and the planes
	//    of all adjacent faces in order to clip against.
	virtual void GetIncidentReferencePolygon(
		const Vector3& axis,
		std::list<Vector3>& out_face,
		Vector3& out_normal,
		std::vector<Plane>& out_adjacent_planes) const = 0;

protected:
	Vector3 m_Offset;
	PhysicsNode* m_Parent;
	float boundingRadius;
};