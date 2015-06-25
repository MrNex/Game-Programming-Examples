/*
Title: Icosphere
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

References:
http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

Description:
Contains code for generating a 3D Icosahedron, and then refining the edges down to create
an Icosphere. This is a manner of generating a spherical-like object. By default, the number
of revisions on the icosphere is 5, which can create a pretty mesmerizing effect (because I
also have it randomizing the colors of each vertex and spinning around constantly). You can
also choose to lower or increase the number of revisions.

WARNING: Performance will drop painfully once you reach the 7-9 revisions range. I haven't been
able to push it past 9 revisions. Also note that this is incredibly inefficient in terms of the
way it generates the points, so there may be a long startup time for a high number of revisions.
Feel free to optimize it and send it back to me and I'll upload the better version!
*/

#ifndef _GL_INCLUDES_H
#define _GL_INCLUDES_H

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