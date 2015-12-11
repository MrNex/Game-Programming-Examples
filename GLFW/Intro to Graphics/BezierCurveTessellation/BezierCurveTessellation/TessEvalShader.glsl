/*
Title: Bezier curve tessellation
File Name: TessEvalShader.glsl
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
This program demonstrates the implementation of bezier curve with the tessellation
being computed in the shader. In this program, we send only the control points to
the shader and adjust the tessellation level accordingly.

The tessellation control shader begins by defining the number of vertices in the
output patch: layout (vertices = 4) out;

Note that this is not the same number of vertices that will be produced by the process.
In this case the number of control points are 4, so we pass 4 in one patch.

The vertex sahder simply passes the input data as read from the buffer to the next shader
(tessellation control shader). The TCS sets the tessellation levels by assigning a value to
gl_TessLevlOuter array. The first element defines the number of isolines that will be generated.
In this example that value will be 1. The second value defines the number of line segments per
isoline.

In tessellation evaluation shader, we start by defining the input primitive type using a layout
declaration: layout (isolines) in;
In TCS, we access the uv coordinates using glTessCoord. Then we access the position of the
four control points (all the points in our patch). Using these values we compute the bernstein
polynomials at given uv cordinates and calculate the position.

References:
OpenGL 4 Shading language cookbook (second edition)

*/

#version 430 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(isolines) in;

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

void main(void)
{
	// The Tessellation u coordinate (because we are rendering a line only u is enough for tessellation)
	float u = gl_TessCoord.x;

	//The path vertices (control points)
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;
	vec3 p2 = gl_in[2].gl_Position.xyz;
	vec3 p3 = gl_in[3].gl_Position.xyz;

	float u1 = (1.0f - u);
	float u2 = (u * u);

	// Bernstein Polynomial evaluated at u
	float b3 = u2 * u;
	float b2 = 3.0f * u2 * u1;
	float b1 = 3.0f * u * u1 * u1;
	float b0 = u1 * u1 * u1;

	// Cubic Bezier interpolation

	vec3 p = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;

	gl_Position = MVP * vec4(p, 1.0f);
}