/*
Title: Sphere-Triangle 3D collision Detection
File Name: FragmentSahder.glsl
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
This is an example to detect the intersection of a 2D triangle and a 3D sphere in 3 Dimensions.
In this example, we find the closest point on the triangle to the center of the sphere, 
and check if that point lies inside the sphere, If it intersects, then the traingle intersects with the 
sphere else it does not.

To get the closest point ,we figure out on which voronoi region the center of the sphere lies on. Then we
convert that opint into baricenteric coordinate, and calculate the colest point on the triangle to the center 
of the sphere. Then we check the distance between the sphere and that point.

All this is done mathematically on paper, only the main formula is encoded here. For more detail, refer to
RealTime Collision Detection by Ericson

References:
Real time collision Detection by Ericson
Nicholas Gallagher
AABB-2D by Brockton Roth
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}