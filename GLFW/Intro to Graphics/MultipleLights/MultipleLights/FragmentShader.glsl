/*
Title: Multiple Lights
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
Combines the Directional Light, Point Light, and Spot Light projects together to allow several lights to be placed 
in a scene at once. This introduces using a uniform struct array of lights and a uniform integer that defines the number 
of lights. Directional lights are identified as the position variable w = 0. Point lights are differentiated from Spot 
lights by setting the cone angle to 180 or higher. Realistically, this should be done with Uniform Buffers.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

#define MAX_LIGHTS 10

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;
in vec3 normal;
in vec3 pixPosition;

uniform int numLights;

uniform struct Light {
	vec4 position;
	vec4 diffuseColor;
	vec4 coneDirection;
	float coneAngle;
	float attenuation;
	float ambientCoefficient;
	float padding;
} lights[MAX_LIGHTS];

vec3 ApplyLight(int index)
{
	// Is this a directional light? Only if w == 0, so...
	//float directionalLight = abs(sign(lights[i].position.w)); // equals 0 if true, 1 if false
	
	vec3 surfaceToLight;
	float attenuation = 1.0;

	// Is this a directional light?
	if(lights[index].position.w == 0.0)
	{
		// The position is the direction since this is a directional light.
		// Flip the z position because it seems to be backwards.
		surfaceToLight = -normalize(vec3(lights[index].position.x, lights[index].position.y, -lights[index].position.z));
	}
	else // This is either a point light or a cone light
	{
		// Get vector from pixel to light source.
		// Flip the z position because it seems to be backwards.
		surfaceToLight = vec3(lights[index].position.x, lights[index].position.y, -lights[index].position.z) - pixPosition;
	
		// Get the distance to the light source.
		float distanceToLight = length(surfaceToLight);

		// Normalize our surfaceToLight variable.
		surfaceToLight = normalize(surfaceToLight);

		// Calculate attenuation.
		attenuation = 1.0 / (1.0 + lights[index].attenuation * pow(distanceToLight, 2));

		// Check to see if this is inside or outside of the light cone. We will use a replacement for an if statement here, to prevent overhead.
		// Start by receiving the angle between the center of the cone and the ray of light.
		// We also flip the z position here, as it too seems to be backwards.
		float lightToSurfaceAngle = degrees(acos(dot(-surfaceToLight, normalize(vec3(lights[index].coneDirection.x, lights[index].coneDirection.y, -lights[index].coneDirection.z)))));

		// if (lightToSurfaceAngle > coneAngle) then attenuation = 0.
		attenuation = attenuation * (1.0 - max(sign(lightToSurfaceAngle - lights[index].coneAngle), 0.0));
	}	

	// Calculate ambient and diffuse lighting.
	vec3 ambient = lights[index].ambientCoefficient * color.rgb * lights[index].diffuseColor.rgb;
	
	vec3 diffuse = max(0.0, dot(normal, surfaceToLight)) * color.rgb *  lights[index].diffuseColor.rgb;
	
	// Combine the colors together to create your linearColor.
	return ambient + attenuation * diffuse;
}
 
void main(void)
{
	// In this example we are just hardcoding these values. Realistically, you'd want to pass these in so that you can dynamically change the direction and diffuse color.

	vec3 linearColor = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < numLights; i++)
	{
		linearColor += ApplyLight(i);
	}

	// Gamma correction variable.
	vec3 gammaCorrection = vec3(1.0/2.2);

	// Perform gamma correction and send out the color.
	out_color = vec4(pow(linearColor, gammaCorrection), color.a);
}