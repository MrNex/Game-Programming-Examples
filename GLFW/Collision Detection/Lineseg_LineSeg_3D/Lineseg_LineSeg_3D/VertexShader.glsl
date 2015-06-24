/*
Title: Line Segment - Line Segment 3D collision Detection
File Name: VertexShader.glsl
Copyright © 2015
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
This is collision test between two lien segments in 3D. We cannot use the 
same approach here as we did in 2D. Because in 2D, if 2 lines are not parallel,
then they are deifnitly intersecting at some point. That is not true in 3D.

Furthur complication in 3D are that, even if the the lines are intersecting
in whe nmathematically derived by hand, we won't get the same result because of 
floating point errors. Thus we use a threshold. We find thetwo closest points
on the lines and find the distance between them, if the distance is less than 
the threshold, then collision is detected, else not.

Use Mouse to move in x-y plane, and "w and s" to move in z axis.
Use "spacebar" to change between the two lines.
Use "left shift" to change between the endpoitns of the chosen line.
Use "left Ctrl" to switch between viewpoints.

The thin lines indicate the 3 axis. they start at 0 and end at infinity.

References:
Real time collision Detection by Ericson
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
	color = vec4(blue,0.0f);	// Pass the color through
	gl_Position = MVP * vec4(in_position, 1.0); //w is 1.0, also notice cast to a vec4
}
