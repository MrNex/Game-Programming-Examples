/*
Title: Runge-Kutta 4 integration method
File Name: FragmentShader.glsl
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
Euler integration method. The runge kutta method is of fourth order. Euler integrator,
takes the value of the function F(x,y(x)) and integrates it over the time step T.
Unlike Euler integrator, RK integrator, divides the entire time step into 3 parts and 
averages the slope of the velocity It uses that value of the y(x) to integrate over
the entire time step T. This results in having a reduced margin of error.

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

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}