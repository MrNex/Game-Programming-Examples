/*
Title: AABB - Circle Dynamic 2D collision Detection
File Name: VertexShader.glsl
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
This example demonstrates the dynamic collision detection between a circle 
and a axis aligned bounding box. This test is composed of three tests. First,
to check if the starting point is colliding with the rectangle. Second, Check
if the end state is colliding with the rectangle.Now that we know that the start
and the end state don't collide, then we can presumme that the collision is
occuring along the path if occuring at all. We take the path of the circle (relative
 to the the box) and find the path followed by the end points on the circle which are 
 prependicular to the the movement. Following these poitns we, get two lines. If any 
 of these lines intersect with the box, then the circle will collide in the next timestep.

 use "w,a,s and d " to move the circle. Consider each movement to be done in one timestep.
 If the program detects a collision will occur in the next timestep, then it wont move the circle.

References:
Real time collision Detection by Ericson
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
	color = in_color; // Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}
