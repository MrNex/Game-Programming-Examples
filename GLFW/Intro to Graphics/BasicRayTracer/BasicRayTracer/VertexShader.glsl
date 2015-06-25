/*
Title: Basic Ray Tracer
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

References:
https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-(Part-I)

Description:
This program serves to demonstrate the concept of ray tracing. This
example is very basic, storing the triangles as a hardcoded constant
in the Fragment Shader itself. It draws a quad with the Vertex Shader,
and renders each pixel via tracing a ray through that pixel from the camera
position. There is no lighting.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

// This variable is used to constrict our points from the 0 to 1 range, as opposed to -1 to 1
const vec2 madd=vec2(0.5, 0.5);

// The input vertex coordinates.
layout(location = 0) in vec2 vertexIn;

// The coordinates for the fragment shader to use to determine where the pixel is on the quad.
out vec2 textureCoord;

void main(void)
{
	// Convert our coordinates from -1 to 1 range to 0 to 1 range.
	textureCoord = vertexIn.xy*madd+madd;

	// Pass out the vertex as a vec4.
	gl_Position = vec4(vertexIn.xy, 0.0, 1.0);
}