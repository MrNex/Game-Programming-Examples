/*
Title: Rodrigues' Rotation Formula
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
This is a demonstration of using Rodrigues' Rotation Formula to generate
a rotation matrix to rotate an object around an arbitrary axis by an arbitrary angle.

When started, the user can input 3 numbers representing the X, Y, and Z components
of the axis which the rotation matrix should rotate around. The user will
also need to specify an angle in radians which is the magnitude of rotation.
After this, the rotation matrix will be calculated and printed out.

At any point the user can type "quit" to exit the program.

References:
NGen by Nicholas Gallagher
*/
#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <iomanip>
#include <string>
#include "glm/glm.hpp";


int main(int argc, char* argv[])
{
	float angle;
	std::string input;
	while(true)
	{
		std::cout << "Enter a number to serve as the X component of the axis of rotation \nor type \"quit\" to exit the program:\n";
		std::cin >> input;

		if(input.compare(0,4,"quit") == 0)
			break;

		glm::vec3 inputVector;

		inputVector.x = std::stof(input);

		std::cout << "Enter a number to serve as the Y component of the axis of rotation\nor type \"quit\" to exit the program:\n";
		std::cin >> input;
		if(input.compare(0,4,"quit") == 0)
			break;
		inputVector.y = std::stof(input);

		std::cout << "Enter a number to serve as the Z component of the axis of rotation\nor type \"quit\" to exit the program:\n";
		std::cin >> input;
		if(input.compare(0,4,"quit") == 0)
			break;
		inputVector.z = std::stof(input);

		inputVector = glm::normalize(inputVector);

		std::cout << "Enter a number to serve as the angle of rotation (In radians)\nor type \"quit\" to exit the program:\n";
		std::cin >> input;
		if(input.compare(0,4,"quit") == 0)
			break;
		angle = std::stof(input);

		//Now that we have an vector chosen by the user and an angle of rotation
		//We want to generate the rotation matrix which will rotate a vector around the
		//inputVector by the given number of radians
		glm::mat3 rotation;

		//Our rotation matrix will be the sum of 3 matrices
		glm::mat3 m1, m2, m3;

		//The first is the identity matrix
		m1 = glm::mat3(1.0f);

		//The second is a scale of the Skew Symmetric Matrix formed by the inputVector
		//The skew symmetric matrix can be given as
		//	0	-z	y
		//	z	0	-x
		//	-y	x	0
		//
		//Where x, y, and z refer to the x, y, and z components of the input vector.
		m2[0][0] = 0.0f;			m2[0][1] = -inputVector.z;	m2[0][2] = inputVector.y;
		m2[1][0] = inputVector.z;	m2[1][1] = 0.0f;			m2[1][2] = -inputVector.x;
		m2[2][0] = -inputVector.y;	m2[2][1] = inputVector.x;	m2[2][2] = 0.0f;

		//The third matrix is a scale of the squared skew symmetrix matrix formed by the input vector
		m3 = m2 * m2;

		//The second matrix is scaled by the sin of the angle of rotation
		m2 *= sinf(angle);

		//The third matrix is scaled by 1 minus the cosine of the angle of rotation
		m3 *= 1.0f - cos(angle);

		//The final rotation matrix is the sum of the three
		rotation = m1 + m2 + m3;

		std::cout << "\nThe rotation matrix needed to rotate an object " << angle << " radians \naround the axis < " << inputVector.x << ", " << inputVector.y << ", " << inputVector.z << " >\nis:\n";
		std::cout << std::fixed << std::setprecision(6) << rotation[0][0] << "\t" << std::setprecision(6) << rotation[0][1] << "\t" << std::setprecision(6) << rotation[0][2] << std::endl;
		std::cout << std::fixed << std::setprecision(6) << rotation[1][0] << "\t" << std::setprecision(6) << rotation[1][1] << "\t" << std::setprecision(6) << rotation[1][2] << std::endl;
		std::cout << std::fixed << std::setprecision(6) << rotation[2][0] << "\t" << std::setprecision(6) << rotation[2][1] << "\t" << std::setprecision(6) << rotation[2][2] << std::endl << std::endl;
	};

}