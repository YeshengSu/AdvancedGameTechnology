#include "CommonUtils.h"
#include "SphereCollisionShape.h"
#include "CuboidCollisionShape.h"
#include "CommonMeshes.h"
#include "ScreenPicker.h"
#include <nclgl\RenderNode.h>
#include <functional>

//Horrible!!!
// TODO: Fix these variables!
bool dragDataSet = false;
Matrix3 dragDataInertia;
float dragDataMass;

void CommonUtils::DragableObjectCallback(GameObject* obj, float dt, const Vector3& newWsPos, const Vector3& wsMovedAmount, bool stopDragging)
{
	if (obj->HasPhysics())
	{
		if (Window::GetMouse()->ButtonDown(MOUSE_LEFT))
		{
			//Position
			obj->Physics()->SetPosition(newWsPos);
			obj->Physics()->SetLinearVelocity(wsMovedAmount / dt);
			obj->Physics()->SetAngularVelocity(Vector3(0, 0, 0));
		}
		else if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT))
		{
			Matrix3 viewRot = (Matrix3(GraphicsPipeline::Instance()->GetCamera()->BuildViewMatrix()));
			Matrix3 invViewRot = Matrix3::Transpose(viewRot);

			//Rotation
			Vector3 angVel = invViewRot * Vector3::Cross(Vector3(0, 0, 1), viewRot * wsMovedAmount * 25.f);

			obj->Physics()->SetAngularVelocity(angVel);
			Quaternion quat = obj->Physics()->GetOrientation();
			quat = quat + Quaternion(angVel * dt * 0.5f, 0.0f) * quat;
			quat.Normalise();
			obj->Physics()->SetOrientation(quat);
			obj->Physics()->SetLinearVelocity(Vector3(0, 0, 0));
		}


		if (stopDragging)
		{
			dragDataSet = false;

			obj->Physics()->SetInverseInertia(dragDataInertia);
			obj->Physics()->SetInverseMass(dragDataMass);
			
		}
		else if(!dragDataSet)
		{
			dragDataSet = true;

			dragDataInertia = obj->Physics()->GetInverseInertia();
			dragDataMass = obj->Physics()->GetInverseMass();

			obj->Physics()->SetInverseInertia(Matrix3::ZeroMatrix);
			obj->Physics()->SetInverseMass(0.0f);
		}
	}
	else
	{
		Matrix4 transform = obj->Render()->GetTransform();
		transform.SetPositionVector(newWsPos);
		obj->Render()->SetTransform(transform);
	}
}

Vector4 CommonUtils::GenColor(float scalar, float alpha)
{
	Vector4 c;
	c.w = alpha;

	//Hue
	float t;
	c.x = abs(modf(scalar + 1.0f, &t) * 6.0f - 3.0f) - 1.0f;
	c.y = abs(modf(scalar + 2.0f / 3.0f, &t) * 6.0f - 3.0f) - 1.0f;
	c.z = abs(modf(scalar + 1.0f / 3.0f, &t) * 6.0f - 3.0f) - 1.0f;

	//Safety Checks
	c.x = min(max(c.x, 0.0f), 1.0f);
	c.y = min(max(c.y, 0.0f), 1.0f);
	c.z = min(max(c.z, 0.0f), 1.0f);

	return c;
}

Vector4 CommonUtils::GenHSVColor(const Vector3& hsv, float alpha)
{
	Vector4 c = GenColor(hsv.x, alpha);

	//Saturation & Value
	c.x = hsv.z * (c.x * hsv.y + 1.0f - hsv.y);
	c.y = hsv.z * (c.y * hsv.y + 1.0f - hsv.y);
	c.z = hsv.z * (c.z * hsv.y + 1.0f - hsv.y);

	return c;
}

GameObject* CommonUtils::BuildSphereObject(
	const std::string& name,
	const Vector3& pos,
	float radius,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	//Due to the way SceneNode/RenderNode's were setup, we have to make a dummy node which has the mesh and scaling transform
	// and a parent node that will contain the world transform/physics transform
	RenderNode* rnode = new RenderNode();

	RenderNode* dummy = new RenderNode(CommonMeshes::Sphere(), color);
	dummy->SetTransform(Matrix4::Scale(Vector3(radius, radius, radius)));
	rnode->AddChild(dummy);

	rnode->SetTransform(Matrix4::Translation(pos));
	rnode->SetBoundingRadius(radius);

	PhysicsNode* pnode = NULL;	
	if (physics_enabled)
	{
		pnode = new PhysicsNode();
		pnode->SetPosition(pos);
		pnode->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pnode->SetInverseInertia(SphereCollisionShape(radius).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new SphereCollisionShape(radius);
			pnode->SetCollisionShape(pColshape);
			pnode->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	GameObject* obj = new GameObject(name, rnode, pnode);

	if (dragable)
	{
		ScreenPicker::Instance()->RegisterNodeForMouseCallback(
			dummy, //Dummy is the rendernode that actually contains the drawable mesh, and the one we can to 'drag'
			std::bind(&DragableObjectCallback, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);
	}

	return obj;
}

GameObject* CommonUtils::BuildCuboidObject(
	const std::string& name,
	const Vector3& pos,
	const Vector3& halfdims,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	//Due to the way SceneNode/RenderNode's were setup, we have to make a dummy node which has the mesh and scaling transform
	// and a parent node that will contain the world transform/physics transform
	RenderNode* rnode = new RenderNode();

	RenderNode* dummy = new RenderNode(CommonMeshes::Cube(), color);
	dummy->SetTransform(Matrix4::Scale(halfdims));
	rnode->AddChild(dummy);

	rnode->SetTransform(Matrix4::Translation(pos));
	rnode->SetBoundingRadius(halfdims.Length());

	PhysicsNode* pnode = NULL;
	if (physics_enabled)
	{
		pnode = new PhysicsNode();
		pnode->SetPosition(pos);
		pnode->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pnode->SetInverseInertia(CuboidCollisionShape(halfdims).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new CuboidCollisionShape(halfdims);
			pnode->SetCollisionShape(pColshape);
			pnode->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	GameObject* obj = new GameObject(name, rnode, pnode);

	if (dragable)
	{
		ScreenPicker::Instance()->RegisterNodeForMouseCallback(
			dummy, //Dummy is the rendernode that actually contains the drawable mesh
			std::bind(&DragableObjectCallback, obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);
	}
	
	return obj;
}
