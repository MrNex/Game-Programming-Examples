/*
Title: Matrix and Vector Arithmetic
File Name: main.cpp
Copyright © 2015
Original authors: Nicholas Gallagher
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the Q public license.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

Description:
This is a demonstration on how to program various vector
and matrix operations. When run a few operations and their results will
be printed into a console window. The operations are performed on arrays of floats. 
This program was written in a C99 compatible subset of C++.

All operations have been programmed to be scalable to any dimension.
Operations includes the Vector operations Addition, subtraction, dot product, cross product,
projection, and magnitude aswell as the Matrix operations multiplication, inversion, determinant calculation,
minor calculation, row slicing, column slicing, and indexing.

The user must press CTRL+f5 to fun the solution and have the window say open.
Alternatively the user can click Debug->Run without debugging.

References:
NGen by Nicholas Gallagher
*/
#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include "Matrix.h"


int main(int argc, char* argv[])
{
	//Create 3 mutually perpendicular vectors
	float X[] = {1.0f, 0.0f, 0.0f};
	float Y[] = {0.0f, 1.0f, 0.0f};
	float Z[] = {0.0f, 0.0f, 1.0f};

	//Create a matrix which rotates a vector by 90 degrees around the Z axis
	uint16_t numRows = 3;
	uint16_t numColumns = 3;
	float R[] = 
	{
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};


	//Print the vectors
	printf("Vectors\n--------");
	printf("\nX:\t");
	Vector_PrintTransposeArray(X, 3);
	printf("Y:\t");
	Vector_PrintTransposeArray(Y, 3);
	printf("Z:\t");
	Vector_PrintTransposeArray(Z, 3);

	//Print the matrix
	printf("\nMatrices\n--------");
	printf("\nR:\n");
	Matrix_PrintArray(R, numRows, numColumns);

	//Perform the dot product on the vectors to show they are mutually perpendicular
	float dotProduct;
	dotProduct = Vector_DotProductArray(X, Y, 3);
	printf("\ndot(X, Y) = %f\n", dotProduct);
	dotProduct = Vector_DotProductArray(X, Z, 3);
	printf("dot(X, Z) = %f\n", dotProduct);
	dotProduct = Vector_DotProductArray(Y, Z, 3);
	printf("dot(Y, Z) = %f\n", dotProduct);

	//Take the cross product of the X and Y vectors
	float result[3] = {0.0f};
	float* operands[2] = {X, Y};
	Vector_CrossProductArray(result, 3, operands);
	printf("\ncross(X, Y) = ");
	Vector_PrintTransposeArray(result, 3);

	//Take the sum of the vectors
	Vector_AddArray(result, X, Y, 3);
	Vector_IncrementArray(result, Z, 3);
	printf("\nSum = X + Y + Z = ");
	Vector_PrintTransposeArray(result, 3);

	//Scale the sum by 5
	Vector_ScaleArray(result, 5.0f, 3);
	printf("\n5 * Sum = ");
	Vector_PrintTransposeArray(result, 3);

	//Project the sum onto the X axis
	Vector_ProjectArray(result, X, 3);
	printf("\nProject(Sum, X) = ");
	Vector_PrintTransposeArray(result, 3);


	//Perform Matrix multiplication on the X axis
	Matrix_GetProductVectorArray(result, R, X, numRows, numColumns);
	printf("\nR * X = ");
	Vector_PrintTransposeArray(result, 3);

	//Get the inverse of R
	float Rinverse[9];
	Matrix_GetInverseArray(Rinverse, R, numRows, numColumns);
	printf("\nRinverse:\n");
	Matrix_PrintArray(Rinverse, numRows, numColumns);

	//Take the product of R and Rinverse
	float I[9];
	Matrix_GetProductMatrixArray(I, R, Rinverse, numRows, numColumns, numColumns);
	printf("\nR * Rinverse =\n");
	Matrix_PrintArray(I, numRows, numColumns);

}