/*
Title: GeometryShader
File Name: GeometryShader.glsl
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
This project introduces the concept of a geometry shader, which takes in 
primitives and is able to create more primitives from them. It is 2D, 
having a vertex shader that takes in a vec2 for position, a vec3 for color, 
and a float that determines the number of sides. Then, the geometry shader 
takes that data, and for each vertex passed in draws a shape with the number 
of sides given centered on that vertex. The fragment shader takes in a color 
and draws with that color.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(points) in;	// Defines that we will receive primitives of the type POINTS in this geometry shader
layout(line_strip, max_vertices = 64) out; // Defines that we will export primitives of the type LINE_STRIP from this geometry shader. Also states a maximum of 64 vertices will be exported.

in float vSides[];	// Take in the number of sides
in vec3 vColor[];	// Take in the color

out vec3 fColor;	// Export the color

const float PI = 3.1415926;	// Create a const for PI for trig math

void main()
{
    fColor = vColor[0];	// The color is a pass through

	// The loop runs once for every side, and then one more at the end to close the line loop.
	// At the end of every loop iteration, it will create a vertex.
    for (int i = 0; i <= vSides[0]; i++)
	{
        // Angle between each side in radians
        float ang = PI * 2.0 / vSides[0] * i;

        // Offset from center of point (0.3/0.4 to accomodate for aspect ratio since there's no transform)
        vec4 offset = vec4(cos(ang) * 0.3, -sin(ang) * 0.4, 0.0, 0.0);
        
		// The position is equal to the position of the POINT passed in with an offset.
		gl_Position = gl_in[0].gl_Position + offset;

		// Make a vertex at that position.
        EmitVertex();
    }

	// This line tells it to end the primitive, and create a LINE_STRIP with the vertices made with EmitVertex().
    EndPrimitive();
}