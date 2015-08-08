/*
Title: Fluid Simulation (SPH)
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
In this example we demonstrate the use of SPH to emulate fluid motion.
Fluid mechanics can be implemented in 2 ways: Eularian based or Lagragian 
based. Eularian based fluid simulation are done using a grid. There are points 
on the grid, and the particles contained within the grid follow a specific set
of rules. In Eularian approach, you need to account for conservation of mass 
explicitly. 
The lagragian approach accoutns for conservation of mass implicitly, Since each 
cluster of particles interact with each other and are separatly accountable.
The lagragian approach, considers the forces caused by all the surrounding particles.
It interpolates between the position of the surrounding particles to get the overall 
force acting on the selected particle. This process of interpolation is called SPH.

In SPH, we use smoothing kernels to inetrpolate based on the distance from the particle. 
We use different kernels for different "aspects" of fluid properties. For pressure, we 
implement a spike kernel, as the pressure sould increase drastically as the distance 
gets smaller. But we use a poly6 smoothing kernel for density distributions and surface 
tension. We use the gradient or laplacian of the kernel, based on whichever one is more applicable.
For more info see the referenced papers.

In SPH fluid simulation, the particles each consist of mass, velocity and acceleration.
The particles expereince density change, forces due to pressure, Viscosity, surface tension
and collision amongst themselves. 

Use "SPACE" to toggle gravity in x-axis, or use "W" to toggle gravity in y-axis.

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
Nicholas Gallagher
*/




#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

void main(void)
{
	color = vec4(0,0,1,1);//in_position.x, in_position.y, in_position.z ,1.0f);

	mat4 translation = mat4( 1.0f, 0.0f, 0.0f, 0.0f,
							 0.0f, 1.0f, 0.0f, 0.0f, 
							 0.0f, 0.0f, 1.0f, 0.0f,
							 in_position.x, in_position.y, in_position.z, 1.0);
	

	gl_Position = (MVP * translation) * vec4( 0.0f, 0.0f, 0.0f, 1.0);			 //w is 1.0, also notice cast to a vec4
}
