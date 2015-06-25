/*
Title: Cone - Plane
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
This is a dmonstration of collision detection between a cone and a plane.
The demo contains a wireframe of a cone and a solid plane. When the objects are
not colliding the plane will appear blue and the cone will appear green. When
the two objects collide the plane will become pink and the cone will become yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. Lastly, you
can rotate the objects by clicking the left mouse button and dragging the mouse.

This demo detects collisions by computing the point on the cone's base most in the direction
of the plane, then determining if that extreme point and the tip of the cone are on the same side
of the plane. This is done by observing the sign of the dot product of the plane normal with both
the point on the tip of the cone and the extreme point on the base of the cone after the system has
been shifted to have it's origin on the center of the plane. If the signs are different the two points
reside on different sides of the plane and there must be a collision. Else, there is no collision.

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
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