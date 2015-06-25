/*
Title: BasicComputeShader
File Name: ComputeShader.glsl
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
Introduces the concept of a compute shader, which can take in arbitrary data, 
operate on it in parallel, then return some data. In this example, the compute 
shader exists mostly as a pass-through shader, but it also modifies the position 
of each vertex by a factor of 1.001, slowly growing the square. The compute 
shader uses a Shader Storage Buffer Object (SSBO) which it can read from and write to.

WARNING: Requires OpenGL version 4.3 or later for the compute shader to compile.
*/

#version 430 // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

struct VertexData
{
	vec4 color;	// A vector4 for color has 4 floats: red, green, blue, and alpha
	vec3 position;	// A vector3 for position has 3 float: x, y, and z coordinates
};

// A layout describing the buffer we're accessing. Note that there are no in/out variables here.
layout(std140, binding = 0) buffer destBuffer
{
	VertexData data[];
} outBuffer;

// A layout describing the local size, which is basically the number of invocations that will occur within each work group. Here we have 2 invocations
// This is hard to explain with comments, if you're interested, Google up on Compute Shaders, Local Size, and Work Groups. Also look into Graphics Cards and how the hardware works.
layout(local_size_x = 2, local_size_y = 1, local_size_z = 1) in;

// Declare main program function which is executed once
// glDispatchCompute is called from the application.
void main()
{
	// Grab the current global position for this thread
	int index = int(gl_GlobalInvocationID);

	// Pass the color right through.
	outBuffer.data[index].color = outBuffer.data[index].color;

	// Take the position, multiply it by 1.001, then pass it through.
	outBuffer.data[index].position = outBuffer.data[index].position * 1.001; // Will slowly grow the square
}