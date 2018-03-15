#include "EdgeCollisionShape.h"
#include "PhysicsNode.h"
#include "GeometryUtils.h"
#include <nclgl\NCLDebug.h>
#include <nclgl/Matrix3.h>

Hull EdgeCollisionShape::cubeHull = Hull();

EdgeCollisionShape::EdgeCollisionShape()
{
	halfDims = Vector3(0.5f, 0.5f, 0.5f);

	if (cubeHull.GetNumVertices() == 0)
	{
		ConstructCubeHull();
	}
	ConstructBoundingRadius();
}

EdgeCollisionShape::EdgeCollisionShape(const Vector2 & size)
{
	halfDims = Vector3(size.x, size.y, 0.0f);

	if (cubeHull.GetNumVertices() == 0)
	{
		ConstructCubeHull();
	}

	ConstructBoundingRadius();
}

EdgeCollisionShape::~EdgeCollisionShape()
{

}

Matrix3 EdgeCollisionShape::BuildInverseInertia(float invMass) const
{
	Matrix3 inertia;

	Vector3 dimsSq = (halfDims + halfDims);
	dimsSq = dimsSq * dimsSq;

	inertia._11 = 12.f * invMass / (dimsSq.y + dimsSq.z);
	inertia._22 = 12.f * invMass / (dimsSq.x + dimsSq.z);
	inertia._33 = 12.f * invMass / (dimsSq.x + dimsSq.y);

	return inertia;
}

void EdgeCollisionShape::GetCollisionAxes(
	const PhysicsNode* otherObject,
	std::vector<Vector3>& out_axes) const
{
	Matrix3 objOrientation = Parent()->GetOrientation().ToMatrix3();
	out_axes.push_back(objOrientation * Vector3(1.0f, 0.0f, 0.0f)); //X - Axis
	out_axes.push_back(objOrientation * Vector3(0.0f, 1.0f, 0.0f)); //Y - Axis
	out_axes.push_back(objOrientation * Vector3(0.0f, 0.0f, 1.0f)); //Z - Axis
}

Vector3 EdgeCollisionShape::GetClosestPoint(const Vector3& point) const
{
	//Iterate over each edge and get the closest point on any edge to point p.

	// Build World Transform
	Matrix4 wsTransform = Parent()->GetWorldSpaceTransform() * Matrix4::Translation(m_Offset)  * Matrix4::Scale(halfDims);

	// Convert world space axis into model space (Axis Aligned Cuboid)
	Matrix4 invWsTransform = Matrix4::Inverse(wsTransform);
	Vector3 local_point = invWsTransform * point;

	float out_distSq = FLT_MAX;
	Vector3 out_point;
	for (size_t i = 0; i < cubeHull.GetNumEdges(); ++i)
	{
		const HullEdge& e = cubeHull.GetEdge(i);
		Vector3 start = cubeHull.GetVertex(e._vStart)._pos;
		Vector3 end = cubeHull.GetVertex(e._vEnd)._pos;

		Vector3 ep = GeometryUtils::GetClosestPoint(local_point, Edge(start, end));

		float distSq = Vector3::Dot(ep - local_point, ep - local_point);
		if (distSq < out_distSq)
		{
			out_distSq = distSq;
			out_point = ep;
		}
	}

	return wsTransform * out_point;
}

void EdgeCollisionShape::GetMinMaxVertexOnAxis(
	const Vector3& axis,
	Vector3& out_min,
	Vector3& out_max) const
{
	// Build World Transform
	Matrix4 wsTransform = Parent()->GetWorldSpaceTransform() * Matrix4::Translation(m_Offset) * Matrix4::Scale(halfDims);

	// Convert world space axis into model space (Axis Aligned Cuboid)
	Matrix3 invNormalMatrix = Matrix3::Inverse(Matrix3(wsTransform));
	Vector3 local_axis = invNormalMatrix * axis;
	local_axis.Normalise();

	// Get closest and furthest vertex id's
	int vMin, vMax;
	cubeHull.GetMinMaxVerticesInAxis(local_axis, &vMin, &vMax);

	// Return closest and furthest vertices in world-space
	out_min = wsTransform * cubeHull.GetVertex(vMin)._pos;
	out_max = wsTransform * cubeHull.GetVertex(vMax)._pos;
}



