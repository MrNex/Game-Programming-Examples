/*
Title: GeometryShader
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
 
in vec2 pos;	// Our vec2 for position (x, y)
in vec3 color;	// Our vec3 for color (r, g, b)
in float sides;	// Our float for the number of sides

out vec3 vColor;	// Pass out the color
out float vSides;	// Pass out the number of sides

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0); // Pass the position through, giving 0.0 for the z coordinate and 1.0 for the w coordinate
    vColor = color;	// Pass through color
    vSides = sides; // Pass through num sides
}