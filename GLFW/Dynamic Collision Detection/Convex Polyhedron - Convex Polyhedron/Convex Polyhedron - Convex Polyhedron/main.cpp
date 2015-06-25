/*
Title: Convex Polygon - Convex Polygon (2D)
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
This is a demonstration of using continuous collision detection to prevent tunnelling.
The demo contains two moving tetrahedrons, one pink, and one yellow.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving polyhedrons to move any further.
when a moving box reaches one side of the screen, it will wrap around to the other side again.

The user can disable the continuous collision detection by holding spacebar.
This will cause the program to nopt run any collision detection.
When two polyhedrons collide the user can cause the simulation
to continue by toggling collision detection off. 
(Release spacebar if pressed, tap and hold spacebar, then release).

This algorithm detects potentially missed collisions by performing a dynamic version of the
separating axis test. First we must determine the distances along each axis signifying
the distance to begin collision (dFirst) & the distance to separate from that collision (dLast). Then
we can easily determine the time at which these distances will be reached by dividing them by the magnitude of the
velocity along the axis (tFirst, tLast). If we keep the largest tFirst and the smallest tLast from all axes,
we will determine the time interval which the polygons will be intersecting! If tLast < tFirst, the boxes will not overlap.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
AABB - AABB (Dynamic 2D) by Nicholas Gallagher
Convex Hull (3D) by Nicholas Gallagher
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader program
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

//Uniforms
GLuint uniMVP;
GLuint uniHue;
glm::mat4 VP;
glm::mat4 hue;
// Reference to the window object being created by GLFW.
GLFWwindow* window;

struct Vertex
{
	float
		x, y, z,
		r, g, b, a;
};

//Struct for rendering
struct Mesh
{
	GLuint VBO;
	GLuint VAO;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	int numVertices;
	struct Vertex* vertices;
	GLenum primitive;

	Mesh::Mesh(int numVert, struct Vertex* vert, GLenum primType)
	{

		glm::mat4 translation = glm::mat4(1.0f);
		glm::mat4 rotation = glm::mat4(1.0f);
		glm::mat4 scale = glm::mat4(1.0f);

		this->numVertices = numVert;
		this->vertices = new struct Vertex[this->numVertices];
		memcpy(this->vertices, vert, this->numVertices * sizeof(struct Vertex));

		this->primitive = primType;

		//Generate VAO
		glGenVertexArrays(1, &this->VAO);
		//bind VAO
		glBindVertexArray(VAO);

		//Generate VBO
		glGenBuffers(1, &this->VBO);

		//Configure VBO
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * this->numVertices, this->vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)12);
	}

	Mesh::~Mesh(void)
	{
		delete[] this->vertices;
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);
	}

	glm::mat4 Mesh::GetModelMatrix()
	{
		return translation * rotation * scale;
	}

	void Mesh::Draw(void)
	{
		//GEnerate the MVP for this model
		glm::mat4 MVP = VP * this->GetModelMatrix();

		//Bind the VAO being drawn
		glBindVertexArray(this->VAO);

		// Set the uniform matrix in our shader to our MVP matrix for this mesh.
		glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
		//Draw the mesh
		glDrawArrays(this->primitive, 0, this->numVertices);
	}
};

//Struct for linear kinematics
struct RigidBody
{
	glm::vec3 position;			//Position of the rigidbody
	glm::vec3 velocity;			//The velocity of the rigidbody
	glm::vec3 acceleration;		//The acceleration of the rigidbody

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	///
	//Parameterized constructor, creates rigidbody at certain position with certain velocity and acceleration
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc)
	{
		position = pos;
		velocity = vel;
		acceleration = acc;
	}
};

struct Edge
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::vec3 direction;
};

//Convex hull collider struct
struct ConvexHull
{
	glm::vec3 center;
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> edgeDirections;
	std::vector<glm::vec3> normals;

	///
	//Generates a convex hull from a given mesh
	//This method assumes the vertices of the mesh are given as lines
	//Where each set of two vertices represents two endpoints of a line which makes up the mesh.
	//And every two lines are coplanar
	//
	//Parameters:
	//	m: A reference to the mesh to fit this convex hull to
	ConvexHull::ConvexHull(const Mesh &m, const glm::vec3 c, float scale)
	{
		center = c;

		//We must be able to track adjacent edges
		std::vector<struct Edge> edges;

		//Loop through all mesh vertices and add them and
		//the edges which they make up to the convex hull points if they are not duplicates
		for (int i = 0; i < m.numVertices; i += 2)
		{
			//Construct a vec3 to represent the ith vertex
			glm::vec3 currentPoint(m.vertices[i].x * scale, m.vertices[i].y * scale, m.vertices[i].z * scale);

			//Check if it is a duplicate
			bool contains = false;
			int size = points.size();
			for (int j = 0; j < size; j++)
			{
				if (points[j] == currentPoint)
				{
					contains = true;
					break;
				}
			}

			//If it is not a duplicate add it to the list
			if (!contains)
				points.push_back(currentPoint);

			//Construct a vec3 to represent the (i + 1)th vertex
			glm::vec3 nextPoint(m.vertices[i + 1].x * scale, m.vertices[i + 1].y * scale, m.vertices[i + 1].z * scale);
			//Check if it's a duplicate
			contains = false;
			for (int j = 0; j < size; j++)
			{
				if (points[j] == nextPoint)
				{
					contains = true;
					break;
				}
			}
			//If it is not a duplicate add it to the list
			if (!contains)
				points.push_back(nextPoint);

			//Generate an edge from the two points
			struct Edge edge =
			{
				currentPoint,
				nextPoint,
				glm::normalize(nextPoint - currentPoint)
			};

			//Add it to the list of edges
			edges.push_back(edge);

			//Check if the edges direction is contained in the list of edge directions yet
			contains = false;
			size = edgeDirections.size();
			for (int j = 0; j < size; j++)
			{
				if (edgeDirections[j] == edge.direction)
				{
					contains = true;
					break;
				}
			}

			//If it is not a duplicate add it to the list
			if (!contains)
				edgeDirections.push_back(edge.direction);
		}

		//We must loop through all edges and find the co-planar ones to take the cross product
		//And generate face normals for the convex hull
		int size = edges.size();
		for (int i = 0; i < size; i++)
		{
			for (int j = i; j < size; j++)
			{
				//If the ith edge and the jth edge have a point in common we must cross their directions to get a face normal
				if (edges[i].startPoint == edges[j].startPoint ||
					edges[i].startPoint == edges[j].endPoint ||
					edges[i].endPoint == edges[j].startPoint ||
					edges[i].endPoint == edges[j].endPoint)
				{
					glm::vec3 norm = glm::normalize(glm::cross(edges[i].direction, edges[j].direction));
					//Check if normal is contained in the list of normals yet
					bool contains = false;
					int nSize = normals.size();
					for (int k = 0; k < nSize; k++)
					{
						if (normals[k] == norm)
						{
							contains = true;
							break;
						}
					}

					//If it is not a duplicate, add it to the list
					if (!contains)
						normals.push_back(norm);

				}
			}
		}
	}
};

struct Mesh* polyhedron1;
struct Mesh* polyhedron2;

struct RigidBody* polyhedron1Body;
struct RigidBody* polyhedron2Body;

struct ConvexHull* convexHull1;
struct ConvexHull* convexHull2;


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.5; // This is the number of milliseconds we intend for the physics to update.


#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Reads a shader file and returns a string containing the source
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	std::ifstream file(fileName, std::ios::in);

	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;
		return "";
	}

	file.seekg(0, std::ios::end);
	shaderCode.resize((unsigned int)file.tellg());
	file.seekg(0, std::ios::beg);

	file.read(&shaderCode[0], shaderCode.size());

	file.close();

	return shaderCode;
}

//Creates shader from source code
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str();
	const int shader_code_size = sourceCode.size();


	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader);

	GLint isCompiled = 0;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;
		glDeleteShader(shader);
	}

	return shader;
}

// Initialization code
void init()
{
	//Initialize glew
	glewInit();

	//create shader
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	//Generate the View Projection matrix
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	VP = proj * view;

	//Create uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set rendering options
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glLineWidth(5.0f);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Performs a dynamic collision check between a moving convex polyhedron and a static convex polyhedron.
//
//Overview:
//	This algorithm detects potentially missed collisions by performing a moving version of the 
//	separating axis test. First we must determine the distances along each axis signifying
//	the distance to begin collision (dFirst) & the distance to separate from that collision (dLast). Then
//	we can easily determine the time at which these distances will be reached by dividing them by the magnitude of the
//	velocity along the axis (tFirst / tLast). If we keep the largest tFirst and the smallest tLast from all axes,
//	we will determine the time interval which the boxes will be intersecting! If tLast < tFirst, the boxes will not overlap.
//	Alternatively, if tFirst > 1.0f,  the boxes will not overlap!
//
//Parameters:
//	hull1: The moving convex hull
//	hull2: The static convex hulll
//	mvmt: The relative movement vector of box 1 from an observer on box 2
//
//Returns:
//	a t value between 0 and 1 indicating the "relative time" since the start of this frame that the collision occurred.
//	a t value of 0.0f would indicate the very start of this frame, a t value of 1.0f would indicate the very end of this frame.
float CheckDynamicCollision(const ConvexHull &hull1, const ConvexHull &hull2, const glm::vec3 &mvmt)
{
	float tFirst = 0.0f;
	float tLast = 1.0f;
	float tCurrent = 0.0f;

	//Get the points in worldspace for both objects
	std::vector<glm::vec3> worldPts1, worldPts2;
	int pSize = hull1.points.size();
	for (int i = 0; i < pSize; i++)
	{
		worldPts1.push_back(hull1.points[i] + hull1.center);
	}
	pSize = hull2.points.size();
	for (int i = 0; i < pSize; i++)
	{
		worldPts2.push_back(hull2.points[i] + hull2.center);
	}

	//For every axis on hull1
	int nSize = hull1.normals.size();
	for (int i = 0; i < nSize; i++)
	{
		glm::vec3 currentNormal = hull1.normals[i];

		//Determine the projection bounds of hull1 on this axis
		float min1, max1;
		min1 = max1 = glm::dot(worldPts1[0], currentNormal);
		//For each point after the first
		pSize = worldPts1.size();
		for (int j = 1; j < pSize; j++)
		{
			//Get scalar projection
			float sProj = glm::dot(worldPts1[j], currentNormal);
			//Determine if this point is a bound on the current axis
			if (min1 > sProj) min1 = sProj;
			if (max1 < sProj) max1 = sProj;
		}

		//Determine the projection bounds of hull2 on this axis
		float min2, max2;
		min2 = max2 = glm::dot(worldPts2[0], currentNormal);
		//For each point after the first
		int pSize = hull2.points.size();
		for (int j = 1; j < pSize; j++)
		{
			//Get scalar projection
			float sProj = glm::dot(worldPts2[j], currentNormal);
			//Determine if this point is a bound on the current axis
			if (min2 > sProj) min2 = sProj;
			if (max2 < sProj) max2 = sProj;
		}

		//Check the direction of the projection of the movement vector on this axis
		float sProjMvmt = glm::dot(mvmt, currentNormal);
		if (sProjMvmt < 0.0f)
		{
			//In this case object 1 is moving in the negative direction along axis from an observer on object 2.
			//So if object 1 is more negative in direction than object 2, they will not collide on this axis.
			if (max1 < min2) return -1.0f;
			//Is the "low part" object 1 higher than the "high part" object 2 along this axis?
			if (min1 > max2)
			{
				//If so, the shapes are not yet colliding on this axis, so determine when they first will collide on this axis
				tCurrent = (max2 - min1) / sProjMvmt;			//We solve for a negative distance here, because we are dividing by a negative velocity to get a positive time
				//This strange ordering prevents us from needing to make a call to fabs (absolute value function)
				//If it is larger than the current tFirst, change tFirst
				if (tCurrent > tFirst) tFirst = tCurrent;
			}
			//Is the "High Part" of object 1 higher than the low part of object 2 along this axis?
			if (max1 > min2)
			{
				//If so, the shapes have not yet separated on this axis, so determine when they will finish colliding
				tCurrent = (min2 - max1) / sProjMvmt;		//Note the wierd ordering again, for the same reason as above
				//If it is smaller than current tLast, update tLast
				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
		else if (sProjMvmt > 0.0f)
		{
			//If object 1 is more positive along the axis than object 2, they will not collide
			if (min1 > max2) return -1.0f;
			//Is the "High part" of object 1 lower than the "low part" of object 2 along this axis?
			if (max1 < min2)
			{
				//If so, the shapes are not yet colliding on this axis so determine when they will first collide on this axis
				tCurrent = (min2 - max1) / sProjMvmt;
				//If it is larger than the current tFirst, update tFirst
				if (tCurrent > tFirst) tFirst = tCurrent;
			}
			//Is the "Low part" of object 1 lower than the "high part" of object 2 along this axis?
			if (min1 < max2)
			{
				//If so, the shapes have not yet separated on this axis, so determine when they will finish collidiing
				tCurrent = (max2 - min1) / sProjMvmt;
				//If it is smaller than the current tLast, update tLast
				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
	}

	//For every axis on hull2
	nSize = hull2.normals.size();
	for (int i = 0; i < nSize; i++)
	{
		glm::vec3 currentNormal = hull2.normals[i];

		//Determine the projection bounds of hull1 on this axis
		float min1, max1;
		min1 = max1 = glm::dot(worldPts1[0], currentNormal);
		//For each point after the first
		pSize = worldPts1.size();
		for (int j = 1; j < pSize; j++)
		{
			//Get scalar projection
			float sProj = glm::dot(worldPts1[j], currentNormal);
			//Determine if this point is a bound on the current axis
			if (min1 > sProj) min1 = sProj;
			if (max1 < sProj) max1 = sProj;
		}

		//Determine the projection bounds of hull2 on this axis
		float min2, max2;
		min2 = max2 = glm::dot(worldPts2[0], currentNormal);
		//For each point after the first
		int pSize = hull2.points.size();
		for (int j = 1; j < pSize; j++)
		{
			//Get scalar projection
			float sProj = glm::dot(worldPts2[j], currentNormal);
			//Determine if this point is a bound on the current axis
			if (min2 > sProj) min2 = sProj;
			if (max2 < sProj) max2 = sProj;
		}

		//Check the direction of the projection of the movement vector on this axis
		float sProjMvmt = glm::dot(mvmt, currentNormal);
		if (sProjMvmt < 0.0f)
		{
			//In this case object 1 is moving in the negative direction along axis from an observer on object 2.
			//So if object 1 is more negative in direction than object 2, they will not collide on this axis.
			if (max1 < min2) return -1.0f;
			//Is the "low part" object 1 higher than the "high part" object 2 along this axis?
			if (min1 > max2)
			{
				//If so, the shapes are not yet colliding on this axis, so determine when they first will collide on this axis
				tCurrent = (max2 - min1) / sProjMvmt;			//We solve for a negative distance here, because we are dividing by a negative velocity to get a positive time
				//This strange ordering prevents us from needing to make a call to fabs (absolute value function)
				//If it is larger than the current tFirst, change tFirst
				if (tCurrent > tFirst) tFirst = tCurrent;
			}
			//Is the "High Part" of object 1 higher than the low part of object 2 along this axis?
			if (max1 > min2)
			{
				//If so, the shapes have not yet separated on this axis, so determine when they will finish colliding
				tCurrent = (min2 - max1) / sProjMvmt;		//Note the wierd ordering again, for the same reason as above
				//If it is smaller than current tLast, update tLast
				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
		else if (sProjMvmt > 0.0f)
		{
			//If object 1 is more positive along the axis than object 2, they will not collide
			if (min1 > max2) return -1.0f;
			//Is the "High part" of object 1 lower than the "low part" of object 2 along this axis?
			if (max1 < min2)
			{
				//If so, the shapes are not yet colliding on this axis so determine when they will first collide on this axis
				tCurrent = (min2 - max1) / sProjMvmt;
				//If it is larger than the current tFirst, update tFirst
				if (tCurrent > tFirst) tFirst = tCurrent;
			}
			//Is the "Low part" of object 1 lower than the "high part" of object 2 along this axis?
			if (min1 < max2)
			{
				//If so, the shapes have not yet separated on this axis, so determine when they will finish collidiing
				tCurrent = (max2 - min1) / sProjMvmt;
				//If it is smaller than the current tLast, update tLast
				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
	}

	//For the cross product of the edges
	int eSize1 = hull1.edgeDirections.size();
	int eSize2 = hull1.edgeDirections.size();
	for (int i = 0; i < eSize1; i++)	//For each edge in hull1
	{
		for (int j = 0; j < eSize2; j++)	//For each edge in hull2
		{
			//Take the cross product of the two & test that normal!
			glm::vec3 normal = glm::cross(glm::normalize(hull1.edgeDirections[i]), glm::normalize(hull2.edgeDirections[i]));

			//If the vector is the zero vector, continue testing
			if (glm::length(normal) <= 3.0f * FLT_EPSILON) continue;

			//Determine the projection bounds of hull1 on this axis
			float min1, max1;
			min1 = max1 = glm::dot(worldPts1[0], normal);
			//For each point after the first
			pSize = worldPts1.size();
			for (int j = 1; j < pSize; j++)
			{
				//Get scalar projection
				float sProj = glm::dot(worldPts1[j], normal);
				//Determine if this point is a bound on the current axis
				if (min1 > sProj) min1 = sProj;
				if (max1 < sProj) max1 = sProj;
			}

			//Determine the projection bounds of hull2 on this axis
			float min2, max2;
			min2 = max2 = glm::dot(worldPts2[0], normal);
			//For each point after the first
			int pSize = hull2.points.size();
			for (int j = 1; j < pSize; j++)
			{
				//Get scalar projection
				float sProj = glm::dot(worldPts2[j], normal);
				//Determine if this point is a bound on the current axis
				if (min2 > sProj) min2 = sProj;
				if (max2 < sProj) max2 = sProj;
			}

			//Check the direction of the projection of the movement vector on this axis
			float sProjMvmt = glm::dot(mvmt, normal);
			if (sProjMvmt < 0.0f)
			{
				//In this case object 1 is moving in the negative direction along axis from an observer on object 2.
				//So if object 1 is more negative in direction than object 2, they will not collide on this axis.
				if (max1 < min2) return -1.0f;
				//Is the "low part" object 1 higher than the "high part" object 2 along this axis?
				if (min1 > max2)
				{
					//If so, the shapes are not yet colliding on this axis, so determine when they first will collide on this axis
					tCurrent = (max2 - min1) / sProjMvmt;			//We solve for a negative distance here, because we are dividing by a negative velocity to get a positive time
					//This strange ordering prevents us from needing to make a call to fabs (absolute value function)
					//If it is larger than the current tFirst, change tFirst
					if (tCurrent > tFirst) tFirst = tCurrent;
				}
				//Is the "High Part" of object 1 higher than the low part of object 2 along this axis?
				if (max1 > min2)
				{
					//If so, the shapes have not yet separated on this axis, so determine when they will finish colliding
					tCurrent = (min2 - max1) / sProjMvmt;		//Note the wierd ordering again, for the same reason as above
					//If it is smaller than current tLast, update tLast
					if (tCurrent < tLast) tLast = tCurrent;
				}
			}
			else if (sProjMvmt > 0.0f)
			{
				//If object 1 is more positive along the axis than object 2, they will not collide
				if (min1 > max2) return -1.0f;
				//Is the "High part" of object 1 lower than the "low part" of object 2 along this axis?
				if (max1 < min2)
				{
					//If so, the shapes are not yet colliding on this axis so determine when they will first collide on this axis
					tCurrent = (min2 - max1) / sProjMvmt;
					//If it is larger than the current tFirst, update tFirst
					if (tCurrent > tFirst) tFirst = tCurrent;
				}
				//Is the "Low part" of object 1 lower than the "high part" of object 2 along this axis?
				if (min1 < max2)
				{
					//If so, the shapes have not yet separated on this axis, so determine when they will finish collidiing
					tCurrent = (max2 - min1) / sProjMvmt;
					//If it is smaller than the current tLast, update tLast
					if (tCurrent < tLast) tLast = tCurrent;
				}
			}
		}
	}

	//If there was no overlap
	if (tLast < tFirst) return -1.0f;

	return tFirst;

}


// This runs once every physics timestep.
void update(float dt)
{
	//If the user presses spacebar, do not detect collision
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		//Move the polyhedron rigid bodies
		polyhedron1Body->position += polyhedron1Body->velocity * dt;
		polyhedron2Body->position += polyhedron2Body->velocity * dt;

		//Move colliders
		convexHull1->center = polyhedron1Body->position;
		convexHull2->center = polyhedron2Body->position;

		//If the position goes off of the right edge of the screen, loop it back to the left
		if (polyhedron1Body->position.x > 1.0f)
		{

			polyhedron1Body->position.x = -1.0f;
			convexHull1->center = polyhedron1Body->position;
		}
		if (polyhedron2Body->position.x < -1.0f)
		{
			polyhedron2Body->position.x = 1.0f;
			convexHull2->center = polyhedron2Body->position;
		}

		//Once we have solved for the bodys position, we should translate the mesh to match it
		polyhedron1->translation = glm::translate(glm::mat4(1.0f), polyhedron1Body->position);
		polyhedron2->translation = glm::translate(glm::mat4(1.0f), polyhedron2Body->position);
	}
	//Else, use continuous collision detection
	else
	{
		//Determine relative velocity of polyhedron 1 from a stationary polyhedron 2
		glm::vec3 relV = polyhedron1Body->velocity - polyhedron2Body->velocity;
		float t = CheckDynamicCollision(*convexHull1, *convexHull2, relV * dt);

		if (t >= 0.0f)
		{
			//Reposition at point of intersection
			polyhedron1Body->position += polyhedron1Body->velocity * dt * t;
			polyhedron2Body->position += polyhedron2Body->velocity * dt * t;

		}
		else
		{
			//Move the polyhedron rigid bodies
			polyhedron1Body->position += polyhedron1Body->velocity * dt;
			polyhedron2Body->position += polyhedron2Body->velocity * dt;
		}



		//Move colliders
		convexHull1->center = polyhedron1Body->position;
		convexHull2->center = polyhedron2Body->position;

		//If the position goes off of the right edge of the screen, loop it back to the left
		if (polyhedron1Body->position.x > 1.0f)
		{

			polyhedron1Body->position.x = -1.0f;
			convexHull1->center = polyhedron1Body->position;
		}
		if (polyhedron2Body->position.x < -1.0f)
		{
			polyhedron2Body->position.x = 1.0f;
			convexHull2->center = polyhedron2Body->position;
		}

		//Once we have solved for the bodys position, we should translate the mesh to match it
		polyhedron1->translation = glm::translate(glm::mat4(1.0f), polyhedron1Body->position);
		polyhedron2->translation = glm::translate(glm::mat4(1.0f), polyhedron2Body->position);
	}



}

// This runs once every frame to determine the FPS and how often to call update based on the physics step.
void checkTime()
{
	// Get the current time.
	time = glfwGetTime();

	// Get the time since we last ran an update.
	double dt = time - timebase;

	// If more time has passed than our physics timestep.
	if (dt > physicsStep)
	{

		timebase = time; // Set timebase = time so we have a reference for when we ran the last physics timestep.

		// Limit dt so that we if we experience any sort of delay in processing power or the window is resizing/moving or anything, it doesn't update a 
		// bunch of times while the player can't see.
		// This will limit it to .25 seconds of updating, before it just freezes the game until it responds again.
		if (dt > 0.25)
		{
			dt = 0.25;
		}

		// The accumulator is here so that we can track the amount of time that needs to be updated based on dt, but not actually update at dt intervals and instead use 
		// our physicsStep.
		accumulator += dt;

		// Run a while loop, that runs update(physicsStep) until the accumulator no longer has any time left in it (or the time left is less than physicsStep, at which 
		// point it save that leftover time and use it in the next checkTime() call.
		while (accumulator >= physicsStep)
		{
			update(physicsStep);
			accumulator -= physicsStep;
		}
	}
}



// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Draw the Gameobjects
	polyhedron1->Draw();
	polyhedron2->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	//Create window
	window = glfwCreateWindow(800, 800, "Convex Polygon - Convex Polygon (2D Dynamic Collision Detection)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate the polyhedron1 mesh
	float polyScale = 0.1f;
	struct Vertex poly1Verts[12];
	poly1Verts[0] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[1] = { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[2] = { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[3] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[4] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[5] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[6] = { 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[7] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[8] = { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[9] = { 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[10] = { 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	poly1Verts[11] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };

	struct Vertex poly2Verts[12];
	poly2Verts[0] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[1] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[2] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[3] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[4] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[5] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[6] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[7] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[8] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[9] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[10] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	poly2Verts[11] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };


	//polyhedron1 creation
	polyhedron1 = new struct Mesh(12, poly1Verts, GL_LINES);

	//polyhedron2 creation
	polyhedron2 = new struct Mesh(12, poly2Verts, GL_LINES);


	//Scale the polyhedrons
	polyhedron1->scale = glm::scale(polyhedron1->scale, glm::vec3(polyScale));
	polyhedron2->scale = glm::scale(polyhedron2->scale, glm::vec3(polyScale));


	//Generate the polygons's rigidbody
	polyhedron1Body = new struct RigidBody(
		glm::vec3(-1.0f, 0.0f, 0.0f),		//Start on left side of screen
		glm::vec3(1.0f, 0.0f, 0.0f),		//constant right velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	polyhedron2Body = new struct RigidBody(
		glm::vec3(0.75f, 0.0f, 0.0f),		//Start on right side
		glm::vec3(-0.5f, 0.0f, 0.0f),		//Constant left velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	//Position polygons
	polyhedron1->translation = glm::translate(polyhedron1->translation, polyhedron1Body->position);
	polyhedron2->translation = glm::translate(polyhedron2->translation, polyhedron2Body->position);

	convexHull1 = new ConvexHull(*polyhedron1, polyhedron1Body->position, polyScale);
	convexHull2 = new ConvexHull(*polyhedron2, polyhedron1Body->position, polyScale);

	//Print controls
	std::cout << "Controls:\nPress and hold spacebar to disable collision detection.\nWhen two polygons collide, continue the simulation by toggling collision detection off and back on.\n";
	std::cout << "(tap and hold spacebar, then release.)\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		//Check time will update the programs clock and determine if & how many times the physics must be updated
		checkTime();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	delete polyhedron1;
	delete polyhedron2;
	delete polyhedron1Body;
	delete polyhedron2Body;
	delete convexHull1;
	delete convexHull2;

	// Frees up GLFW memory
	glfwTerminate();
}