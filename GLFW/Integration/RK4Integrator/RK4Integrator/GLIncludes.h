/*
Title: Runge-Kutta 4 integration method
File Name: GLIncludes.h
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
This is a program that shows the difference in integration mechanisms Runge kutta and
Euler integration method. The runge kutta method is of fourth order. Euler integrator,
takes the value of the function F(x,y(x)) and integrates it over the time step T.
Unlike Euler integrator, RK integrator, divides the entire time step into 3 parts and
averages the slope of the velocity It uses that value of the y(x) to integrate over
the entire time step T. This results in having a reduced margin of error.

You can see the error margin between the two techniques. The red line uses the RK method,
while the blue line uses euler integrator. The red line is closer to its precise implementation.

use "space" to move one time Step.

References:
Nicholas Gallagher
Book : physics based animation by Kenny Erleben,Jon Sporring, Knud Henriksen and Henrik ;
Wikipedia : https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods ;
https://en.wikipedia.org/wiki/Midpoint_method

*/

#pragma once
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