/*
Title: Icosphere
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
http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

Description:
Contains code for generating a 3D Icosahedron, and then refining the edges down to create 
an Icosphere. This is a manner of generating a spherical-like object. By default, the number 
of revisions on the icosphere is 5, which can create a pretty mesmerizing effect (because I 
also have it randomizing the colors of each vertex and spinning around constantly). You can 
also choose to lower or increase the number of revisions.

WARNING: Performance will drop painfully once you reach the 7-9 revisions range. I haven't been 
able to push it past 9 revisions. Also note that this is incredibly inefficient in terms of the 
way it generates the points, so there may be a long startup time for a high number of revisions. 
Feel free to optimize it and send it back to me and I'll upload the better version!
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}