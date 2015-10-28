/*
Title: Gaussian Blur
File Name: VertexShader.glsl
Copyright © 2015
Original authors: Srinivasan T
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
This demo demonstrates the implementation of gaussian blur filter using fragment shader.
To implement a blur effect,we avarage the color values of a pixel with the values of the
nearby pixels. Gaussian blur just implements this "averaging between pixels" using gaussian
weights, which are computed on the CPU side application and sent to the GPU as uniforms.

To implement gaussian blur, we need to implement 2 dimensional gaussian equation. This would require
a NxN sampling calls of a texture. where N is the number of pixels we average for one pixel in the final image.

We reduce this problem to an order of N, by adding another render call. in The first render
call we average the values of the pixels horizontally. and in the second render call, we do this vertically.
This is equivalent to implementing a 2D gaussian function.

In this program, we first render the scene normally onto a texture using a frame buffer, and apply horizontal
blur on it and render this onto another texture. On the third render call, we apply vertical blur and then we
render this texture on a plane which covers the entire scene/window.

Use "SPACEBAR" to toggle blur on and off.

References:
OpenGL 4 Shading language cookbook, Second Edition
*/

#version 430 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Our position in a vector3 specifying x, y, and z.
layout(location = 1) in vec2 in_tex;		// Our texture coordinates in a vector2 specifying u and v.

out vec2 texCoord; // We pass texture coordinates to the fragment shader, where they will actually be used.

uniform mat4 trans; // Our uniform transformation matrix.

void main(void)
{
	texCoord = in_tex;	// Copy the texture values over.
	gl_Position = trans * vec4(in_position, 1.0); // Pass out the vertices to the gl_Position after transforming them by our matrix.
}