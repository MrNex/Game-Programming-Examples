/*
Title: Cloth Simulation using Compute Shaders
File Name: ComputeShader.glsl
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
This program demonstrates the implementation of cloth like surface using
compute shaders. The cloth is considered to be made up of a mesh of springs.
Each point represents a point mass on the cloth which is connected to 4
other point masses (top, bottom, left and right).

Each point mass computes the force exerted on it due to the 4 springs, which
connect the point mass to the neighbouring masses. This computation is done
in GPU, namely the compute shader.

In this example, we allocate the required memory space in GPU buffers. We make
4 buffers: One to read position data from and one to write position data to.
similarly for velocity of each particle. We have two separate buffers to read
and write to avoid data races and read-before-write errors.

We send the data to one of the buffers once, then compute the position and velocity
at the of the frame in the sahder and store it in the output buffers. Then we
bind the output buffer to the GPU ARRAY BUFFER, and tell the GPU "how to" read the data,
i.e. set attribute pointers and enable them.

Then we simply call the drawArrays function of openGL. Notice how we never read
from the buffers on the CPU side of the applicaiton. This is the advantage of using
shaders in this type of situations: We avoid unnecessary transfer of data from CPU to GPU.

References:
OpenGL 4 shading language cookbook by David Wolff
*/

#version 430 // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
//This decides the number of invocations in 1 workgroup.
layout (local_size_x = 10, local_size_y = 10) in;

uniform vec3 Gravity = vec3(0.0f,-10.0f,0.0f);
uniform vec3 externalForce;
uniform float ParticleMass = 0.1f;
uniform float particleInvMass = 1.0f/0.1f;
uniform float SpringK = 2000.0f;
uniform float RestLengthHoriz;
uniform float RestLengthVert;
uniform float RestLengthDiag;
uniform float deltaT = 0.00001f;
uniform float DampingConst = 0.1f;

layout (std430, binding = 0) buffer PosIn { vec4 PositionIn[]; };
layout (std430, binding = 1) buffer PosOut { vec4 PositionOut[]; };
layout (std430, binding = 2) buffer VelIn { vec4 VelocityIn[]; };
layout (std430, binding = 3) buffer VelOut { vec4 VelocityOut[]; };

void main(void)
{
	//Get the number of particles in the cloth
	uvec3 nParticles = gl_NumWorkGroups * gl_WorkGroupSize;
	//Get the ID for the current invocation.
	uint idx = (gl_GlobalInvocationID.y * nParticles.x) + gl_GlobalInvocationID.x;
	
	vec3 p = vec3(PositionIn[idx]).xyz;
	vec3 v = vec3(VelocityIn[idx]).xyz;
	vec3 r;
	
	//compensate for external forces (gravity and wind)
	vec3 force = (Gravity * ParticleMass) + externalForce;
	
	//Horizontal
	if(gl_GlobalInvocationID.x > 0)																//left
	{
		r = PositionIn[idx - 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
	}
	
	if(gl_GlobalInvocationID.x < nParticles.x - 1)												//Right
	{
		r = PositionIn[idx + 1].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
	}
	
	//Vertical including the diagonals															
	if(gl_GlobalInvocationID.y > 0)
	{
		r = PositionIn[idx - nParticles.x].xyz - p;												//Vertically below
		force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
		
		//Uncomment this section if you wish the point mass to be connected to the diagonal elements as well
		//if( gl_GlobalInvocationID.x > 0)														//Bottom left
		//{
		//	r = PositionIn[idx - nParticles.x - 1].xyz - p;
		//	force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
		//}
		//if( gl_GlobalInvocationID.x < nParticles.x - 1)											//Bottom Right
		//{
		//	r = PositionIn[idx - nParticles.x + 1].xyz - p;
		//	force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
		//}
		
	}
	
	if(gl_GlobalInvocationID.y < nParticles.y - 1)												//Vertically above
	{
		r = PositionIn[idx + nParticles.x].xyz - p;
		force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
		
		//Uncomment this section if you wish the point mass to be connected to the diagonal elements as well
		//if( gl_GlobalInvocationID.x > 0)														//Top left	
		//{
		//	r = PositionIn[idx + nParticles.x - 1].xyz - p;
		//	force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
		//}
		//if( gl_GlobalInvocationID.x < nParticles.x - 1)											//top right
		//{
		//	r = PositionIn[idx + nParticles.x + 1].xyz - p;
		//	force += normalize(r) * SpringK * (length(r) - RestLengthHoriz);
		//}
		
	}	

	force += -DampingConst * v;
	
	vec3 a = force * particleInvMass;
	
	//set the data in output buffer.
	PositionOut[idx].xyz = p + (v * deltaT) + (0.5f * a * deltaT * deltaT);	
	VelocityOut[idx].xyz = vec3(v + a*deltaT);
	
	//Pin the top Vertices
	if(gl_GlobalInvocationID.y ==(nParticles.y - 1)
	&&( mod(gl_GlobalInvocationID.x,10.0f) == 0.0f || gl_GlobalInvocationID.x ==(nParticles.x - 1)))
	{
		PositionOut[idx] = PositionIn[idx];
		VelocityOut[idx] = VelocityIn[idx];
	}
}






















