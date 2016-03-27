/*
Title: Deferred Shading
File Name: LightVertexShader.glsl
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
layout(location = 1) in vec2 in_texCoord;

out vec2 texCoord;

void main(void)
{
	texCoord = in_texCoord;	// Pass the texture coordinate through
	gl_Position = vec4(in_position, 1.0);
}