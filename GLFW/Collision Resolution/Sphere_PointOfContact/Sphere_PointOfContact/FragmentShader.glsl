/*
Title: 3D Sphere collision detection with point of contact derivation
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
This example build upon the previous example of deriving the MTV for colision 
between two spheres and decoupling them (3D Sphere collision with MTV derivation
and decoupling). This example demonstrates deriving the point of collision from
the MTV after decoupling the spheres.

Once the two spheres have been moved in such a way that they are now in justcontact with each other,
then the point of contact is at a distance of radius from the center of A in the direction pointed by MTV.

In this program, the point of contact will blink as soon as the collision occours.

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