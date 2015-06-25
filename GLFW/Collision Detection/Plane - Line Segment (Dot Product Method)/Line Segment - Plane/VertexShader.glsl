/*
Title: Line Segment - Plane
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
This is a demonstration of a line segement and a Plane. 
The demo contains a line and a plane. The plane is colored blue
And the line is colored green until the two collide, then the plane will change to the color
pink and the line will change to the color yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD
and along the Z axis with Left Shift & Left Control. You can also rotate the selected shape by clicking
and dragging the mouse.

This demo detects collisions by making sure the endpoints of the line lie on the same side of the plane.
We do this by translating the endpoints of the box into world space and the normal of the plane
into world space, then translating the entire system to to be centered on the origin of the plane.
From here, you are able to determine what side of the plane the endpoints fall on by observing the sign of dot
product of the endpoint's position vectors with the plane's normal. If one sign doesn't match the other,
there is a collision.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
2D Game Collision Detection by Thomas Schwarzl
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