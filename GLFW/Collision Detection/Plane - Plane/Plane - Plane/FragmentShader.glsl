/*
Title: Plane - Plane
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
This is a demonstration of collision detection between two planes.
When the objects are not colliding one plane will appear blue and the
other will appear green. When the two objects collide one will 
become pink and the other will become yellow. Keep in mind that planes extend
infinitely in two directions, but to represent them in a cleaner manner
the meshes do not extend infinitely.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. Lastly, you
can rotate the objects by clicking the left mouse button and dragging the mouse. It is
worth noting that Planes almost always collide in 3D. This means as soon as you
rotate a plane it will be difficult- if not impossible to get them to not collide again.

This demo detects collisions between planes by first testing if their normals are the same. 
If two planes do not have the same (or anti-parallel) normals they must collide 
(remember, planes extend infinitely in two directions). If two planes do have the same
or antiparallel normals they may still be colliding if and only if they are on top of each other.
This is just a matter of testing if the center of one plane lies on another plane. We can do this
using the dot product and the mathematical definition of a plane.

References:
Base by Srinivasan Thiagarajan
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