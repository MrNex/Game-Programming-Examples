/*
Title: AABB-2D
File Name: GameObject.h
Copyright © 2015
Original authors: Brockton Roth
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Description:
This is an Axis-Aligned Bounding Box collision test. This is in 2D.
Contains two squares, one that is stationary and one that is moving. They are bounded by
AABBs (Axis-Aligned Bounding Boxes) and when these AABBs collide, the moving object
"bounces" on the X-axis (because that is the only direction the object is moving). The
algorithm will detect collision along any axis, but will not be able to output the axis
of collision because it doesn't know. Thus, we assume X and hardcode in the X-axis bounce.
If you would like to know the axis of collision, try out the Swept AABB collision.
There is a physics timestep such that every update runs at the same delta time, regardless
of how fast or slow the computer is running. The squares should be the exact same as their
AABBs, since they are aligned on the X-Y axes but should you wish to see how the AABB
recalculates when the object's orientation changes, simply uncomment the rotate lines
(obj1->Rotate, obj2->Rotate).
*/

#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include "Model.h"

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB(const glm::vec3 &minVal, const glm::vec3 &maxVal)
	{
		min = minVal;
		max = maxVal;
	}
	AABB()
	{
		min = glm::vec3(0.0f);
		max = glm::vec3(0.0f);
	}
};

struct CalculatorAABB
{
	glm::vec4 min;
	glm::vec4 max;

	CalculatorAABB(const glm::vec4 &minVal, const glm::vec4 &maxVal)
	{
		min = minVal;
		max = maxVal;
	}
	CalculatorAABB()
	{
		min = glm::vec4(0.0f);
		max = glm::vec4(0.0f);
	}
};

class GameObject
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;

	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	glm::mat4 transformation;

	glm::quat quaternion;

	Model* model;
	AABB box;

public:
	GameObject(Model*);

	void CalculateMatrices();

	void Update(float);

	AABB GetAABB()
	{
		return box;
	}

	void CalculateAABB();

	Model* GetModel()
	{
		return model;
	}
	glm::mat4* GetTransform()
	{
		return &transformation;
	}
	glm::vec3 GetPosition()
	{
		return position;
	}
	glm::vec3 GetVelocity()
	{
		return velocity;
	}
	glm::vec3 GetAcceleration()
	{
		return acceleration;
	}

	void AddPosition(glm::vec3);
	void SetPosition(glm::vec3 pos)
	{
		position = pos;

		SetTranslation(pos);
	}
	void AddVelocity(glm::vec3);
	void SetVelocity(glm::vec3 vel)
	{
		velocity = vel;
	}
	void AddAcceleration(glm::vec3);
	void SetAcceleration(glm::vec3 accel)
	{
		acceleration = accel;
	}

	// Scales the current scale value by the x, y and z values given.
	void Scale(glm::vec3);

	// Sets the scale in the x, y, and z position to the given values.
	void SetScale(glm::vec3);

	// Rotates in x, y, and z degrees (not radians) based on given values.
	void Rotate(glm::vec3);

	// Sets yhr rotation matrix to a given value.
	void SetRotation(glm::mat4*);
	void SetRotation(glm::vec3);

	// Translates in the x, y, and z directions based on the given values.
	void Translate(glm::vec3);

	// Sets the translations to the exact x, y, and z position values given.
	void SetTranslation(glm::vec3);
};

#endif //_GAME_OBJECT_H
