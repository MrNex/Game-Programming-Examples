/*
Title: Directional Lighting
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

References:
http://www.tomdalling.com/blog/modern-opengl/08-even-more-lighting-directional-lights-spotlights-multiple-lights/

Description:
Takes the Standard3D project from Basic Shape Rendering and adds in directional lighting. This involves adding a normal 
to each vertex (and thus, binding the normal and passing it into the Vertex Shader), modifying the normal by the 
world/model/transformation matrix, then passing it into the Fragment Shader and performing the calculations necessary. 
The diffuseColor and lightDirection are hard-coded into the Fragment Shader, but could easily be extracted as variables 
passed in so that they could be dynamically modified.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;
in vec3 normal;
 
void main(void)
{
	// In this example we are just hardcoding these values. Realistically, you'd want to pass these in so that you can dynamically change the direction and 
	// diffuse color.
	vec4 diffuseColor = vec4(1.0, 1.0, 1.0, 1.0); // This is the "color" of the light. Our light here is emitting a white light, but it could be anything. Feel free 
	// to experiment with this.
	vec3 lightDirection = vec3(-0.2, -0.2, -1.0); // This means the light is pointing slightly down, slightly left and mostly forward.
	// Note that we reverse lightDirection for proper calculation.

	// Take the dot product of the normal and the reverse lightDirection, giving you a value of how intense the light should be at that point.
	// Then clamp that value between 0 and 1.
	float lightIntensity = clamp(dot(normal, -lightDirection), 0.0, 1.0);

	// Multiply the diffuseColor (white, in this case) by the intensity of the light.
	// The clamping here is normally supposed to be from 0.0 to 1.0, but the 0.1 prevents the light from going completely to zero and resulting in black.
	// Consider it ambient lighting.
	out_color = clamp(diffuseColor * lightIntensity, 0.1, 1.0);

	// Now we have the value for our light, but we need to add the actual color of the vertex in. (Otherwise we'd only be rendering the color of the light.)
	out_color = out_color * color;
}