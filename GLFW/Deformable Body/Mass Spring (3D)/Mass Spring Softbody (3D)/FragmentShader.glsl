/*
Title: Mass Spring Softbody (3D)
File Name: main.cpp
Copyright � 2015
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
This is a demonstration of using mass spring systems to simulate soft body physics.
The demo contains a blue structure made of a 6x10x6 grid of masses with springs connecting them.
The top of the structure is pinned in place and the rest is hanging freely.

Each physics timestep the mass spring system is solved to determine the force on each
individual point mass in the system. This is done using Hooke's law. The springs also contain 
dampening forces to help relax the system upon purterbation.

The user can apply forces to the bottom edge of the cloth.
Hold the left mouse button to apply a force along the positive X axis.
Hold the right mouse button to apply a force along the negative X axis.
Hold Left shift to switch the axis to the Y axis.
Press the up arrow to increase the rigidness of the structure.
Press the down arrow to decrease the rigidness of the structure.

References:
Game Physics by David Eberly
NGenVS by Nicholas Gallagher
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
 uniform mat4 hue;	//Global hue control

void main(void)
{
	out_color = hue * color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}