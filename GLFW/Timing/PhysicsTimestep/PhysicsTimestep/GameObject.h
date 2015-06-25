/*
Title: Physics Timestep
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
Builds upon the FPS project to introduce the concept of using a physics timestep.
What this means is that every update will have a constant delta time that is set by
a variable. This allows for smooth animations and deterministic physics. This particular
project also implements an accumulator, which will take the delta time between the two
frames and add it to a variable. That variable is then compared to the physics timestep,
and we may end up calling the update function twice in a given frame. Even then, the
delta time for the update function will always equal the physics timestep.
*/

#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include "Model.h"

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

public:
	GameObject(Model*);

	void CalculateMatrices();

	void Update(float);

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
