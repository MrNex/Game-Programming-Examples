/*
Title: Euler Angles
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
This is a demonstration of using euler angles (pronounced "oiler"-- by the way)
to describe an orientation in 3D space. The demo contains 3 lines representing 
the 3 cardinal axes:	X (red), Y (green), and Z (blue)
Which can be rotated to any orientation.

Euler angles are 3 scalar values which measure angles. The first angle,
the heading, measure the angle of rotation around the Y axis from the 
initial orientation (the frame of reference created by the cardinal axes X,Y,Z)
to the objects final orientation (X'Y'Z'). The second angle, the Elevation, measures 
the angle of rotation around the X axis from the initial X-Z plane to the 
X'-Z' plane created by the object's final orientation. The third and final
angle, the Roll, is the angle of rotation around the Z' axis to reach the final
orientation. It should be noted that the Z' axis is the Z axis of the object in worldspace, 
not the Z axis of the world.

An interesting side effect of euler angles is called Gimbal Lock, which is able to be experienced
in this simulation. Gimble lock is due to the fact that the axis of rotation of the heading never
changes, but the axis of rotation of the Roll does. As a result of this, it is possible to cause
the axis of rotation of the Roll to change in such a way that it has the same axis of rotation
as the heading. When this happens a degree of freedom is lost and it is only possible
to rotate about 2 linearly independent independent directions rather than 3.

In this simulation the Q and E buttons will alter the heading
W and S will alter the pitch
and A and D will alter the roll

In order to experience Gimble Lock, alter the pitch using W or S
such that the Z' axis (blue axis) is pointing straight up or straight down.
Once this is achieved you can see that the Q and E buttons will
accomplish the same action as the A and D buttons.

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