void EdgeCollisionShape::GetIncidentReferencePolygon(
	const Vector3& axis,
	std::list<Vector3>& out_face,
	Vector3& out_normal,
	std::vector<Plane>& out_adjacent_planes) const
{
	//Get the world-space transform
	Matrix4 wsTransform = Parent()->GetWorldSpaceTransform() * Matrix4::Translation(m_Offset) * Matrix4::Scale(halfDims);

	//Get normal and inverse-normal matrices to transfom the collision axis to and from modelspace
	Matrix3 invNormalMatrix = Matrix3::Inverse(Matrix3(wsTransform));
	Matrix3 normalMatrix = Matrix3::Inverse(invNormalMatrix);


	Vector3 local_axis = invNormalMatrix * axis;


	//Get the furthest vertex along axis - this will be part of the furthest face
	int undefined, maxVertex;
	cubeHull.GetMinMaxVerticesInAxis(local_axis, &undefined, &maxVertex);
	const HullVertex& vert = cubeHull.GetVertex(maxVertex);


	//Compute which face (that contains the furthest vertex above)
	// is the furthest along the given axis. This is defined by
	// it's normal being closest to parallel with the collision axis.
	const HullFace* best_face = 0;
	float best_correlation = -FLT_MAX;
	for (int faceIdx : vert._enclosing_faces)
	{
		const HullFace* face = &cubeHull.GetFace(faceIdx);
		float temp_correlation = Vector3::Dot(local_axis, face->_normal);
		if (temp_correlation > best_correlation)
		{
			best_correlation = temp_correlation;
			best_face = face;
		}
	}


	// Output face normal
	Vector3 vec = best_face->_normal;
	out_normal = vec.Normalise();

	// Output face vertices (transformed back into world-space)
	for (int vertIdx : best_face->_vert_ids)
	{
		const HullVertex& vert = cubeHull.GetVertex(vertIdx);
		out_face.push_back(wsTransform * vert._pos);
	}

	// Now, we need to define a set of planes that will clip any 3d geometry down to fit inside 
	// the shape. This results in us forming list of clip planes from each of the
	// adjacent faces along with the reference face itself.

	Vector3 wsPointOnPlane = wsTransform * cubeHull.GetVertex(cubeHull.GetEdge(best_face->_edge_ids[0])._vStart)._pos;

	// First, form a plane around the reference face
	{
		//We use the negated normal here for the plane, as we want to clip geometry left outside the shape not inside it.
		Vector3 planeNrml = -(normalMatrix * best_face->_normal);
		planeNrml.Normalise();

		float planeDist = -Vector3::Dot(planeNrml, wsPointOnPlane);
		//out_adjacent_planes.push_back(Plane(planeNrml, planeDist));
	}

	// Now we need to loop over all adjacent faces, and form a similar
	// clip plane around those too.
	// - The way that the HULL object is constructed means each edge can only
	//   ever have two adjoining faces. This means we can iterate through all
	//   edges of the face and then build a plane around the 'other' face that
	//   also shares that edge.
	for (int edgeIdx : best_face->_edge_ids)
	{
		const HullEdge& edge = cubeHull.GetEdge(edgeIdx);

		wsPointOnPlane = wsTransform * cubeHull.GetVertex(edge._vStart)._pos;

		for (int adjFaceIdx : edge._enclosing_faces)
		{
			if (adjFaceIdx != best_face->_idx)
			{
				const HullFace& adjFace = cubeHull.GetFace(adjFaceIdx);

				Vector3 planeNrml = -(normalMatrix * adjFace._normal);
				planeNrml.Normalise();
				float planeDist = -Vector3::Dot(planeNrml, wsPointOnPlane);

				out_adjacent_planes.push_back(Plane(planeNrml, planeDist));
			}
		}
	}

}


void EdgeCollisionShape::DebugDraw() const
{
	// Just draw the cuboid hull-mesh at the position of our PhysicsNode
	Matrix4 transform = Parent()->GetWorldSpaceTransform() * Matrix4::Translation(m_Offset) * Matrix4::Scale(halfDims);
	cubeHull.DebugDraw(transform);
}

void EdgeCollisionShape::DebugDrawBounding() const
{
	NCLDebug::DrawSphere(Parent()->GetPosition() + m_Offset, GetBoundingRadius(), Vector4(0.3f, 1.0f, 1.0f, 1.0f));
}

float EdgeCollisionShape::GetBoundingRadius() const
{
	return boundingRadius;
}

void EdgeCollisionShape::ConstructBoundingRadius()
{
	boundingRadius = halfDims.Length() + m_Offset.Length();
	if (m_Parent != nullptr)
		m_Parent->ConstructBoundingRadius();
}

void EdgeCollisionShape::ConstructCubeHull()
{
	//Vertices
	cubeHull.AddVertex(Vector3(-1.0f, -1.0f, -1.0f));		// 0
	cubeHull.AddVertex(Vector3(-1.0f, 1.0f, -1.0f));		// 1
	cubeHull.AddVertex(Vector3(1.0f, 1.0f, -1.0f));		// 2
	cubeHull.AddVertex(Vector3(1.0f, -1.0f, -1.0f));		// 3

	cubeHull.AddVertex(Vector3(-1.0f, -1.0f, 1.0f));		// 4
	cubeHull.AddVertex(Vector3(-1.0f, 1.0f, 1.0f));		// 5
	cubeHull.AddVertex(Vector3(1.0f, 1.0f, 1.0f));		// 6
	cubeHull.AddVertex(Vector3(1.0f, -1.0f, 1.0f));		// 7

														//Indices ( MUST be provided in ccw winding order )
	int face1[] = { 0, 1, 2, 3 };
	int face2[] = { 7, 6, 5, 4 };
	int face3[] = { 5, 6, 2, 1 };
	int face4[] = { 0, 3, 7, 4 };
	int face5[] = { 6, 7, 3, 2 };
	int face6[] = { 4, 5, 1, 0 };

	//Faces
	cubeHull.AddFace(Vector3(0.0f, 0.0f, -1.0f), 4, face1);
	cubeHull.AddFace(Vector3(0.0f, 0.0f, 1.0f), 4, face2);
	//cubeHull.AddFace(Vector3(0.0f, 1.0f, 0.0f), 4, face3);
	//cubeHull.AddFace(Vector3(0.0f, -1.0f, 0.0f), 4, face4);
	//cubeHull.AddFace(Vector3(1.0f, 0.0f, 0.0f), 4, face5);
	//cubeHull.AddFace(Vector3(-1.0f, 0.0f, 0.0f), 4, face6);
}