/*
Title: Circle - Triangle (2D)
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
This is a demonstration of using continuous collision detection to prevent tunnelling.
The demo contains two a pink moving circle, and a yellow moving triangle.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving circle to move any further.
If the moving circle reaches the right side of the screen, it will wrap around to the left side again.

The user can disable collision detection by holding spacebar.

The continuous collision detection algorithm uses the concept of Minkowski Sums
in order to test for collision between a moving circle and a moving triangle.
First we get the relative velocity of the circle with respect to the triangle.
Then we create two line segments for each triangle edge positioned at +- the radius of the circle
in the direction of the vector perpendicular to the edge. Then we test for collisions of the
line segment created by the movement of the center point of the circle & the 6 edge line segments.
If no collision is detected we must also create three circles centered at the vertices of the triangle
and test for collisions between the line segment created by the movement of the center point of the circle
and these three circles.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson

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