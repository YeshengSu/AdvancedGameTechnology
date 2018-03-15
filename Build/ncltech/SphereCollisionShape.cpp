#include "SphereCollisionShape.h"
#include "PhysicsNode.h"
#include <nclgl\NCLDebug.h>
#include <nclgl\Matrix3.h>
#include <nclgl\Vector3.h>


SphereCollisionShape::SphereCollisionShape()
{
	m_Radius = 1.0f;
	ConstructBoundingRadius();
}

SphereCollisionShape::SphereCollisionShape(float radius)
{
	m_Radius = radius;
	ConstructBoundingRadius();
}

SphereCollisionShape::~SphereCollisionShape()
{

}

Matrix3 SphereCollisionShape::BuildInverseInertia(float invMass) const
{
	//https://en.wikipedia.org/wiki/List_of_moments_of_inertia
	float i = 2.5f * invMass / (m_Radius * m_Radius); //SOLID
	//float i = 1.5f * invMass / (m_Radius * m_Radius); //HOLLOW

	Matrix3 inertia;
	inertia._11 = i;
	inertia._22 = i;
	inertia._33 = i;

	return inertia;
}


//TUTORIAL 4 CODE
void SphereCollisionShape::GetCollisionAxes(const PhysicsNode* otherObject, std::vector<Vector3>& out_axes) const
{
	/* There are infinite possible axes on a sphere so we MUST handle it seperately
		- Luckily we can just get the closest point on the opposite object to our centre and use that.
	*/
	Matrix4 transform = Parent()->GetWorldSpaceTransform().GetRotationMatrix();

	Vector3 dir = (otherObject->GetPosition() - Parent()->GetPosition() + transform*m_Offset).Normalise();

	Vector3 p1 = Parent()->GetPosition() + transform*m_Offset;

	const std::vector<CollisionShape*>& shapes = otherObject->GetCollisionShape();
	Vector3 p2 = (*shapes.cbegin())->GetClosestPoint(p1);
	for (std::vector<CollisionShape*>::const_iterator i = (++shapes.cbegin()); i != shapes.cend(); i++)
	{
		Vector3 temp = (*i)->GetClosestPoint(p1);
		if ((p1 - temp).Length() < (p1 - p2).Length())
			p2 = temp;
	}

	out_axes.push_back((p1 - p2).Normalise());
}

Vector3 SphereCollisionShape::GetClosestPoint(const Vector3& point) const
{
	Matrix4 transform = Parent()->GetWorldSpaceTransform().GetRotationMatrix();

	Vector3 diff = (point - Parent()->GetPosition() + transform*m_Offset).Normalise();
	return Parent()->GetPosition() + transform*m_Offset + diff * m_Radius;
}

void SphereCollisionShape::GetMinMaxVertexOnAxis(
	const Vector3& axis,
	Vector3& out_min,
	Vector3& out_max) const
{
	Matrix4 transform = Parent()->GetWorldSpaceTransform().GetRotationMatrix();
	out_min = Parent()->GetPosition() + transform*m_Offset - axis * m_Radius;
	out_max = Parent()->GetPosition() + transform*m_Offset + axis * m_Radius;
}
//-------------


void SphereCollisionShape::GetIncidentReferencePolygon(
	const Vector3& axis,
	std::list<Vector3>& out_face,
	Vector3& out_normal,
	std::vector<Plane>& out_adjacent_planes) const
{
	Matrix4 transform = Parent()->GetWorldSpaceTransform().GetRotationMatrix();
	//This is used in Tutorial 5
	out_face.push_back(Parent()->GetPosition() + transform*m_Offset + axis * m_Radius);
	out_normal = axis;	
}

void SphereCollisionShape::DebugDraw() const
{
	Matrix4 transform = Parent()->GetWorldSpaceTransform().GetRotationMatrix();
	NCLDebug::DrawSphere(Parent()->GetPosition() + transform*m_Offset, m_Radius, Vector4(1.0f, 0.3f, 1.0f, 1.0f));
}

void SphereCollisionShape::DebugDrawBounding() const
{
	NCLDebug::DrawSphere(Parent()->GetPosition(), GetBoundingRadius(), Vector4(0.3f, 1.0f, 1.0f, 1.0f));
}

float SphereCollisionShape::GetBoundingRadius() const
{
	return boundingRadius;
}

void SphereCollisionShape::ConstructBoundingRadius()
{
	boundingRadius = m_Radius + m_Offset.Length();
	if (m_Parent != nullptr)
		m_Parent->ConstructBoundingRadius();
}
