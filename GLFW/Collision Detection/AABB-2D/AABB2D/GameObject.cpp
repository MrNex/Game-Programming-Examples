/*
Title: AABB-2D
File Name: GameObject.cpp
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

#ifndef _GAME_OBJECT_CPP
#define _GAME_OBJECT_CPP

#include "GameObject.h"

// Note that the model does not actually get copied, but instead we just save a pointer to it.
// So make sure that model is stored and cleaned up elsewhere!
GameObject::GameObject(Model* inModel)
{
	model = inModel;

	// Initialize identity matrices.
	translation = glm::mat4();
	rotation = glm::mat4();
	scale = glm::mat4();
	transformation = glm::mat4();

	// And default vectors.
	position = glm::vec3();
	velocity = glm::vec3();
	acceleration = glm::vec3();

	// And a default quaternion.
	quaternion = glm::quat();
}

void GameObject::Update(float dt)
{
	// Do basic physics calcuations based on dt.
	velocity += acceleration * dt;
	position += velocity * dt;

	// Set the translation equal to the new position of the object.
	SetTranslation(position); // Note that this will also recalculate the transformation matrix.
}

void GameObject::CalculateAABB()
{
	// Create local variables for the vertices of the model.
	VertexFormat* vertexArray = model->Vertices();
	int numVertexArray = model->NumVertices();

	// Create a temporary AABB that uses vec4 for the purposes of matrix multiplication.
	CalculatorAABB newBox;

	// Set the min and max equal to the first vertex in the object times the transformation matrix.
	newBox.min = transformation * glm::vec4(vertexArray[0].position, 1.0f);
	newBox.max = newBox.min;

	// Loop through the rest of the vertices.
	for (int i = 1; i < numVertexArray; i++)
	{
		// Create a temporary vertex that is the vertices at index i turned into a vector4 and modified by the transformation matrix.
		glm::vec4 tempVert = transformation * glm::vec4(vertexArray[i].position, 1.0f);

		// If this vertex has a value larger than the max value of our newBox, replace the newBox max value with that value.
		if (tempVert.x > newBox.max.x)
		{
			newBox.max.x = tempVert.x;
		}
		if (tempVert.y > newBox.max.y)
		{
			newBox.max.y = tempVert.y;
		}
		if (tempVert.z > newBox.max.z)
		{
			newBox.max.z = tempVert.z;
		}

		// Do the same for the minimum value.
		if (tempVert.x < newBox.min.x)
		{
			newBox.min.x = tempVert.x;
		}
		if (tempVert.y < newBox.min.y)
		{
			newBox.min.y = tempVert.y;
		}
		if (tempVert.z < newBox.min.z)
		{
			newBox.min.z = tempVert.z;
		}
	}

	// Now set the actual AABB min/max (vec3 instead of vec4) to be stored in this GameObject.
	box.min.x = newBox.min.x;
	box.min.y = newBox.min.y;
	box.min.z = newBox.min.z;
	box.max.x = newBox.max.x;
	box.max.y = newBox.max.y;
	box.max.z = newBox.max.z;
}

// Calculates the transformation matrix based on translation, then rotation, then scale.
void GameObject::CalculateMatrices()
{
	transformation = translation * rotation * scale;
}

// Adds the incoming vec3 pos to the position, and then translates the object to that position.
void GameObject::AddPosition(glm::vec3 pos)
{
	position += pos;

	Translate(pos);
}

// Adds the incoming vec3 vel to the velocity.
void GameObject::AddVelocity(glm::vec3 vel)
{
	velocity += vel;
}

// Adds the incoming vec3 accel to the acceleration.
void GameObject::AddAcceleration(glm::vec3 accel)
{
	acceleration += accel;
}

// Scales the current scale value by the x, y and z values given. (So if the scale is [0.5, 0.5, 0.5] and we pass in [0.5, 0.5, 0.5] we end up with [0.25, 0.25, 0.25].)
void GameObject::Scale(glm::vec3 scaleFactor)
{
	// Scales the scale matrix.
	scale = glm::scale(scale, scaleFactor);

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

// Sets the scale in the x, y, and z position to the given values.
void GameObject::SetScale(glm::vec3 scaleFactor)
{
	// Scales the identity matrix.
	scale = glm::scale(glm::mat4(), scaleFactor);

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

// Rotates in x, y, and z radians based on given values.
void GameObject::Rotate(glm::vec3 rotFactor)
{
	// WARNING: These are interpreted as radian values, so be sure to specify them not as degrees.
	
	// Create a quaternion based on the euler angles given.
	glm::quat q = glm::quat(rotFactor);

	// Rotate our quaternion by that quaternion's value.
	quaternion *= q;

	// Turn our quaternion into a mat4.
	rotation = glm::toMat4(quaternion);

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

// Sets the rotation matrix to a given value.
void GameObject::SetRotation(glm::mat4* rotMatrix)
{
	rotation = *rotMatrix;

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

// Sets the rotation matrix to a given value of x, y, and z radians.
void GameObject::SetRotation(glm::vec3 rotFactor)
{
	// WARNING: These are interpreted as radian values, so be sure to specify them not as degrees.

	// Set our quaternion equal to a quaternion created from the given euler angles.
	quaternion = glm::quat(rotFactor);

	// Turn our quaternion into a mat4.
	rotation = glm::toMat4(quaternion);

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

// Translates in the x, y, and z directions based on the given values.
void GameObject::Translate(glm::vec3 transFactor)
{
	// Translates the translation matrix.
	translation = glm::translate(translation, transFactor);

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

// Sets the translations to the exact x, y, and z position values given.
void GameObject::SetTranslation(glm::vec3 transFactor)
{
	// Translates the identity matrix.
	translation = glm::translate(glm::mat4(), transFactor);

	// Then we have to recalculate the transformation matrix.
	CalculateMatrices();
}

#endif // _GAME_OBJECT_CPP