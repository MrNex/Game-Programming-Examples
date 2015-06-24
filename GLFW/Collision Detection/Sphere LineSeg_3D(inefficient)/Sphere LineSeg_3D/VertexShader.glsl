/*
Title: Sphere-Line Segment (inefficient) 3D collision Detection
File Name: main.cpp
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
This is an example to detect the intersection of a line segment and circle in 3D.
This program uses seeveral techniques, and employs a series of test to ensure the collision of sphere
and triangle.
We have three separate checks in this example:
1) point-Sphere
2) line-Sphere
3) Plane sphere
This is an ineffective way to do this check, but its easier to understand.

References:
Real time collision Detection by Ericson
Nicholas Gallagher
AABB-2D by Brockton Roth
*/




#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

uniform	vec3 blue;

void main(void)
{
	color = in_color + vec4(blue,0.0f);	// Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}
