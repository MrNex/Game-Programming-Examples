/*
Title: TextShaders2
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
This project does the same thing as the previous Text Shaders example, only 
it puts the shaders into actual .glsl files. The shader code is otherwise 
exactly the same, having hard-coded vertices and color. Two functions, 
readShader and createShader, are used to compile the shaders from the .glsl 
files. This simply renders a green triangle on a red background.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

out vec4 color; // Establishes the variable we will pass out of this hsader.
void main(void)
{
	color = vec4(0.0, 1.0, 0.0, 1.0); // Hardcoded color for simplicity.
}