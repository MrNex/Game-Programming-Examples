/*
Title: 3D rolling resistance
File Name: VertexShader.glsl
Copyright © 2015
Original authors: Srinivasan Thiagarajan
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
This program demonstrates how an object which is in rolling motion, 
experiences rolling friction and comes to a stop. This example build 
upon the previous example of "3D rolling friction". 

Rolling friction is caused by the imperfection of an rolling object
at the point of contact. Because of this, an object has a surface
which is in contact with the other object experiences a force which 
opposes the rotating motion. In a perfect scenario, if the object 
undergoes 0 deformation, the object should continue to roll forever.

The force (Rolling resistance coefficient) experienced by the object 
is dependant on the depth by which the object is inside the other 
object (deformation).
				Crr = sqrt(z/d)
z = sinkage depth
d = diameter of the rigid wheel

Since this is a computer simulation, z is 0, so the object never ceases 
rolling. We have to had code these values in our program. From a game 
perspective, its upto the designer.
With this constant, we can computethe force experienced due to rolling
 friction using :
				F = Crr x N
N = normal force.

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
Nicholas Gallagher
*/




#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

uniform	vec3 blue;

void main(void)
{
	color = in_color; // Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}
