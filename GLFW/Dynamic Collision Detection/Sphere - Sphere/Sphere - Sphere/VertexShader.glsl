/*
Title: Sphere - Sphere
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
The demo contains two moving spheres, one pink, and one yellow.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving sphere to move any further.
When a sphere reaches the right side of the screen, it will wrap around to the left side again.

The user can disable the continuous collision detection by holding spacebar. 
This will cause the program to run static collision detection at the end of every physics timestep.
This will not prevent tunnelling. When two circles collide the user can cause the simulation
to continue by toggling continuous and noncontinuous collision (Release spacebar if pressed, tap and hold spacebar, 
then release).

The continuous collision detection algorithm used employs a technique known as interval halving.
First it is necessary that we get the relative movement, such that one sphere is moving at X speed
relative to the other being still. Following this, we perform the interval halving by starting the algorithm over the entire movement interval.
Over the given movement interval, this algorithm will surround the extent of the movement of the moving circle
with a bounding sphere. If this bounding circle still collides with the static sphere, the interval
is split into two halves and the function calls itself recursively on the smaller intervals.
Once the interval being tested gets to a range which is smaller or equal to a set interval epsilon based on desired accuracy
the function will exit, returning the end time (0.0f <= t <= 1.0f) of the smallest interval which first occurred.
If at any point the function does not detect a collision between the static sphere and the bounding sphere
in full interval before the exit condition is met, the function registers no collision.

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