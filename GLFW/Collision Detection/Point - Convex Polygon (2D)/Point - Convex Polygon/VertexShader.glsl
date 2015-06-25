/*
/*
Title: Point - Triangle (Normal Method)
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
This is a demonstration of collision detection between a point and a triangle.
The demo contains a triangle. When the mouse is not detected as colliding with
the triangle, the triangle  will appear green. When the mouse is detected as colliding 
with the triangle, the triangle will become yellow. 

The triangle is able to be moved. you can translate the triangle in the XY plane with WASD.
The triangle can be rotated by with the Q and E keys. 

This algorithm tests for collisions between a point and a triangle by using the cross product
to determine the direction of a cycle of points. Given a point P and a triangle ABC
you can make three other triangles PAB, PBC, and PCA. If P is contained in triangle ABC
then the other triangles must all have the same direction of points (clockwise or counter-clockwise).
We test this by taking two edges of each sub-triangle and taking the cross product of those edges
to get then triangle normal. Then we are able to take the dot product of the normals, and if the dot
product returns a number which is less than 0, the normals point different directions and no collision is
possible. However if all dot products return numbers greater than 0, the point must be contained
in the triangle ABC.

This is a more geometric approach to the solution of this problem. For a more mathematical approach
please see the Barycentric Method.

References:
Real Time Collision Detection by Christer Ericson
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

void main(void)
{
	color = in_color;	// Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}