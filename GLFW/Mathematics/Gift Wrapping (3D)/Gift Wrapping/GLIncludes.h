/*
Title: Gift Wrapping
File Name: main.cpp
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
This is a demonstration of implementing an algorithm known as Gift Wrapping.
The Gift Wrapping algorithm is a method of computing the smallest convex hull which contains 
a set of points in 3D. It is the 3D analog of the popular Jarvis March algorithm.

The Gift-Wrapping algorithm works by first computing an initial edge which is known to be on the convex hull.
The initial edge is calculated using the Jarvis March algorithm. Then, The algorithm 
creates faces attached to the existing edges which contain all other points on one side of them.

References:
A Direct Convex Hull Algorithm by John Henckel:
http://poorfox.com/tru-physics/hull.html
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

#endif _GL_INCLUDES_H