/*
Title: Standard3D
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
Builds upon the "Standard2D" project and renders a 3D cube, spinning about the 
X-Y axis. This uses a uniform transformation matrix. This project is more about 
getting started and understanding what a basic OpenGL program looks like.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
 layout(location = 0) in vec3 in_position;
 layout(location = 1) in vec4 in_color;

 out vec4 color;

 uniform mat4 trans;

void main(void)
{
	color = in_color;
	gl_Position = trans * vec4(in_position, 1.0);//w is 1.0, also notice cast to a vec4 // Pass out the vertices to the gl_Position
}