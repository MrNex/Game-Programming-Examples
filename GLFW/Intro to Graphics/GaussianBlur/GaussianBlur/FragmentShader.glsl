/*
Title: Gaussian Blur
File Name: FragmentShader.glsl
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

#version 430 core 

in vec2 texCoord; 

layout(location = 2) out vec4 out_color; // The layout is 2 because we are binding the COLOR_ATTACHMENT2 to the framebuffer.

layout(binding = 0) uniform sampler2D tex; 

uniform float weights[5];				// contain the gaussian weights used in bluring

subroutine vec4 RenderPassType();
subroutine uniform RenderPassType renderPass;

subroutine (RenderPassType)
vec4 pass1()
{
	//First pass: render the scene as it is.
	return texture(tex, texCoord);
}

subroutine (RenderPassType)
vec4 pass2()
{
	//Second pass: Implement horizontal blur
	vec4 returnColor = vec4(0);

	vec2 pix = vec2(gl_FragCoord.xy);
	
	float total = weights[0];

	returnColor = texture(tex, texCoord) * weights[0];

	for (int i = 1; i < 5; i++)
	{
		returnColor += texture(tex, texCoord + (ivec2( i, 0) / 800.0f))* weights[i];
		returnColor += texture(tex, texCoord + (ivec2(-i, 0) / 800.0f))* weights[i];
	}
	
	return returnColor;
}

subroutine (RenderPassType)
vec4 pass3()
{
	//Second pass: Implement Vertical blur
	vec4 returnColor = vec4(0);

	vec2 pix = vec2(gl_FragCoord.xy);
	
	float total = weights[0];
	
	returnColor = texture(tex, texCoord) * weights[0];

	for (int i = 1; i < 5; i++)
	{
		returnColor += texture(tex, texCoord + (ivec2(0, i) / 800.0f)) * weights[i];
		returnColor += texture(tex, texCoord + (ivec2(0, -i) / 800.0f)) * weights[i];
		total += weights[i] * 2;
	}
	
	return returnColor;
}

void main(void)
{
	// The color of the pixel will be determined by the subroutine selected 
	out_color = renderPass();
}