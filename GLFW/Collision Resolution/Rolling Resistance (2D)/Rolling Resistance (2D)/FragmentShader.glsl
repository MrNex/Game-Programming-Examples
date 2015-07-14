/*
Title: Rolling Resistance(2D)
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
This is a demonstration of calculating and applying a rolling resistance angular impulse. The demo contains a yellow wireframe of
a circle and a pink ground. The ground has infinite mass and cannot be moved, while the yellow circle is subject to
various impulses and forces.

The algorithm Calculates & applies an angular impulse to inhibit rolling motion. This is done based on a coefficient of rolling resistance
given by design & the mass aswell as radius of the body. First a rolling resistance force is calculated, this is then translated to a torque.
Because we have already defined rolling mechanics, the torque applied which will inhibit angular motion will also slow the
linear velocity of a rolling object.

The user can press and hold the spacebar to apply a constant linear force at the yellow circles center of mass.

References:
Rolling-Friction-Resistance as presented by EngineeringToolbox
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