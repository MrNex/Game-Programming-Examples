/*
Title: Sphere-AABB 3D collision Detection
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
This is a collision test between a sphere and a Oriented bounding box in 3D.
We Find the closest point on the box, closest to the center of the circle. To
do this, we bring the circle from the world coordinate system, to the box's 
model co-ordinate system. From there on is is similar to the Sphere and AABB collision check.
We check if that point lies on/inside the sphere or outside the sphere. 
If it lies within the sphere, then collision is detected. else no collision has occoured.
The movable sphere changes color when a collision is detected.

Use Mouse to move in x-y plane, and "w and s" to move in z axis.

References:
AABB-2D by Brockton Roth
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}