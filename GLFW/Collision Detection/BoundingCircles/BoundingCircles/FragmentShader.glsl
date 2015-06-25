/*
Title: Bounding Circles
File Name: FragmentShader.glsl
Copyright © 2015
Original authors: Brockton Roth
Revision authors: Nicholas Gallagher
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
This is a bounding circle collision test.  This is in 2D.
Contains two circles, both . They are bounded by circles, and when these circles collide they will change colors to red.
The larger circle can be moved using the arrow keys, likewise the smaller circle can be moved with WASD.
The algorithm will detect any type of collision, including containment.
The circles should be the exact same as their bounding circles, since they circles any rotation applied to the objects will not make a difference.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color

uniform mat4 hue;	//Matrix to alter color of vertices
 
void main(void)
{
	out_color = hue * color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}