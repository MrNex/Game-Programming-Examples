/*
Title: Line - Triangle (3D)
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
This is a demonstration of collision detection between a line and a triangle.
The demo contains a line and a triangle. When the objects are not 
colliding the triangle will appear blue and the line will appear green. When the 
two objects collide the triangle will become pink and the line will become yellow. 

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. The shapes
can be rotated by clicking and dragging the left mouse button. You can swap which shape is 
selected with spacebar. 

This algorithm tests for collisions between a line and an triangle by determining if
the point on the line which intersects the plane which the triangle lies in is contained within the triangle.
We are able to do this by observing and comparing the signs of a scalar triple product between vectors going from
the line to the points of the triangle & the direction of the line.

References:
Base by Srinivasan Thiagarajan
Plane - OBB by Nicholas Gallagher
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