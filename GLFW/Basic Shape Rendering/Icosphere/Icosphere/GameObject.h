/*
Title: Icosphere
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

References:
http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

Description:
Contains code for generating a 3D Icosahedron, and then refining the edges down to create
an Icosphere. This is a manner of generating a spherical-like object. By default, the number
of revisions on the icosphere is 5, which can create a pretty mesmerizing effect (because I
also have it randomizing the colors of each vertex and spinning around constantly). You can
also choose to lower or increase the number of revisions.

WARNING: Performance will drop painfully once you reach the 7-9 revisions range. I haven't been
able to push it past 9 revisions. Also note that this is incredibly inefficient in terms of the
way it generates the points, so there may be a long startup time for a high number of revisions.
Feel free to optimize it and send it back to me and I'll upload the better version!
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
