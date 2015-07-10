/*
Title: Angular Friction (2D)
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
This is a demonstration of calculating and applying an angular frictional force to rigidbodies in
such a way that a rotation which is around an axis perpendicular to a surface can be diminished through friction. 
The demo contains a yellow circle which is supposed to be laying flat on the ground. The demo uses a modified version 
of the Coulomb Impulse-Based model of friction to simulate frictional forces between colliding bodies.

The algorithm Calculates & applies an angular frictional impulse similat to Coulomb's impulse based model of friction
however it has been adopted for angular friction. The algorithm calculates the friction due to flat-spin on a surface.
Because there is only one degree of angular freedom in a 2D game, we assume that the body is resting on another object and the surface
normal is in the Z direction.

References:
Gravitas: An extensible physics engine framework using object-oriented and design pattern-driven software architecture principles by Colin Vella, supervised by Dr. Ing. Adrian Muscat
NGen by Nicholas Gallagher
A Coulomb Based Model for Simulating Angular Friction Normal to a Surface by Nicholas Gallagher
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
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