/*
Title: Graham-Schmidt Process
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
This is a demonstration of an implementation of the Graham-Schmidt process.
The demo contains 3 axes which kept mutually perpendicular through the Graham-Schmidt
process. The red axis represents the axis which is controlled by user input.

The Graham Schmidt process is an algorithm for taking a single vector and computing
the rest of the vectors needed to span that space. It is useful for generating vectors
which represent a coordinate system of a plane.

The red axis can be rotated about the Y axis by holding the left mouse button
and moving the mouse left and right. The red axis can be rotated about the X axis
by holding the left mouse button and moving the mouse up and down.

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection - Christer Ericson
AABB-2D by Brockton Roth
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
 uniform mat4 hue;	//Global hue control

void main(void)
{
	out_color = hue * color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}