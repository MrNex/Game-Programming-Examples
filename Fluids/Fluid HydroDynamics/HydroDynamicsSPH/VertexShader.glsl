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
Description:
In this example we use the previous "Fluid Simulation (SPH)" example to
demonstrate the hydrodynamics of fluid.
Fluid when poured onto a a container which is connected to another by a
pipe along the bottom, the fluid flows into the second container until
the level of fluid is the same on both containers. This is because it
need to balance the pressure on both sides. The pressure on each side is
independant of the surface area and dependant on the height.
So, if external pressure is applied on any one side, the fluid level changes
or more accurately balances out the difference in p[ressure between the two containers.

In this example, all the fluid particles are released in one container and
they gradually flow into the adjacent container until there is equal liquid in both
the containers.

Press "SHIFT" to start simulation
Use "SPACE" to toggle gravity in x-axis, or use "W" to toggle gravity in y-axis.

Changing gravity in x-axis will cause all the fluid to flow into the left container.

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
