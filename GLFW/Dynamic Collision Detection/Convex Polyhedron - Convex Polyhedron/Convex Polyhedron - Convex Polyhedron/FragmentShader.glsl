/*
Title: AABB - AABB (2D)
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
The demo contains two moving boxes, one pink, and one yellow.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving boxes to move any further.
when a moving box reaches one side of the screen, it will wrap around to the other side again.

The user can disable the continuous collision detection by holding spacebar. 
This will cause the program to run static collision detection at the end of every physics timestep.
This will not prevent tunnelling. When two boxes collide the user can cause the simulation
to continue by toggling continuous and noncontinuous collision (Release spacebar if pressed, tap and hold spacebar, 
then release).

This algorithm detects potentially missed collisions by performing a moving version of the
separating axis test. First we must determine the distances along each axis signifying
the distance to begin collision (dFirst) & the distance to separate from that collision (dLast). Then
we can easily determine the time at which these distances will be reached by dividing them by the magnitude of the
velocity along the axis (tFirst, tLast). If we keep the largest tFirst and the smallest tLast from all axes,
we will determine the time interval which the boxes will be intersecting! If tLast < tFirst, the boxes will not overlap.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
 uniform mat4 hue;	//Global hue control

void main(void)
{
	out_color = hue * color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}