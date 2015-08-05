/*
Title: Finite Element Method (1D)
File Name: Matrix.h
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

#ifndef MATRIX_H
#define MATRIX_H

#include "Vector.h"
#include <stdint.h>

#define Matrix_INIT_ON_STACK( mat, numRow, numCol) \
	mat.numRows = numRow; \
	mat.numColumns = numCol; \
	float comp##mat[numRow * numCol] = { 0 }; \
	mat.components = comp##mat; \
	if (numRow == numCol) Matrix_ToIdentity(&mat);

typedef struct Matrix
{
	uint16_t numRows;
	uint16_t numColumns;

	float* components;
}Matrix;

///
//Allocates memory for a new matrix
//
//Parameters:
//	numRows: The number of rows in the matrix
//	numCols: The number of columns in the matrix
//
//Returns:
//	Pointer to new matrix
Matrix* Matrix_Allocate();

///
//Initializes a matrices components
//
//Parameters:
//	mat: Matrix to initialize
void Matrix_Initialize(Matrix* mat, const uint16_t numRows, const uint16_t numCols);


///
//Frees a matrix's resources
//
//Parameters:
//	mat: Matrix to free
void Matrix_Free(Matrix* mat);

///
//Copies a matrix array
//
//Parameters:
//	dest: The destination of the copy
//	source: The matrix to copy
//	numRows: The number of rows in the matrix being copied
//	numCols: The number of columns in the matrix being copied
void Matrix_CopyArray(float* dest, const float* source, const uint16_t numRows, const uint16_t numCols);
//Checks for errors then calls matrix_CopyArray
void Matrix_Copy(Matrix* dest, const Matrix* src);

///
//Transforms a matrix into an nXn Identity matrix
//
//Parameters:
//	mat: The matrix to transform
//	dim: The dimension of the desired identity matrix
void Matrix_ToIdentityArray(float* mat, const uint16_t dim);
//Calls Matrix_ToIdentityArray after error checking
void Matrix_ToIdentity(Matrix* mat);

///
//Indexes an array representing amatrix
//
//Parameters:
//	mat: The matrix to index
//	row: the row of the element we wish to index
//	col: The column of the element we wish to index
//	numRows: The numberof rows in the matrix
//	numCols: The number of columns in the matrix
float* Matrix_IndexArray(float* mat, const uint16_t row, const uint16_t col, const uint16_t numCols);
//Const correct indexing
float Matrix_GetIndexArray(const float* mat, const uint16_t row, const uint16_t col, const uint16_t numCols);
//Performs error checking then calls Matrix_IndexArray
float* Matrix_Index(Matrix* mat, const uint16_t row, const uint16_t col);
//Const correct error checking indexing! Wahooo!
float Matrix_GetIndex(const Matrix* mat, const uint16_t row, const uint16_t col);

///
//Gets the minor matrix corresponding to the specified index
//
//Parameters:
//	dest: The destination matrix
//	mat: The matrix to extract the minor from
//	row: The row of the index to get the corresponding minor of
//	col: The column of the index to get the corresponding minor of
//	numRows: The number of rows in the matrix
//	numColumns: The number of columns in the matrix
void Matrix_GetMinorArray(float* dest, const float* mat, const uint16_t row, const uint16_t col, const uint16_t numRows, const uint16_t numColumns);
//Checks for errors then calls CMatrix_GetMinorArray
void Matrix_GetMinor(Matrix* dest, const Matrix* mat, const uint16_t row, const uint16_t col);

///
//Gets an array representing a row Vector of this matrix
//
//Parameters:
//	destination: The destination of the row Vector array
//	mat: The matrix to get a row from
//	desiredRow: The row to get
//	numColumns: The number of columns in the source matrix
void Matrix_GetRowVectorArray(float* destination, const float* mat, const uint16_t desiredRow, const uint16_t numColumns);
//Checks for errors then calls Matrix_GetRowVectorArray
void Matrix_GetRowVector(Vector* destination, const Matrix* mat, const uint16_t desiredRow);

///
//Gets an array representing a column of this Vector
//
//Parameters:
//	destination: The destination of the column Vector array
//	mat: The matrix to get a column from
//	desiredCol: The column to get from the matrix
//	numRows: The number of rows in the matrix
//	numColumns: The number of columns in the matrix
void Matrix_GetColumnVectorArray(float* destination, const float* mat, const uint16_t desiredCol, const uint16_t numRows, const uint16_t numColumns); 
//Checks for errors then calls Matrix_GetColumnVectorArray
void Matrix_GetColumnVector(Vector* destination, const Matrix* mat, const uint16_t desiredCol);

///
//Slices a row of a matrix storing the contents in a Vector
//
//Parameters:
//	destination: The destination array to hold the row contents
//	mat: The matrix to slice
//	desiredRow: The row of the matrix to slice
//	sliceStart: The index to begin slicing the row (inclusive)
//	sliceRange: The amount of indices to slice
//	numColumns:	The number of columns in the matrix
void Matrix_SliceRowArray(float* destination, const float* mat, const uint16_t desiredRow, const uint16_t sliceStart, const uint16_t sliceRange, const uint16_t numColumns);
//Checks for errors then calls Matrix_SliceRowArray
void Matrix_SliceRow(Vector* destination, const Matrix* mat, const uint16_t desiredRow, const uint16_t sliceStart, const uint16_t sliceRange);

///
//Slices a row of a matrix storing the contents in a Vector
//
//Parameters:
//	destination: The destination array to hold the row contents
//	mat: The matrix to slice
//	desiredColumn: The row of the matrix to slice
//	sliceStart: The index to begin slicing the row (inclusive)
//	sliceRange: The amount of indices to slice
//	numColumns:	The number of columns in the matrix
void Matrix_SliceColumnArray(float* destination, const float* mat, const uint16_t desiredColumn, const uint16_t sliceStart, const uint16_t sliceRange, const uint16_t numColumns);
//Checks for errors then calls Matrix_SliceColumnArray
void Matrix_SliceColumn(Vector* destination, const Matrix* mat, const uint16_t desiredRow, const uint16_t sliceStart, const uint16_t sliceRange); 

///
//Scales a matrix by a scalar
//
//Parameters:
//	matrix: A pointer to the array of floats representing the matrix to scale
//	numRows: The number of rows in the matrix
//	numCols: The number of columns in the matrix
//	scalarValue: The value by which to scale the matrix
void Matrix_ScaleArray(float* matrix, const uint16_t numRows, const uint16_t numColumns, const float scalarValue);
//Calls Matrix_ScaleArray
void Matrix_Scale(Matrix* matrix, const float scalarValue);

///
//Calculates the determinate of a matrix in array form.
//
//Parameters:
//	mat: The matrix to calculate the determinate of
//	numColumns: The number of columns in the matrix
//	numRows: The number of rows in the matrix
float Matrix_GetDeterminateArray(const float* mat, const uint16_t numRows, const uint16_t numColumns);
//Checks for errors then calls CMatrix_GetDeterminateArray
float Matrix_GetDeterminate(const Matrix* mat);

///
//Trasposes a matrix in array form
//Matrix must be NxN
//
//Parameters:
//	mat: The matrix to transpose in array form
//	numRows: The number of rows in the matrix
//	numColumns: the number of columns in the matrix
void Matrix_TransposeArray(float* mat, const uint16_t numRows, const uint16_t numColumns);
//Checks for errors then calls Matrix_TransposeArray
void Matrix_Transpose(Matrix* mat);

///
//Finds the transpose of a matrix array and stores it in a given array
//
//Parameters:
//	dest: A pointer to an array of floats as the destinaton of the transpose matrix
//	matrix: A pointer to an array of floats representing the matrix to transpose
//	numRows: the number of rows in the matrix
//	numColumns: The number of columns in the matrix
void Matrix_GetTransposeArray(float* dest,const float* matrix, const uint16_t numRows, const uint16_t numColumns);
//Checks for errors, then calls Matrix_GetTransposeArray
void Matrix_GetTranspose(Matrix* dest, Matrix* src);

///
//Calculates the inverse of a matrix in array form.
//
//Parameters:
//	dest: A pointer to an array of floats to store the inverse of the components
//	matrix: A pointer to an array of floats containing the components of the matrix to invert
//	numRows: The number of rows in the matrix being inverted
//	numCols: The number of columns in the matrix being inverted
void Matrix_GetInverseArray(float* dest, const float* matrix, const uint16_t numRows, const uint16_t numCols);
//Checks for errors, then calls Matrix_GetInverseArray
void Matrix_GetInverse(Matrix* dest, const Matrix* matrix);

///
//Multiplies a matrix onto another, transforming the latter.
//
//Parameters:
//	LHSMatrix: Left hand side matrix (Will not be altered)
//	RHSMatrix: Right hand side matrix (Destination of product)
//	LHSNumRows: Number of rows in the left hand side matrix (Must be equal to the number of columns in the right hand side matrix)
//	LHSNumCols: Number of columns in the left hand side matrix (Must be equal to the number of rows in the right hand side matrix)
void Matrix_TransformMatrixArray(const float* LHSMatrix, float* RHSMatrix, const uint16_t LHSNumRows, const uint16_t LHSNumCols, const uint16_t RHSNumCols);
//Checks for errors then calls Matrix_TransformMatrixArray
void Matrix_TransformMatrix(const Matrix* LHSMatrix, Matrix* RHSMatrix);

///
//Gets the product of a matrix acting upon another matrix
//
//Parameters:
//	destMatrix: The destination of the product matrix
//	LHSMatrix: The left hand side matrix
//	RHSMatrix: The right hand side matrix
//	LHSNumRows: The number of rows in the left hand side matrix
//	LHSNumCols: The number of columns in the left hand side matrix
void Matrix_GetProductMatrixArray(float* destMatrix, const float* LHSMatrix, const float* RHSMatrix, const uint16_t LHSNumRows, const uint16_t LHSNumCols, const uint16_t RHSNumCols);
//Checks for errors then calls GetProductMatrixArray
void Matrix_GetProductMatrix(Matrix* destMatrix, const Matrix* LHSMatrix, const Matrix* RHSMatrix);

///
//Multiplies a matrix onto a vecor, transforming the Vector
//
//Parameters:
//	LHSMatrix: The left hand side operand, the matrix
//	RHSVector: The right had side operand and the destination, the Vector
//	LHSNumRows: The number of rows in the Left Hand Side matrix
//	LHSNumCols: The number of rows in the Right Hand Side matrix
void Matrix_TransformVectorArray(const float* LHSMatrix, float* RHSVector, const uint16_t LHSNumRows, const uint16_t LHSNumCols);
//Checks for errors then calls Matrix_TransformVectorArray
void Matrix_TransformVector(const Matrix* LHSMatrix, Vector* RHSVector);


///
//Gets the product of a matrix acting upon a Vector
//
//Parameters:
//	destVector: The destination of the product Vector
//	LHSMatrix: The left hand side matrix operand
//	RHSVector: The right hand side Vector operand
//	LHSNumRows: The number of rows in the LHS Matrix operand
//	LHSNumCols: The number of columns in the LHS MatrixOperand
void Matrix_GetProductVectorArray(float* destVector, const float* LHSMatrix, const float* RHSVector, const uint16_t LHSNumRows, const uint16_t LHSNumCols);
//Checks for errors then calls Matrix_GetPRoductVectorArray
void Matrix_GetProductVector(Vector* destVector, const Matrix* LHSMatrix, const Vector* RHSVector);



///
//Prints out a matrix
//
//Parameters:
//	mat: The Matrix to print
//	numRows: The number of rows in the matrix
//	numCols: The number of columns in the matrix
void Matrix_PrintArray(const float* mat, const uint16_t numRows, const uint16_t numCols);
//Calls Matrix_PrintArray
void Matrix_Print(const Matrix* mat);


#endif
