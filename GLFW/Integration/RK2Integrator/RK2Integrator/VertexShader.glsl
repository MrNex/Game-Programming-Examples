/*
Title: Runge-Kutta / Midpoint integration method 
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
This is a program that shows the difference in integration mechanisms Runge kutta and 
Euler integration method. The runge kutta method is of second order. Euler integrator,
takes the value of the function F(x,y(x)) and integrates it over the time step T.
Unlike Euler integrator, RK integrator, takes the value of the variable y(x) at the 
middle of the time step T i.e T/2. It uses that value of the y(x) to integrate over
the entire time step T. This results in having a reduced margin of error of the order of 3.

You can see the error margin between the two techniques. The red line uses the RK method, 
while the blue line uses euler integrator. The red line is closer to its precise implementation. 

use "space" to move one time Step.

References:
Nicholas Gallagher
Book : physics based animation by Kenny Erleben,Jon Sporring, Knud Henriksen and Henrik ;
Wikipedia : https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods ;
https://en.wikipedia.org/wiki/Midpoint_method

*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform vec3 colorAndPos; // Our uniform MVP matrix to modify our position values

void main(void)
{
	color = in_color + vec4(0.0f,0.0f,colorAndPos.z,0.0f);	// Pass the color through
	gl_Position = vec4(in_position.xy + colorAndPos.xy,0.0f, 1.0); //w is 1.0, also notice cast to a vec4
}