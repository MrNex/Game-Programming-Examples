/*
Title: Deferred Shading
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
This example demonstrates the implementation of deferred shading
rendering technique. In this example we render two spheres.

In the first pass, we store the position, normal and diffuse color
on separate textures. This is done by attaching three separate textures
to a frame buffer object. and then rendering to the frame buffer.

In the second pass, we get the position, normal and diffuse color by
sampling the three textures, and use those to compute the final color
of that particular pixel based on lighting calculations.

References:
OpenGL 4 Shading language Cookbook
*/


#ifndef _GL_INCLUDES_H
#define _GL_INCLUDES_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <soil\SOIL.h>
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
	glm::vec3 position;	// A vector3 for position has 3 float: x, y, and z coordinates
	glm::vec3 normal;	// A vector3 for surface normals has 3 floats too.
	glm::vec4 color;	// A vector4 for color has 4 floats: red, green, blue, and alpha
	
	// Default constructor
	VertexFormat()
	{
		position = glm::vec3(0.0f);
		normal = glm::vec3(0);
		color = glm::vec4(0.0f);
	}

	// Constructor
	VertexFormat(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec4 &iColor)
	{
		position = pos;
		normal = norm;
		color = iColor;
	}
};

#endif _GL_INCLUDES_H