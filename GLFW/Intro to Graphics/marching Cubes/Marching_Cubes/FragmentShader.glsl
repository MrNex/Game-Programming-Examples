/*
Title: Marching Cubes
File Name: FragmentShader.glsl
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
Marching cubes is a algorithm to construct isosurfaces for oddly shaped
objects, like fluids. It can be used to attain higher detail at lower memory cost.

We divide the space into symmetric cubes. these cubes can be subdivided into smaller cubes 
for better resolution. This structure resembles that of a oct-tree. 
For each vertex of every cube, we check if that vertex constitues a part of the surface.
since each each vertex can either be a part or not, we have a combination of 2^8 different 
scenarios. But these can be reduced to 15 unique cases which  can be transformed to reproduce 
the other formations.

These 8 values can be stored in 1 byte using each bit to represent a corner. In this example,
we are using bool variables instead, for ease of understanding.  

References:
https://www.jvrb.org/past-issues/5.2008/1309
https://www.youtube.com/watch?v=LfttaAepYJ8 - best visual demo for understanding
https://en.wikipedia.org/wiki/Marching_cubes
*/


#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}