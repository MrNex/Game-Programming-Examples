/*
Title: Finite Element Method (1D)
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
This is a demonstration of using the finite element method to simulate deformable body physics.
The demo contains a beam made from 10 nodes which can be compressed and stretched.

The finite element method is an advanced, physically correct, and intensive numerical
algorithm used to take a complex problem and model it with a system of small simple problems
to approximate a solution.

Because of the given limitations of this example, we are able to pre-compute
most of the information needed at the startup of the program (notice the long startup time).
This means each physics timestep we simply solve a system of equations using the pre-computed information
and interpolate each nodes position using harmonic oscillation equations to simulate
the deformation of the body to an equilibrium state after external forces are applied.

The user can apply forces to the right end of the beam.
Hold the left mouse button to apply a force along the positive X axis.
Hold the right mouse button to apply a force along the negative X axis.

References:
Finite Element Analysis (MCEN 4173/5173) Fall 2006 course materials from University of Colorado Boulder as taught by Dr. H. "Jerry" Qi
PhysicsTimestep by Brockton Roth
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