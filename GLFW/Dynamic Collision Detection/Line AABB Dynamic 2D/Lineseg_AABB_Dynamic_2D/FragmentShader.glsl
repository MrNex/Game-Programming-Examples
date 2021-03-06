/*
Title: Line Segment - AABB Dynamic 2D collision Detection
File Name: Fragmentshader.glsl
Copyright � 2015
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
This example demonstrates the collision between a stationary line and a 
axis aligned bounding box. We use the same approach we used in determining 
the collision between a line and an AABB.
First we find out if the rectangle before and after the collision lies
completly on one side of the line.
second, we check if both the before and after timeStep the box lies on the
same Side of the triangle. If it does, then it is not colliding.

We determine the relative position of the rectangle with respect to line, 
using dot products of the 4 points to the normal of the line. If all the dot
products are of same sign then, then all 4 points lie on the same side.

Use "left Shift" to toggle the intergration mode from automatic to manual. 
Use "space" to move ahead by 1 timestep.

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
*/


#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}