/*
Title: TextShaders2
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
This project does the same thing as the previous Text Shaders example, only 
it puts the shaders into actual .glsl files. The shader code is otherwise 
exactly the same, having hard-coded vertices and color. Two functions, 
readShader and createShader, are used to compile the shaders from the .glsl 
files. This simply renders a green triangle on a red background.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

void main(void)
{
	const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0), // Hardcoded vertices for simplicity.
									vec4(-0.25, -0.25, 0.5, 1.0),
									vec4(0.0, 0.25, 0.5, 1.0));

	gl_Position = vertices[gl_VertexID]; // Pass out the vertices to the gl_Position
}