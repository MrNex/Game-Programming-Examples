/*
Title: 3D Sphere collision with MTV derivation and decoupling
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
This program demostrates how to calculate the MTV for two spheres colliding with each other.
It also demonstrates how to decouple objects after the collision is detected.
In this program, a much simpler version version of SAT is implemented. Unlike conventional SAT,
in this scenario, we only have to check for overlap along only 1 axis. the line connecting the
centers of the two spheres. This is because the sphere's projection would be the same on any vector.

Depending on the overlap, we can decide if the collision has occured or not. If collision has occured,
then the overlap can be used to compute the MTV by bultiplying with the unit direction vector from A to B.

Once collision has been detected, the other object can be moved by the MTV to decouple the objects.
This is done in this example to demostrate decoupling. Essentially, you should take the ratios of the object's
velocities and move them back by that much.

use " W, A, S, D " to rotate the sphere. This should have no effect on the algorithm as all poitns
are equidistant from the center. Use " U, O, I, K, J ,L " to move the sphre in Z, Y and X plane respectively.
Use Space to switch between the two spheres.

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
#define DIVISIONS  20

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