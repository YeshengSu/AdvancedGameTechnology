/******************************************************************************
Class: CollisionDetectionSAT
Implements:
Author: 
	Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

	This wrapper class is used to perform a Seperating Axis Therorem test between
	two convex collision shapes, firstly detecting if a collision occured and 
	then more throughly building a collision manifold if required.

	As part of Tutorial 4/5 we will be covering this in detail and building it 
	up ourselves. 

	The additional functionality provided here are geometric functions used in the 
	aforementioned tutorials to perform collision detection. I will try and detail
	them below:
		GetClosestPoint(<point> A, <edges>)
		 - Iterates through all edges returning the the point X which is the closest
		   point along any of the given edges to the provided point A as possible.
		   - Used in SAT detection phase

		PlaneEdgeIntersection(<plane>, <edge>[start, end])
		  - Performs a plane/edge collision test, if an intersection does occur then
			it will return the point on the line where it intersected the given plane.
			- Used in Sutherland-Hodgman Clipping (below)

		SutherlandHodgmanClipping(<mesh>, <clip_planes>)
		  - Performs sutherland hodgman clipping algorithm to clip the provided mesh
			or polygon in regards to each of the provided clipping planes. For more
			information see:
			https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
			 - Used to build collision manifold around instance and reference
			   faces.

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PhysicsNode.h"
#include "CollisionShape.h"
#include "Manifold.h"

struct CollisionData
{
	//The direction of collision from obj1 to obj2
	Vector3		_normal;

	//The amount the objects penetrate eachother (negative overlap distance)
	float		_penetration;

	//The point on obj1 where they overlap
	Vector3		_pointOnPlane;
};

class CollisionDetectionSAT
{
public:
	CollisionDetectionSAT();

	//Start processing new (possible) collision pair
	// - Clear all previous collision data
	void BeginNewPair(
		PhysicsNode* objA,
		PhysicsNode* objB,
		CollisionShape* shapeA,
		CollisionShape* shapeB);

	// Seperating-Axis-Theorem
	// - Returns true if the objects are colliding or false otherwise
	bool AreColliding(CollisionData* out_coldata = NULL);

	// Clipping Method
	// - Uses clipping to construct a manifold describing the surface area
	//   of the collision region
	void GenContactPoints(Manifold* out_manifold);
	
protected:
//<---- SAT ---->
	//Add a new possible colliding axis
	// - This only inserts the axis if it is not a duplicate of pre-existing collision axis
	bool AddPossibleCollisionAxis(Vector3 axis);

	// This will evaluate the given axis working out if the the two objects
	// are indeed colliding in this direction.
	bool CheckCollisionAxis(const Vector3& axis, CollisionData& coldata);
	
private:
	//Physics Nodes
	const PhysicsNode*		pnodeA;
	const PhysicsNode*		pnodeB;

	//Collision shapes
	const CollisionShape*	cshapeA;
	const CollisionShape*	cshapeB;

	//Collision Axes
	std::vector<Vector3>	possibleColAxes;

	//Collision Data
	bool					areColliding;
	CollisionData			bestColData;
};