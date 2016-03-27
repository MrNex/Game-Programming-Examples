/*
Title: Deferred Shading
File Name: VertexShader.glsl
Copyright © 2015
Original authors: Brockton Roth
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
This example demonstrates the implementation of deferred shading 
rendering technique. In this example we render two spheres.

In the first pass, we store the position, normal and diffuse color 
on separate textures. This is done by attaching three separate textures
to a frame buffer object. and then rendering to the frame buffer.

In the second pass, we get the position, normal and diffuse color by 
sampling the three textures, and use those to compute the final color 
of that particular pixel based on lighting calculations. 

References:
OpenGL 4 Shading language Cookbook
*/


#version 430 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;		// Get in a vec4 for color

out vec3 out_pos;
out vec3 out_normal;
out vec4 out_color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

void main(void)
{
	out_pos = in_position;
	out_normal = in_normal;
	out_color = in_color;	// Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}