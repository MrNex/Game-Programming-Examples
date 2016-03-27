/*
Title: Deferred Shading
File Name: FragmentShader.glsl
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

//layout(location = 0) out vec4 Color; // Establishes the variable we will pass out of this shader.
layout(location = 0) out vec4 texPos;
layout(location = 1) out vec4 texNormal;
layout(location = 2) out vec4 texColor;
 
in vec3 out_pos;
in vec3 out_normal;
in vec4 out_color;	// Take in a vec4 for color

void main(void)
{
	texPos = vec4(out_pos,1.0f);
	texColor = out_color;
	texNormal = vec4(out_normal,1.0f);
	//Color = out_color; // Set our out_color equal to our in color, basically making this a pass-through shader.

	//gl_FragData[0]= out_color; // Set our out_color equal to our in color, basically making this a pass-through shader.
	//gl_FragData[1]= vec4(out_pos,1.0f);
	//gl_FragData[2]= vec4(out_normal,1.0f);
	//gl_FragData[3]= out_color;
}