/*
Title: AABB-Line 3D collision Detection
File Name: VertexShader.glsl
Copyright � 2015
Original authors: Srinivasan Thiagarajan
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
This is an example to detect the intersection of a line and AABB in 3D.
We extend the concept of finding the point of intersection between a plane and a line
to detect where the line enter and exits the box. The box has 6 planes : 3 parallel to the other 3. 
We take these sets of parallel planes and find where the line intersects them both. 
The nearest one acts as entry point and the farthest acts as exit.
We combine the results of all 3 sides, and find the FARTHEST POINT OF ENTRY AND CLOSEST POINT OF EXIT.
If the entry point is farther than the exit, then the line segment exits that set of planes before 
entering the other set of planes. Thus no collision is detected. 

Use Mouse to move in x-y plane, and "w and s" to move in z axis.

References:
(Pg: 179) RealTime collision detection by Christer Ericson
AABB-2D by Brockton Roth
*/



#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform mat4 MVP; // Our uniform MVP matrix to modify our position values

uniform	vec3 blue;

void main(void)
{
	color = in_color + vec4(blue,0.0f);	// Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}
