/*
Title: Spot Light
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
Takes the Standard3D project from Basic Shape Rendering and adds in a spot light. This involves adding a normal to each 
vertex (and thus, binding the normal and passing it into the Vertex Shader), modifying the normal by the world/model/transformation 
matrix, then passing it into the Fragment Shader and performing the calculations necessary. The diffuseColor, lightPosition, 
ambientCoefficient, attenuation, coneAngle and coneDirection values are hard-coded into the Fragment Shader, but could easily be 
extracted as variables passed in so that they could be dynamically modified. This project introduces the concepts of ambient 
lighting, attenuation, and gamma correction.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;
in vec3 normal;
in vec3 pixPosition;
 
void main(void)
{
	// In this example we are just hardcoding these values. Realistically, you'd want to pass these in so that you can dynamically change the direction and diffuse color.

	// This is the "color" of the light. Our light here is emitting a white light, but it could be anything. Feel free to experiment with this.
	vec4 diffuseColor = vec4(1.0, 1.0, 1.0, 1.0);

	//vec3 lightPosition = vec3(-1.0, 1.0, 2.5); // This means that the light is located to the left, up, and toward the camera.
	vec3 lightPosition = vec3(0.0, 0.0, 2.5); // This means that the light is located toward the camera, and centered.

	// An arbitrary value that will affect the change in light as the distance between the light source varies.
	float lightAttenuation = 0.05;

	// A value that determines how much ambient lighting exists in the scene.
	float ambientCoefficient = 0.025;

	// The angle between the center and the side of the cone, in degrees.
	float coneAngle = 5.0;

	// THe direction from the position of the light through the center of the cone.
	vec3 coneDirection = vec3(0.0, 0.0, -1.0);

	// The above values could all be passed in a uniform struct.

	// Get vector from pixel to light source.
	vec3 surfaceToLight = lightPosition - pixPosition;
	
	// Get the distance to the light source.
	float distanceToLight = length(surfaceToLight);

	// Normalize our surfaceToLight variable.
	surfaceToLight = normalize(surfaceToLight);

	// Calculate attenuation.
	float attenuation = 1.0 / (1.0 + lightAttenuation * pow(distanceToLight, 2));

	// Check to see if this is inside or outside of the light cone. We will use a replacement for an if statement here, to prevent overhead.
	// Start by receiving the angle between the center of the cone and the ray of light.
	float lightToSurfaceAngle = degrees(acos(dot(-surfaceToLight, normalize(coneDirection))));

	// if (lightToSurfaceAngle > coneAngle) then attenuation = 0.
	attenuation = attenuation * (1.0 - max(sign(lightToSurfaceAngle - coneAngle), 0.0));

	// Calculate ambient and diffuse lighting.
	vec3 ambient = ambientCoefficient * color.rgb * diffuseColor.rgb;
	vec3 diffuse = max(0.0, dot(normal, surfaceToLight)) * color.rgb * diffuseColor.rgb;

	// Combine the colors together.
	vec3 linearColor = ambient + attenuation * diffuse;

	// Gamma correction variable.
	vec3 gammaCorrection = vec3(1.0/2.2);

	// Perform gamma correction and send out the color.
	out_color = vec4(pow(linearColor, gammaCorrection), color.a);
}