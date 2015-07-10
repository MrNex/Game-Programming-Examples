/*
Title: Rolling (2D)
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
This is a demonstration of calculating and applying a linear frictional force to colliding rigidbodies in
such a way that rolling can be facilitated. The demo contains a yellow circle & a pink rectangle. The pink rectangle is
supposed to be the ground, which in turn has infinite mass. The demo uses the Coulomb Impulse-Based model of friction to
simulate frictional forces between colliding bodies.

The algorithm Calculates & applies a linear frictional impulse according to Coulomb's impulse based model of friction.
This includes calculations of the linear velocity at the point of collision due to angular motion of an object to facilitate a rolling motion.
Aswell as calculations of changes in angular velocity due to friction at a point on the surface of an object.

References:
Gravitas: An extensible physics engine framework using object-oriented and design pattern-driven software architecture principles by Colin Vella, supervised by Dr. Ing. Adrian Muscat
NGen by Nicholas Gallagher
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