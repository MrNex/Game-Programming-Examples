/*
Title: 3D Sphere collision with MTV derivation and decoupling
File Name: Fragmentshader.glsl
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
This program demostrates how to calculate the MTV for two spheres colliding with each other.
It also demonstrates how to decouple objects after the collision is detected.
In this program, a much simpler version version of SAT is implemented. Unlike conventional SAT,
in this scenario, we only have to check for overlap along only 1 axis. the line connecting the
centers of the two spheres. This is because the sphere's projection would be the same on any vector. 

Depending on the overlap, we can decide if the collision has occured or not. If collision has occured,
then the overlap can be used to compute the MTV by bultiplying with the unit direction vector from A to B.

Once collision has been detected, the other object can be moved by the MTV to decouple the objects.
This is done in this example to demostrate decoupling. Essentially, you should take the ratios of the object's
velocities and move them back by that much.

use " W, A, S, D " to rotate the sphere. This should have no effect on the algorithm as all poitns 
are equidistant from the center. Use " U, O, I, K, J ,L " to move the sphre in Z, Y and X plane respectively.
Use Space to switch between the two spheres.

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
Nicholas Gallagher
*/


#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}