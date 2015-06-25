/*
Title: Spot Light
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
http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/

Description:
Takes the Standard3D project from Basic Shape Rendering and adds in a spot light. This involves adding a normal to each 
vertex (and thus, binding the normal and passing it into the Vertex Shader), modifying the normal by the world/model/transformation 
matrix, then passing it into the Fragment Shader and performing the calculations necessary. The diffuseColor, lightPosition, 
ambientCoefficient, attenuation, coneAngle and coneDirection values are hard-coded into the Fragment Shader, but could easily be 
extracted as variables passed in so that they could be dynamically modified. This project introduces the concepts of ambient 
lighting, attenuation, and gamma correction.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec3 in_normal;

out vec4 color;
out vec3 normal;
out vec3 pixPosition;

uniform mat4 trans;

void main(void)
{
	color = in_color;
	normal = mat3(trans) * in_normal; // The normal only gets modified by the model matrix, which in this case only consists of the transformation.
	normal = normalize(normal); // Ensure that our normal has a magnitude of 1.0
	// In potential future implementations where you have a Model-View-Projection matrix, you have to be certain that you only modify the normal by the Model matrix.

	pixPosition = vec3(trans * vec4(in_position, 1.0));

	gl_Position = trans * vec4(in_position, 1.0);//w is 1.0, also notice cast to a vec4 // Pass out the vertices to the gl_Position
}