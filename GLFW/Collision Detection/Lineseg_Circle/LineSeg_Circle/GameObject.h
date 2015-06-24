/*
Title: Line Segment and Circle 2D intersection
File Name: GameObject.h
Copyright © 2015
Author: Srinivasan Thiagarajan
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
This is an example to detect the intersection of a line segment and circle in 2D.
You can control the two end-points of the line segment, and move them using "w,a,s,d" and "i,j,k,l" respectively.
The line turns blue when an intersection is detected, and turns red when there is no intersection.
The program first checks if either of the end-points lie within the circle (which is stationary), if so, collision is detected.
If neither of the end-points are inside the circle, then it check for the closest point to the circle's center, which also lies on the line,
and checks if that point lies inside the circle. If so, collision is detected else not.
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
