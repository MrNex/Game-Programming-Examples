/*
Title: Cloth Simulation using Compute Shaders
File Name: GLIncludes.h
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
This program demonstrates the implementation of cloth like surface using
compute shaders. The cloth is considered to be made up of a mesh of springs.
Each point represents a point mass on the cloth which is connected to 4
other point masses (top, bottom, left and right).

Each point mass computes the force exerted on it due to the 4 springs, which
connect the point mass to the neighbouring masses. This computation is done
in GPU, namely the compute shader.

In this example, we allocate the required memory space in GPU buffers. We make
4 buffers: One to read position data from and one to write position data to.
similarly for velocity of each particle. We have two separate buffers to read
and write to avoid data races and read-before-write errors.

We send the data to one of the buffers once, then compute the position and velocity
at the of the frame in the sahder and store it in the output buffers. Then we
bind the output buffer to the GPU ARRAY BUFFER, and tell the GPU "how to" read the data,
i.e. set attribute pointers and enable them.

Then we simply call the drawArrays function of openGL. Notice how we never read
from the buffers on the CPU side of the applicaiton. This is the advantage of using
shaders in this type of situations: We avoid unnecessary transfer of data from CPU to GPU.

References:
OpenGL 4 shading language cookbook by David Wolff
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