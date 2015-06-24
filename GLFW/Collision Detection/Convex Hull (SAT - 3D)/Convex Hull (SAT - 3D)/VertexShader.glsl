/*
Title: Convex Hull (SAT - 3D)
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
This is a demonstration of convex hull collision detection between two convex polygons.
The demo contains a wireframe of a frustum and a tetrehedron. Both appear green.
You can move the shapes around the X-Y plane with WASD, and along the Z axis with
Left Shift & Left Control. Pressing space will toggle the shape being moved.
You can also rotate the selected shape by holding the left mouse button and dragging the mouse.

This demo uses the separating axis theorem test for collision detection. The
Theorem states that if you are able to separate two polygons by a plane then 
they must not be colliding. To test this, we develop a collection of potential axes which
the shapes might overlap if projected upon, and if each axis in our collection detects
an overlap between the shapes once they are projected onto it, there must be a collision.
However if there is a single axis which does not detect overlap then there must not be a collision.
The collection of necessary axes to test include the face normals of both polygons as well as the
edge normals which are also normal to an edge on the opposite polygon.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
NGen by Nicholas Gallagher
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