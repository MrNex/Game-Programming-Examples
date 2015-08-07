/*
Title: Rotation Matrix
File Name: GLIncludes.h
Copyright © 2015
Original authors: Nicholas Gallagher
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
This is a demonstration of using a rotation matrix to describe an 
orientation in 3D space. The demo contains 3 lines representing the 
3 cardinal axes:	X (red), Y (green), and Z (blue)
Which can be rotated to any orientation.

A rotation matrix is a collection of 9 scalars arranged in a 3x3 matrix.
The rotation matrix has special properties including:
- Each column represents the object's local X, Y, and Z axes in world space
- A rotation applied from a matrix can easily be reversed by applying the
inverse of that matrix
- The inverse of a rotation matrix is the transpose
- Multiplying two rotation matrices together gets a rotation matrix which,
when applied, has the same result as the two separate matrices

In this simulation the Q and E buttons will alter the Y component of the axis of rotation
W and S will alter the X component of the axis of rotation
and A and D will alter the Z component of the axis of rotation

References:
3D Math Primer for Graphics and Game Development by Fletcher Dunn & Ian Parberry
Base by Srinivasan Thiagarajan
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