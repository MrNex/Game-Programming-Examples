/*
Title: Deferred Shading
File Name: LightFragShader.glsl
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

layout(location = 0) out vec4 Color; // Establishes the variable we will pass out of this shader.

layout (binding = 0) uniform sampler2D PositionTex;
layout (binding = 1) uniform sampler2D NormalTex;
layout (binding = 2) uniform sampler2D ColorTex;
 
in vec2 texCoord;

uniform struct PointLight
{
	vec3 position;
	vec3 Intensity;
	vec3 Ambient;
}pointLight;

vec3 diffuseModel (vec3 pos, vec3 norm, vec3 diff)
{
	vec3 s = normalize(vec3(pointLight.position) - pos);
	float nDotL = max(dot(s, norm),0.0f);
	vec3 diffuse = pointLight.Intensity * diff * nDotL + diff * pointLight.Ambient;
	
	return diffuse;
}

void main(void)
{
	vec3 p = vec3(texture(PositionTex, texCoord));
	vec3 n = vec3(texture(NormalTex, texCoord));
	vec3 d = vec3(texture(ColorTex, texCoord));
	
	Color = vec4(diffuseModel(p,n,d), 1.0f);
}