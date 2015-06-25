/*
Title: Sphere - Plane
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
This is a demonstration of collision detection between a sphere and a plane.
The demo contains a wireframe of a sphere and a solid plane. When the objects are not colliding
the plane will appear blue and the sphere green. When the two objects collide the plane will become
pink and the sphere will become yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with Left Shift and Left Control. Lastly,
you can rotate the plane by clicking and dragging the mouse.

This demo detects collisions by by computing the distance between the plane and the center of the sphere.
This is done by taking a vector from the center of the plane to the center of the sphere and computing the dot product (Or scalar projection)
of this vector and the normalized normal vector of the plane. Once this distance is computed, if the distance is less than the radius, we must have a collision.

References:
Base by Srinivasan Thiagarajan
Sphere Collision 3D by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
 uniform mat4 hue;	//Global hue control

void main(void)
{
	out_color = hue * color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}