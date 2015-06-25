/*
Title: Line - AABB Collision Detection
File Name: VertexShader.glsl
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
This is a collision detection algorithm between a line and an axis-aligned bounding box in 2D.
By determining which side of the line the corners of the AABB fall on, we can tell if they are colliding.
If two corners fall on two different sides of the line, then the line and the box must be colliding.
We can determine which side of the line corners fall on by observing the sign of the dot product of each corner 
with the normal of the line. The objects will appear green when not colliding, and red once they are colliding.

Use WASD to move the line, and Q and E to rotate the line.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
2D Game Collision Detection by Thomas Schwarzl
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