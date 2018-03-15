/******************************************************************************
Class: 
Namespace: CommonUtils
Implements:
Author:
	Pieran Marris <p.marris@newcastle.ac.uk>
Description:
	Common Utilities/functions to help in quickly building up a scene with as little code as possible.
	Contains compounded factory functions to automatically construct GameObject's that fit the parameters
	provided.

	Everything here, probably shouldn't be bundled with the physics engine library... and in honesty
	should have been built better. Though for the sake of simplicity (and for the love of ridiculously long
	parameter lists) it's here, and here to stay. =]

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameObject.h"

namespace CommonUtils
{
	void DragableObjectCallback(GameObject* obj, float dt, const Vector3& newWsPos, const Vector3& wsMovedAmount, bool stopDragging);

	//Generates a unique color based on scalar parameter in the range of 0-1.
	// - Works off HSV color format, in this case Saturation and Value are 100% and Hue is dependant on 'scalar'
	Vector4 GenColor(float scalar, float alpha = 1.0f);

	//Similar as above, but full HSV rather than just hue
	// - All hsv values range from 0-1 (including hue!)
	Vector4 GenHSVColor(const Vector3& hsv, float alpha = 1.0f);

	//Generates a default Sphere object with the parameters specified.
	GameObject* BuildSphereObject(
		const std::string& name,
		const Vector3& pos,
		float radius,
		bool physics_enabled = false,
		float inverse_mass = 0.0f,			//requires physics_enabled = true
		bool collidable = true,				//requires physics_enabled = true
		bool dragable = true,
		const Vector4& color = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	//Generates a default Cuboid object with the parameters specified
	GameObject* BuildCuboidObject(
		const std::string& name,
		const Vector3& pos,
		const Vector3& halfdims,
		bool physics_enabled = false,
		float inverse_mass = 0.0f,			//requires physics_enabled = true
		bool collidable = true,				//requires physics_enabled = true
		bool dragable = true,
		const Vector4& color = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
};