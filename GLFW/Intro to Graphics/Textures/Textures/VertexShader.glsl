/*
Title: Textures
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
Introduces the idea of texturing by loading a .png texture with SOIL. The texture is 
then passed into glTexImage2D to create a 2D texture. In this example, we use glBlendFunc 
to control the alpha blending, since the .png file has transparency. Each vertex passed into 
the Vertex Shader will have UV texture coordinates as wel. These are passed through to the 
Fragment Shader, where they are used as the 2D index to sample color from the texture.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Our position in a vector3 specifying x, y, and z.
layout(location = 1) in vec2 in_tex;		// Our texture coordinates in a vector2 specifying u and v.

out vec2 texCoord; // We pass texture coordinates to the fragment shader, where they will actually be used.

uniform mat4 trans; // Our uniform transformation matrix.

void main(void)
{
	texCoord = in_tex;	// Copy the texture values over.
	gl_Position = trans * vec4(in_position, 1.0); // Pass out the vertices to the gl_Position after transforming them by our matrix.
}