/*
Title: Convex Polygon - Sphere
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
This is a demonstration of collision detection between a polygon and a Sphere.
The demo contains a polygon and a wireframe of a sphere. When the objects are not 
colliding the sphere will appear blue and the polygon will appear green. When the 
two objects collide the sphere will become pink and the polygon will become yellow. 

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. By clicking the 
left mouse button and dragging the mouse you will rotate the selected shape. You can also
swap which shape is selected with spacebar. 

This algorithm tests for collisions between a sphere and a convex polygon through three different tests.
First we perform a halfspace test to determine if the sphere completely lies on one side of the polygon.
Then, if this is not the case, we project the sphere's center onto the plane in which the polygon lies &
determine if the projected center falls inside of the polygon. If it does we have a collision, if not however,
we must perform one more test: Find the closest point on every edge to the sphere & determine if that point
falls inside of the sphere using a distance test.

References:
Base by Srinivasan Thiagarajan
Sphere Collision 3D by Srinivasan Thiagarajan
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