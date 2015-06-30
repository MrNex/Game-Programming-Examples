/*
Title: 3D Convex Hull SAT (minimum translation Vector derivation)
File Name: GLincludes.h
Copyright © 2015
Original authors: Srinivasan Thiagarajan
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
This is a program showing the implementaion of the SAT  (separating axis theorem)
on Convex Hull. It also includes the deduction of the minimum translation vector.
MTV is the vector by which either of the two objects need to move for there to be no collision.
MTV is essential for collision resolution. As we would require the point of collision
and for the proper resolution of collision. This also solves the issue of the
decoupling the obejcts after detecting collision.

In this example, what we do is find the projcetion of the objects on variosu axes and,
check if in any of the them, do they not over lap. If they don't overlap in even 1 axis,
then there exists 1 plane which separates the two objects. This algorithm is called SAT.
Here we the axes we take the projection along are, the face normals of all the faces
on both the obejcts and the cross product of each edge on one object to the every other
edge of the other object.

In this example, you can move one of the shapes and upon detecting collision, the program
will move the other obejct by the MVT. This will result in it looking like one object
pushing the other one.

Use "a,s,d" to rotate the selected object in x, y or z axis respectively. Use "w,a,s,d"
to move the object around in xy plane and "u and o" to move in z direction.
Use "Space" to switch between the two objects.

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
Nicholas Gallagher
*/


#ifndef _GL_INCLUDES_H
#define _GL_INCLUDES_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "glew\glew.h"
#include "glfw\glfw3.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\quaternion.hpp"
#include "glm\gtx\quaternion.hpp"
#include "glm\gtx\rotate_vector.hpp"


#define PI 3.14159265
#define DIVISIONS  40

// We create a VertexFormat struct, which defines how the data passed into the shader code wil be formatted
struct VertexFormat
{
	glm::vec4 color;	// A vector4 for color has 4 floats: red, green, blue, and alpha
	glm::vec3 position;	// A vector3 for position has 3 float: x, y, and z coordinates

	// Default constructor
	VertexFormat()
	{
		color = glm::vec4(0.0f);
		position = glm::vec3(0.0f);
	}

	// Constructor
	VertexFormat(const glm::vec3 &pos, const glm::vec4 &iColor)
	{
		position = pos;
		color = iColor;
	}
};

#endif _GL_INCLUDES_H