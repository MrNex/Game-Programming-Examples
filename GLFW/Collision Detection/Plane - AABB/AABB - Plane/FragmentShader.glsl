/*
Title: AABB - Plane
File Name: main.cpp
Copyright © 2015
Original authors: Nicholas Gallagher
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
This is a demonstration of an Axis Aligned Bounding Box and a Plane. 
The demo contains a wireframe of a box and a solid plane. The plane is colored blue
And the box is colored green until the two collide, then the plane will change to the color
pink and the box will change to the color yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD
and along the Z axis with Left Shift & Left Control. You can also rotate the plane by clicking
and dragging the mouse.

This demo detects collisions by making sure all corners of the box lie on the same side of the plane.
We do this by translating the corners of the box into world space and the normal of the plane
into world space, then translating the entire system to to be centered on the origin of the plane.
From here, you are able to determine what side of the plane the corners fall on by observing the dot
product of the corner's position vectors with the plane's normal. If any sign doesn't match another,
there is a collision.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
2D Game Collision Detection by Thomas Schwarzl
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
 uniform mat4 hue;	//Global hue control

void main(void)
{
	out_color = hue * color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}