/*
Title: Line Segment and Circle 2D intersection
File Name: Model.cpp
Copyright © 2015
Author: Srinivasan Thiagarajan
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
This is an example to detect the intersection of a line segment and circle in 2D.
You can control the two end-points of the line segment, and move them using "w,a,s,d" and "i,j,k,l" respectively.
The line turns blue when an intersection is detected, and turns red when there is no intersection.
The program first checks if either of the end-points lie within the circle (which is stationary), if so, collision is detected.
If neither of the end-points are inside the circle, then it check for the closest point to the circle's center, which also lies on the line,
and checks if that point lies inside the circle. If so, collision is detected else not.
*/

#ifndef _MODEL_CPP
#define _MODEL_CPP

#include "Model.h"

// Creates a new model with a given vertices and indices.
// If no vertices are passed in (numVerts = 0) then it will skip initialization completely.
// If no indices are passed in (numInds = 0) but vertices are, it will set the indices equal to the vertices in order. (So just 0, 1, 2, 3, 4, etc.)
Model::Model(int numVerts, VertexFormat* verts, int numInds, GLuint* inds)
{
	if (numVerts > 0)
	{
		// Allocate space for the size of the vertices array.
		vertices = (VertexFormat*)malloc(sizeof(VertexFormat) * numVerts);

		// Copy the data from the passed in verts to the vertices array.
		memcpy(vertices, verts, sizeof(VertexFormat) * numVerts);

		// Set the numVertices equal to the passed in numVerts.
		numVertices = numVerts;

		if (numInds > 0)
		{
			// Allocate space for the size of the indices array.
			indices = (GLuint*)malloc(sizeof(GLuint) * numInds);

			// Copy the data from the passed in inds to the indices array.
			memcpy(indices, inds, sizeof(GLuint) * numInds);

			// Set the numIndices equal tot he passed in numInds.
			numIndices = numInds;
		}
		else
		{
			// Allocate space for enough indices to have one index per vertex.
			indices = (GLuint*)malloc(sizeof(GLuint) * numVerts);

			// Loop through and set each index to be in sequential order. (0, 1, 2, 3, 4, etc.)
			for (int i = 0; i < numVerts; i++)
			{
				indices[i] = i;
			}

			// Set the numIndices equal to the number of vertices.
			numIndices = numVerts;
		}

		// Initialize the buffer.
		InitBuffer();
	}
}

Model::~Model()
{
	// Free up any remaining data.
	free(vertices);
	free(indices);

	numVertices = 0;
	numIndices = 0;

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}

void Model::InitBuffer()
{
	// This generates buffer object names
	// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
	// (In this example, there's only one buffer object.)
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	//// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
	//// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
	//// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
	//// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
	//// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//// GL_ELEMENT_ARRAY_BUFFER is for vertex array indices, all drawing commands of glDrawElements will use indices from that buffer.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	//// Creates and initializes a buffer object's data.
	//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
	//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
	//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
	//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
	//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
	//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);

	//// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
	//// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
	//// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
	glEnableVertexAttribArray(0);

	//// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
	//// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
	//// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
	//// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
	//// then the pointer parameter is treated as a byte offset into the buffer object's data.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	//// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
	//// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
	//// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
	//// For more info on padding, Google it.

	//// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
}

void Model::UpdateBuffer()
{
	//// Creates and initializes a buffer object's data.
	//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
	//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
	//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
	//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
	//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
	//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);
}

void Model::Draw()
{
	// Draw vertices from the buffer as GL_TRIANGLES
	// There are several different drawing modes, GL_TRIANGLES takes every 3 vertices and makes them a triangle.
	// For reference, GL_TRIANGLE_STRIP would take each additional vertex after the first 3 and consider that a 
	// triangle with the previous 2 vertices (so you could make 2 triangles with 4 vertices)
	// The second parameter is the offset, the third parameter is the number of vertices to draw.
	//glDrawArrays(GL_TRIANGLES, 0, numVertices);



	// Draw numIndices vertices from the buffer as GL_TRIANGLES
	// There are several different drawing modes, GL_TRIANGLES takes every 3 vertices and makes them a triangle.
	// For reference, GL_TRIANGLE_STRIP would take each additional vertex after the first 3 and consider that a 
	// triangle with the previous 2 vertices (so you could make 2 triangles with 4 vertices)
	// The second parameter is the number of vertices, the third parameter is the type of the element buffer data, and the fourth parameter is the offset.
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}

GLuint Model::AddVertex(VertexFormat* vert)
{
	if (numVertices > 0)
	{
		// Allocate space equivalent to our current vertices array.
		VertexFormat* tempVerts = (VertexFormat*)malloc(sizeof(VertexFormat) * numVertices);
		
		// Copy our current vertices array into our temporary array.
		memcpy(tempVerts, vertices, numVertices);

		// Increase the number of vertices count by 1.
		numVertices++;

		// Free the vertices array.
		free(vertices);

		// Allocate space equivalent to our new vertices size.
		vertices = (VertexFormat*)malloc(sizeof(VertexFormat) * numVertices);

		// Copy the data from the temporary array back into the vertices array.
		memcpy(vertices, tempVerts, numVertices - 1);

		// Free the temporary array.
		free(tempVerts);

		// Set the last value in the vertices array to the new vertex.
		vertices[numVertices - 1] = *vert;

		// Update our buffer to match this change.
		UpdateBuffer();

		// Return the index reference to this vertex.
		return numVertices - 1;
	}
	else
	{
		// Create a new vertices array of size 1.
		vertices = (VertexFormat*)malloc(sizeof(VertexFormat));

		// Set the value to the new vertex.
		vertices[0] = *vert;

		// Set the number of vertices to 1.
		numVertices = 1;

		// Initialize the buffer.
		InitBuffer();

		// Return the index reference to this vertex (zero).
		return 0;
	}
}
void Model::AddIndex(GLuint index)
{
	if (numIndices > 0)
	{
		// Allocate space equivalent to our current indices array.
		GLuint* tempInds = (GLuint*)malloc(sizeof(GLuint) * numIndices);

		// Copy our current indices array into our temporary array.
		memcpy(tempInds, indices, numIndices);

		// Increase the number of indices count by 1.
		numIndices++;

		// Free the indices array.
		free(indices);

		// Allocate space equivalent to our new indices size.
		indices = (GLuint*)malloc(sizeof(GLuint) * numIndices);

		// Copy the data from the temporary array back into the indices array.
		memcpy(indices, tempInds, numIndices - 1);

		// Free the temporary array.
		free(tempInds);

		// Set the last value in the indices array to the new index.
		indices[numIndices - 1] = index;
	}
	else
	{
		// Create a new indices array of size 1.
		indices = (GLuint*)malloc(sizeof(GLuint));

		// Set the value to the new index.
		indices[0] = index;

		// Set the numIndices to 1.
		numIndices = 1;
	}
}

#endif _MODEL_CPP