/*
Title: Convex Hull - Line Segment
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
This is a demonstration of collision detection between a convex hull and a line segment.
The demo contains a line segment wireframe of a tetrehedron. The line will appear green and the 
tetrahedron will appear blue. When the shapes collide, their colors will change to red and pink, respectively.

You can move the shapes around the X-Y plane with WASD, and along the Z axis with
Left Shift & Left Control. Pressing space will toggle the shape being moved.
You can also rotate the selected shape by holding the left mouse button and dragging the mouse.

This function uses a series of half-space tests to perform collision detection between
a line segment and a convex hull. A convex hull can be described as a set of intersecting planes.
What this algorithm will do is compute the intersection scalar value denoting the point of intersection of the line segment with the plane
in terms of the parametric equation of the line. If this scalar value is between 0 and 1 then that segment must intersect the plane.
Using the dot product we can determine if the line segment at that intersection is entering the hull or exiting the hull. If we keep track
of the largest t at which it enters, and the smallest t at which it exits, we can determine the intersection interval of the line.
if the end of that interval (the smallest t at which it exits) is smaller than the start of that interval (the largest t at which it enters)
there is no collision. However, if this is not the case, we do have a collision!

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
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