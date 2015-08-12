/*
Title: Fluid Simulation (Eularian)
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
In this example we demonstrate the implementation of fluid motion using Eularian 
approach. In this approach, we only deal with the velocity field in this example,
but the core concepts can be extended to other fields as well. 

The velocity field experiences the 3 separate kinds of forces, namely diffusion,
advection and external forces.

In the Eularian approach, the particles do not have mass. The entire area is classified into 
grids, the particles in a specific grid follow a the same path (have the same velocity).
The velocites of the grid consitute the velocity field.

Diffusion is the property of the of a fluid to spread a value accross the neighbors. 
Advection is the property of a fluid to carry objects from one point to another.Self advection 
is a part of the fluid motion.

use mouse to "Click and Drag" to add forces.
References:
Nicholas Gallagher
Real-Time Fluid Dynamics for Games by Jos Stam
*/




#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

void main(void)
{
	color = vec4((in_position.y - 4.5f) * 0.2f , (in_position.y - 4.5f) *0.2f , (in_position.y * 1.5f) + 0.5f ,1.0f);

	mat4 translation = mat4( 1.0f, 0.0f, 0.0f, 0.0f,
							 0.0f, 1.0f, 0.0f, 0.0f, 
							 0.0f, 0.0f, 1.0f, 0.0f,
							 in_position.x, in_position.y, in_position.z, 1.0);
	

	gl_Position = (MVP * translation) * vec4( 0.0f, 0.0f, 0.0f, 1.0);			 //w is 1.0, also notice cast to a vec4
}
