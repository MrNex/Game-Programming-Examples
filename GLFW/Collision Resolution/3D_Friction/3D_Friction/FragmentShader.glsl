/*
Title: 3D Friction
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
This program demonstrates the implementation of friction in games. 
This is built upon the previous example of "3D Convex Hull collision 
resolution". 
We use coulomb's impulse based frition model to implement friction.
The advantage of this model, is that the frictional force is added as 
an impulse rather than force. This impulse can be added during the 
collision resolution phase.

The friction is applied at the point of contact and is calculated as 
follows:
If the object is not currently in motion parrallely to the object,
then the object does not experience static friction, it experiences 
dynamic friction,(no matter the relative velocity). If the tangential 
force is less than or equal to the normal (reactive) force, then the 
object expereinces a static friction equal tothe force applied and 
does not move.

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