/*
Title: AABB-Plane dynamic 3D collision Detection
File Name: FragmentShader.glsl
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
This is an example to demonstrate the implementation of dynamic collision detection between a
AABB and a plane.In this example we just detect if a collision is occouring or not. We
calculate the positions of AABB before and after the time step. Then we just detect if the two
points lie on the same side of plane. If they do, then they collide, else they dont.

Again, this method is only good to detect collision.This example can be built upon to find the 
point of collision.

Use "SPACE" to move ahead by one time step. Use mouse "Click and drag" to rotate the plane.

References:
Real time collision Detection by Ericson
Nicholas Gallagher
AABB-2D by Brockton Roth
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}