/*
Title: Circle-OBB 2D collision Detection
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
This is a collision test between a Circle and a oriented bounding box in 2D.
In this example, we compute the position of Circle with respect to the 
bounding Box. We do this by converting the position of the circle to from the 
world coordinate system to the box's model coordinate system. After doing this, we can use the simple Circle-AABB test to check for collisions . Use "w,a,s,d to
control the circle. The circle changes color upon detecting collision.

References:
2D Game Collision Detection by Thomas Schwarzl,
AABB-2D by Brockton Roth
*/


#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code
 
layout(location = 0) in vec3 in_position;	// Get in a vec3 for position
layout(location = 1) in vec4 in_color;		// Get in a vec4 for color

out vec4 color; // Our vec4 color variable containing r, g, b, a

uniform vec3 colorAndPos; // Our uniform MVP matrix to modify our position values

void main(void)
{
	color = in_color + vec4(0.0f,0.0f,colorAndPos.z,0.0f);	// Pass the color through
	gl_Position = vec4(in_position.xy + colorAndPos.xy,0.0f, 1.0); //w is 1.0, also notice cast to a vec4
}