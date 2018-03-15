#pragma once

#pragma once

#include "CollisionShape.h"
#include "Hull.h"
#include "../nclgl/Vector2.h"

class EdgeCollisionShape : public CollisionShape
{
public:
	EdgeCollisionShape();
	EdgeCollisionShape(const Vector2 & size);
	virtual ~EdgeCollisionShape();

	// Set Cuboid Dimensions
	void SetHalfWidth(float half_width) {
		halfDims.x = fabs(half_width);
		ConstructBoundingRadius();
	}
	void SetHalfHeight(float half_height) {
		halfDims.y = fabs(half_height);
		ConstructBoundingRadius();
	}
	void SetHalfDepth(float half_depth) {
		halfDims.z = fabs(half_depth);
		ConstructBoundingRadius();
	}

	// Get Cuboid Dimensions
	const Vector3& GetHalfDims() const { return halfDims; }
	float GetHalfWidth()	const { return halfDims.x; }
	float GetHalfHeight()	const { return halfDims.y; }
	float GetHalfDepth()	const { return halfDims.z; }

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
	//Constructs the static cube hull 
	static void ConstructCubeHull();

protected:
	Vector3				 halfDims;
	static Hull			 cubeHull;			//Static cube descriptor, as all cuboid instances will have the same underlying model format ([-1,-1,-1] - [1,1,1] axis aligned cuboid)
};

