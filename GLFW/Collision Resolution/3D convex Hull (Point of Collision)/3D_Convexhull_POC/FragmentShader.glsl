/*
Title: 3D Convex Hull SAT (Point of Collision Derivation)
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
This example builds on the previous example of "3D convex hull SAT (MTV derivation)"
and it also derives the point of contact after decoupling the objects after detecting 
collision. 

There are three possible scenarios, in win which the two objects can collide. 
1) Point-face
2) Face-Face or Edge-face
3) Edge-Edge

For point to face scenario, we take the projection of all the points of the first obejct 
on the MTV. the point wit hthe minimum value is considered to be the point of collision.

For Face to Face or edge to face scenario, we use the Gram-Schmidt process to deduce the 
point of collision. Since in Face to face collision, we cannot give a single vertex as the 
point of collision as this causes inaccurate collision resolution results. So, we deduce 
the center of the area colliding. We use the Gram-Schmidt process to deduce the the three 
basis vector with the MTV being one of them. Now we find the projection of the vertices on
the two axis basis vectors other than the MTV. We use the lower value of the overlap plus 
half the over lap multiplied by that unit vector gives one component of the position of the 
point. Doing this for the two of the basis vectors gives us two components and adding the 
distance of the plane containing the overlapped area gives us the position of the point 
of collision in worldspace.

for edge to edge scenario, we use simple linesegment collision detection to detect the point
of collision. Although this is slightly modified because we already know that hte collision
is occuring, so we need not check it again.

In this example, you can move one of the shapes and upon detecting collision, the program 
will move the other obejct by the MVT. This will result in it looking like one object 
pushing the other one. The point of collision will be highlighted by a RED dot.

Use "a,s,d" to rotate the selected object in x, y or z axis respectively. Use "w,a,s,d" 
to move the object around in xy plane and "u and o" to move in z direction.
Use "Space" to switch between the two objects.

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