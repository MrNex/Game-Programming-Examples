/*
Title: Finite Element Method (1D)
File Name: Vector.h
Copyright � 2015
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

#ifndef VECTOR_H
#define VECTOR_H
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>


//Initializes a vector on the stack
#define Vector_INIT_ON_STACK( vec , dim ) \
	vec.dimension = dim; \
	float comps##vec[dim] = { 0.0f }; \
	vec.components = comps##vec; 

///
//A Vector consists of an array of components,
//And a dimension, or number of components.
typedef struct Vector
{
	uint16_t  dimension;
	float* components;
}Vector;


//Constants
static const float Vector_ZEROComponents[] = { 0.0f, 0.0f, 0.0f };
static const float Vector_E1Components[] = {1.0f, 0.0f, 0.0f};
static const float Vector_E2Components[] = {0.0f, 1.0f, 0.0f};
static const float Vector_E3Components[] = {0.0f, 0.0f, 1.0f};

extern const Vector Vector_ZERO;

extern const Vector Vector_E1;

extern const Vector Vector_E2;

extern const Vector Vector_E3;


///
// Allocates a Vector
//
//Returns:
//	A pouint16_t er to the newly allocated Vector. 
//	Vector is not initialized yet and has no components.
Vector* Vector_Allocate();

///
//Initializes a Vector
//
//Parameters:
//	vec: The Vector to initialize
void Vector_Initialize(Vector* vec, uint16_t dim);

///
//Frees the memory taken by a Vector
//
//Parameters:
//	vec: The Vector which's resources are being freed
void Vector_Free(Vector* vec);

///
//Copies a Vector to an array
//
//Parameters:
//	dest: The destination of the copy
//	src: the Vector to copy
//	dim: The dimension of the Vector being copied
void Vector_CopyArray(float* dest, const float* src, const uint16_t  dim);
//Checks for errors then calls Vector_CopyArray
void Vector_Copy(Vector* dest, const Vector* src);

///
//Sets all components of the Vector to 0
//
//Parameters:
//	vec: The Vector being zeroed
//	dim: The number of components the Vector has
void Vector_ZeroArray(float* vec, const uint16_t  dim);
//Calls Vector_ZeroArray
void Vecor_Zero(Vector* vec);

///
//Determines the magniude^2 of a vector
//
//Parameters:
//	vec: The vector to find the mag squared of
//	dim: The number of components in the vector
float Vector_GetMagSqFromArray(const float* vec, const uint16_t  dim);
float Vector_GetMagSq(const Vector* vec);

///
//Determines the magnitude of a Vector
//
//Parameters:
//	vec: The Vector to find the magnitude of
//	dim: the number of components in the Vector
float Vector_GetMagFromArray(const float* vec, const uint16_t  dim);
float Vector_GetMag(const Vector* vec);

///
//Normalizes a Vector
//
//Parameters:
//	vec: the components to normalize
//	dim: the dimension of the Vector
void Vector_NormalizeArray(float* vec, const uint16_t  dim);
void Vector_Normalize(Vector* vec);

///
//Gets the dot product of two arrays
//
//Parameters:
//	vec1: Vector being dotted
//	vec2: Other Vector being dotted
//	dim: Dimension of the two Vectors
//
//Returns:
//	Float representing value of the dot product
float Vector_DotProductArray(const float* vec1, const float* vec2, const uint16_t  dim);
//Checks for errors then calls Vector_DotProductArray
//Returns 0 on error
float Vector_DotProduct(const Vector* vec1, const Vector* vec2);

///
//Gets a vector perpendicular to all other given vectors within a certain subspace.
//
//PArameters:
//	dest: The destination of the cross product result
//	dim: The dimension of the resulting vector
//	vectors: an array of dim - 1 vectors of dimension dim
void Vector_CrossProductArray(float* dest, const uint16_t  dim, float** vectors);
///
//Checks for errors then calls CVector_CrossProductArray
//
//Parameters:
//	dest: The destination of cross product result
//	...: dest->dim - 1 vectors of dimension dest->dim
void Vector_CrossProduct(Vector* dest, ...);

///
//Calculates the angle between two equally dimensioned arrays
//
//Parameters:
//	vec1: The first vector to find the angle between
//	vec2: The second vector to find the angle between
//	dim: the dimension of the two vectors
//
//returns:
//	Angle in radians
float Vector_GetAngleArray(const float* vec1, const float* vec2, const uint16_t  dim);
//Checks for errors then calls CVector_GetAngleArray
float Vector_GetAngle(const Vector* vec1, const Vector* vec2);


///
//Increments a Vector by another Vector
//
//Parameters:
//	dest: the Vector getting incremented
//	src: The addend Vector, or the Vector being added to the destination
//	dim: The number of components in the Vectors
void Vector_IncrementArray(float* dest, const float* src, const uint16_t  dim);
void Vector_Increment(Vector* dest, const Vector* src);
void Vector_DecrementArray(float* dest, const float* src, const uint16_t  dim);
void Vector_Decrement(Vector* dest, const Vector* src);

///
//Scales a Vector by a scalar
//
//Parameters:
//	vec: The Vector to be scaled
//	scaleValue: the scale factor
//	dim: the dimension of the Vector being scaled
void Vector_ScaleArray(float* vec, const float scaleValue, const uint16_t  dim);
void Vector_Scale(Vector* vec, const float scaleValue);

///
//Gets the scalar product of a specified Vector and a scalar
//
//Parameters:
//	dest: The address to hold the scaled Vector product
//	src: the initial Vector to be scaled
//	scaleValue: the amount by which to scale the initial Vector
//	dim: The number of components in the initial Vector
void Vector_GetScalarProductFromArray(float* dest, const float* src, const float scaleValue, const uint16_t  dim);
void Vector_GetScalarProduct(Vector* dest, const Vector* src, const float scaleValue);

///
//Adds together two Vectors retrieving the sum
//
//Parameters:
//	dest: the destination of the sum Vector
//	vec1: the first addend Vector
//	vec2: the second addend Vector
//	dim: the dimension of the Vectors being added
void Vector_AddArray(float* dest, const float* vec1, const float* vec2, const uint16_t  dim);
void Vector_Add(Vector* dest, const Vector* vec1, const Vector* vec2);
void Vector_SubtractArray(float* dest, const float* vec1, const float* vec2, const uint16_t  dim);
void Vector_Subtract(Vector* dest, const Vector* vec1, const Vector* vec2);

///
//Projects vec1 onto vec2 and stores the result in the destination
//
//Parameters:
//	dest: the destination of the projected vector
//	vec1: The vector being projected
//	vec2: The vector representing the projection axis
//	dim: the dimension of the vectors
void Vector_GetProjectionArray(float* dest, const float* vec1, const float* vec2, const uint16_t  dim);
void Vector_GetProjection(Vector* dest, const Vector* vec1, const Vector* vec2);

///
//Projects vec1 onto vec2 storing the result in vec1
//
//Parameters:
//	vec1: The vector being projected
//	vec2: The vector representing the projection axis
//	dim: The dimension of the vectors
void Vector_ProjectArray(float* vec1, const float* vec2, const uint16_t  dim);
void Vector_Project(Vector* vec1, const Vector* vec2);

///
//Prints out the contents of a Vector
//
//Parameters:
//	vec: the Vector to print
//	dim: the number of components in the Vector
void Vector_PrintTransposeArray(const float* vec, const uint16_t  dim);
void Vector_PrintTranspose(const Vector* vec);
void Vector_PrintArray(const float* vec, const uint16_t  dim);
void Vector_Print(const Vector* vec);

#endif