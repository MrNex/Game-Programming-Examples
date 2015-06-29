/*
Title: Buoyancy Example
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
This is a program showing the implementation of the bouyant force in fluids.
This example has a sphere which represents the mass, and the fluid is represented
by the blue box. The parameters for the fluid like density, and the parameters for
the ball, like mass and size can be changed from the first code block.

Buoyancy is the property of fluid to extert force on an object when submerged in the fuid.
This force is equal to the weight of water displaced by the object. The mass of the water 
displaced can be computed by the volume of the fluid displaced multiplied by the density 
of the fluid. The "weight" can be computed by multiplying the mass and gravity. This gives
us the magnitude of the buoyant force acting on the object(ball).

Buoyant force always acts in the direction opposite to gravity. So, multiplying the magnitude
with the unit vector against the gravitional force gives us the buoyant force. 
Adding this force to the all the other forces acting on the body (in this case gravitational pull)
gives us the behaviour we expect. 

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
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
