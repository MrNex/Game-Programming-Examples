/*
Title: Point - Plane
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
This is a demonstration of collision detection between a point and a plane.
When the objects are not colliding the plane will appear blue and the
point will appear green. When the two objects collide the plane will 
become pink and the point will become yellow. Keep in mind that planes extend
infinitely in two directions, but to represent them in a cleaner manner
the meshes do not extend infinitely.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. You can
swap which shape is selected with spacebar. Lastly, you can rotate the objects 
by clicking the left mouse button and dragging the mouse. 

This algorithm tests collisions between a point and a plane by using the
mathematical definition of a plane. First, we get the normal of the plane in world space.
Then we must shift both objects such that the plane is at the origin of the coordinate system.
Finally, we can perform a dot product of the point with the normal. If the dot product is zero
then we have a collision.

Points represent ifinitesimal volumes and are supposed to indicate exact positions instead.
Therefore, because a point would theoretically have no volume (or the smallest measurable amount)
it is very difficult for a point to exactly intersect a plane (which is infinitely thin).
The point being drawn on the screen is a much larger representation of the point which
actually exists in that space. To make the representation of the point accurately display the 
intersection of the point and the plane, we must accept all non-collisions within a certain range
as collisions.

References:
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