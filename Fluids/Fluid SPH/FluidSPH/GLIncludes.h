/*
Title: Fluid Simulation (SPH)
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
In this example we demonstrate the use of SPH to emulate fluid motion.
Fluid mechanics can be implemented in 2 ways: Eularian based or Lagragian
based. Eularian based fluid simulation are done using a grid. There are points
on the grid, and the particles contained within the grid follow a specific set
of rules. In Eularian approach, you need to account for conservation of mass
explicitly.
The lagragian approach accoutns for conservation of mass implicitly, Since each
cluster of particles interact with each other and are separatly accountable.
The lagragian approach, considers the forces caused by all the surrounding particles.
It interpolates between the position of the surrounding particles to get the overall
force acting on the selected particle. This process of interpolation is called SPH.

In SPH, we use smoothing kernels to inetrpolate based on the distance from the particle.
We use different kernels for different "aspects" of fluid properties. For pressure, we
implement a spike kernel, as the pressure sould increase drastically as the distance
gets smaller. But we use a poly6 smoothing kernel for density distributions and surface
tension. We use the gradient or laplacian of the kernel, based on whichever one is more applicable.
For more info see the referenced papers.

In SPH fluid simulation, the particles each consist of mass, velocity and acceleration.
The particles expereince density change, forces due to pressure, Viscosity, surface tension
and collision amongst themselves.

Use "SPACE" to toggle gravity in x-axis, or use "W" to toggle gravity in y-axis.

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
#include <cmath>
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