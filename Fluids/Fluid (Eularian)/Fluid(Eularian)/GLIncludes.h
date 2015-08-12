/*
Title: Fluid Simulation (Eularian)
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
In this example we demonstrate the implementation of fluid motion using Eularian
approach. In this approach, we only deal with the velocity field in this example,
but the core concepts can be extended to other fields as well.

The velocity field experiences the 3 separate kinds of forces, namely diffusion,
advection and external forces.

In the Eularian approach, the particles do not have mass. The entire area is classified into
grids, the particles in a specific grid follow a the same path (have the same velocity).
The velocites of the grid consitute the velocity field.

Diffusion is the property of the of a fluid to spread a value accross the neighbors.
Advection is the property of a fluid to carry objects from one point to another.Self advection
is a part of the fluid motion.

use mouse to "Click and Drag" to add forces.
References:
Real-Time Fluid Dynamics for Games by Jos Stam
Nicholas Gallagher
*/


#ifndef _GL_INCLUDES_H
#define _GL_INCLUDES_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <random>
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
#define DIVISIONS  15

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