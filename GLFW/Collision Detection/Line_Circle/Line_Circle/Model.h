/*
Title: Line and Circle 2D intersection
File Name: Model.h
Copyright © 2015
Revision Author: Srinivasan Thiagarajan
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
This is an example to detect the intersection of a line and circle in 2D.
You can control the two end-points of the line segment which a small part of the line, and move them using "w,a,s,d" and "i,j,k,l" respectively.
The line turns blue when an intersection is detected, and turns red when there is no intersection.
It checks for the point closest to the circle's center, which also lies on the line,
and checks if that point lies inside the circle. If so, collision is detected else not.
*/

#ifndef _MODEL_H
#define _MODEL_H

#include "GLIncludes.h"

class Model
{
private:
	int numVertices;
	VertexFormat* vertices;

	int numIndices;
	GLuint* indices;

	GLuint vbo;
	GLuint ebo;

	//GLuint shaderProgram;
	//GLuint m_Buffer;

public:
	Model(int numVerts = 0, VertexFormat* verts = nullptr, int numInds = 0, GLuint* inds = nullptr);
	~Model();

	GLuint AddVertex(VertexFormat*);
	void AddIndex(GLuint);

	void InitBuffer();
	void UpdateBuffer();

	void Draw();

	// Our get variables.
	int NumVertices()
	{
		return numVertices;
	}
	int NumIndices()
	{
		return numIndices;
	}
	VertexFormat* Vertices()
	{
		return vertices;
	}
	GLuint* Indices()
	{
		return indices;
	}

	/*Model(int p_nVertices = 3, float _size = 1.0f, float _originX = 0.0f, float _originY = 0.0f, float _originZ = 0.0f)
	{
		if (p_nVertices < 3)
			p_nVertices = 3;

		m_ShaderProgram = 0;
		m_Buffer = 0;

		m_nVertices = p_nVertices;
		m_pPoint = new Point3D[p_nVertices];
		SetOrigin(_originX, _originY, _originZ);
		GeneratePoints(_size);
	}*/

//	~Figure(void)
//	{
//		Release();
//	}
//
//	void Release(void)
//	{
//		m_nVertices = 0;
//		if (m_pPoint != nullptr)
//		{
//			delete[] m_pPoint;
//			m_pPoint = nullptr;
//		}
//	}
//
//	void SetPoint(int nIndex, float x, float y, float z = 0.0f)
//	{
//		if (nIndex < 0 || nIndex >= m_nVertices || m_pPoint == nullptr)
//			return;
//
//		m_pPoint[nIndex].x = x;
//		m_pPoint[nIndex].y = y;
//		m_pPoint[nIndex].z = z;
//	}
//
//	void SetPoint(int nIndex, Point3D p_Point)
//	{
//		if (nIndex < 0 || nIndex >= m_nVertices || m_pPoint == nullptr)
//			return;
//
//		m_pPoint[nIndex] = p_Point;
//	}
//
//	void operator()(int nIndex, float x, float y, float z = 0.0f)
//	{
//		if (nIndex < 0 || nIndex >= m_nVertices || m_pPoint == nullptr)
//			return;
//
//		m_pPoint[nIndex].x = x;
//		m_pPoint[nIndex].y = y;
//		m_pPoint[nIndex].z = z;
//	}
//
//	void operator()(int nIndex, Point3D p_Point)
//	{
//		if (nIndex < 0 || nIndex >= m_nVertices || m_pPoint == nullptr)
//			return;
//
//		m_pPoint[nIndex] = p_Point;
//	}
//
//	Point3D& operator[](int nIndex)
//	{
//		if (nIndex < 0 || nIndex >= m_nVertices || m_pPoint == nullptr)
//			assert(false);
//
//		return m_pPoint[nIndex];
//	}
//
//	void SetOrigin(float x, float y, float z)
//	{
//		m_Origin.x = x;
//		m_Origin.y = y;
//		m_Origin.z = z;
//	}
//
//	void CompileFigure(void)
//	{
//		for (int i = 0; i < m_nVertices; i++)
//		{
//			m_pPoint[i].x += m_Origin.x;
//			m_pPoint[i].y += m_Origin.y;
//			m_pPoint[i].z += m_Origin.z;
//		}
//		InitBuffer();
//	}
//
//	friend std::ostream& operator<<(std::ostream os, Figure& other)
//	{
//		other.PrintContent();
//		return os;
//	}
//
//	void Render(void)
//	{
//		// Use the buffer and shader for each circle.
//		glUseProgram(m_ShaderProgram);
//		glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
//
//		// Initialize the vertex position attribute from the vertex shader.
//		GLuint loc = glGetAttribLocation(m_ShaderProgram, "vPosition");
//		glEnableVertexAttribArray(loc);
//		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
//
//		// Draw the array of this figure
//		glDrawArrays(GL_TRIANGLE_FAN, 0, m_nVertices);
//	}
//
//private:
//	void InitBuffer(void)
//	{
//		// Create a vertex array object
//		GLuint vao;
//		glGenVertexArrays(1, &vao);
//		glBindVertexArray(vao);
//
//		// Create and initialize a buffer object for each circle.
//		glGenBuffers(1, &m_Buffer);
//		glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
//		glBufferData(GL_ARRAY_BUFFER, m_nVertices * sizeof(Point3D), m_pPoint, GL_STATIC_DRAW);
//
//		// Load shaders and use the resulting shader program
//		m_ShaderProgram = InitShader("Shaders\\vshader.glsl", "Shaders\\fshader.glsl");
//	}
//
//	void GeneratePoints(float fSize = 1.0f)
//	{
//		GLfloat theta = 0;
//		for (int i = 0; i < m_nVertices; i++)
//		{
//			theta += static_cast<GLfloat>(2 * M_PI / m_nVertices);
//			m_pPoint[i].x = static_cast<GLfloat>(cos(theta)) * fSize;
//			m_pPoint[i].y = static_cast<GLfloat>(sin(theta)) * fSize;
//		}
//		CompileFigure();
//	}
//
//	void PrintContent(void)
//	{
//		std::cout << "Content:" << std::endl;
//		for (int i = 0; i < m_nVertices; i++)
//		{
//			std::cout << m_pPoint[i] << std::endl;
//		}
//	}
};

#endif //_MODEL_H
