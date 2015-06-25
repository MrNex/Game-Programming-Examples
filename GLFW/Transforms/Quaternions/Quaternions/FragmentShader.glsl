/*
Title: Quaternions
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

Description:
Builds upon the "Improved3D" project by introducing Quaternions. Euler angles 
are basically just 3 rotations stored about the X, Y, and Z axes. Quaternions 
are faster than Euler angles, so use them! Euler angles are more intuitive, so 
use those for user interface. Quaternions are a set of 4 numbers [x, y, z, w] 
that represet rotations as such:

x = RotationAxis.x * sin(RotationAngle / 2)
y = RotationAxis.y * sin(RotationAngle / 2)
z = RotationAxis.z * sin(RotationAngle / 2)
w = cos(RotationAngle / 2)

glm::quat q; // Creates an identity quaternion
q = glm::quat(w, x, y, z); // Specify the direct components, you almost never do this
q = glm::quat(vec3(90, 45, 0)) convert from Euler angles (in radians) to Quaternion
q = glm::angleAxis(AngleInRadians, RotationAxis); // Convert from axis-angle

mat4 RotationMatrix = glm::toMat4(q); // Convert from glm::quat to glm::mat4.
*/

#version 400 core // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

layout(location = 0) out vec4 out_color; // Establishes the variable we will pass out of this shader.

in vec4 color;	// Take in a vec4 for color
 
void main(void)
{
	out_color = color; // Set our out_color equal to our in color, basically making this a pass-through shader.
}