/*
Title: AverageComputeShader
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
Improves on the BasicComputeShader, taking advantage of the parallelization 
and drawing a spinning circle of points (dots, essentially). Each color is 
based off of the vertex ID (retrieved via gl_GlobalInvocationID) and each 
position is based off of a sin, cos, and radius to create the circle (the 
angle is also calculated using the gl_GlobalInvocationID). This allows for 
fast computation. The radius is set via a uniform variable, and the code 
has it growing to a certain point, then shrinking again, and repeating.

WARNING: Requires OpenGL version 4.3 or later for the compute shader to compile.
*/

#version 430 // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

// The uniform paramters which is passed from application for every frame.
uniform float radius;

// A struct defining how vertex data is formatted
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

// A layout describing the local size, which is basically the number of invocations that will occur within each work group. Here we have 8x8, or 64 invocations
// This is hard to explain with comments, if you're interested, Google up on Compute Shaders, Local Size, and Work Groups. Also look into Graphics Cards and how the hardware works.
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
 
// Declare main program function which is executed once
// glDispatchCompute is called from the application.
void main()
{
      // Read current global position for this thread
      ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
 
      // Calculate the global number of threads (size) for this
      uint gWidth = gl_WorkGroupSize.x * gl_NumWorkGroups.x;
      uint gHeight = gl_WorkGroupSize.y * gl_NumWorkGroups.y;
      uint gSize = gWidth * gHeight;
 
      // Since we have 1D array we need to calculate offset.
      uint offset = storePos.y * gWidth + storePos.x;
 
      // Calculate an angle for the current thread
      float alpha = 2.0 * 3.14159265359 * (float(offset) / float(gSize));
 
      // Calculate vertex position based on the already calculate angle
      // and radius, which is given by application
      outBuffer.data[offset].position = vec3(sin(alpha) * radius, cos(alpha) * radius, 0.0);
 
      // Assign color for the vertex
      outBuffer.data[offset].color = vec4(storePos.x / float(gWidth), 0.0, 1.0, 1.0);
}