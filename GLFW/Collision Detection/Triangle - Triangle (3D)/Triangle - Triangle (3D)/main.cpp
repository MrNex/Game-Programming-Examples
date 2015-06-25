/*
Title: Triangle - Triangle (3D)
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
Tests for collisions between two triangles

This algorithm tests for collisions between two triangles based on the algorithm
outlined in "A Fast Triangle - Triangle Intersection Test" by Tomas Moller ('97)
found here:
	http://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf

References:
A Fast Triangle - Triangle Intersection Test by Tomas Moller
Base by Srinivasan Thiagarajan
Line Segment - Triangle by Nicholas Gallagher
Point - Triangle by Nicholas Gallagher
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader variables
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

//Uniform variables
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

//Triangle collider struct
struct Triangle
{
	glm::vec3 a, b, c;

	///
	//Default constructor, constructs basic triangle
	Triangle::Triangle()
	{
		a = glm::vec3(-1.0f, -1.0f, 0.0f);
		b = glm::vec3(1.0f, -1.0f, 0.0f);
		c = glm::vec3(0.0f, 1.0f, 0.0f);
	}


	///
	//Parameterized constructor, constructs triangle pA pB pC
	Triangle::Triangle(glm::vec3 pA, glm::vec3 pB, glm::vec3 pC)
	{
		a = pA;
		b = pB;
		c = pC;
	}
};


struct Mesh* triangle1;
struct Mesh* triangle2;

struct Mesh* selectedShape;

struct Triangle* triangle1Collider;
struct Triangle* triangle2Collider;

float movementSpeed = 0.02f;
float rotationSpeed = 0.01f;

bool isMousePressed = false;
double prevMouseX = 0.0f;
double prevMouseY = 0.0f;

//Out of order Function declarations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Reads Shader Source
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

//Creates shader from source
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
	// Initializes the glew library
	glewInit();
	glEnable(GL_DEPTH_TEST);

	//Create shader program
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

	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	glPointSize(3.0f);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Detects if a point is contained in a triangle when the point and the triangle are co-planar
//
//PArameters:
//	A: point 1 of triangle
//	B: Point 2 of triangle
//	C: Point 3 of triangle
//	point: The point to check
//
//Returns:
//	True if point is detected as being inside triangle, else false
bool CheckPoint(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &C, const glm::vec3 &point)
{
	//In order for the point(P) to be inside of the triangle(made up of points ABC)
	//	the three triangles made from points PAB, PBC, and PCA must have normals in the same direction.
	//	That is to say that the cycles PAB, PBC, and PCA must all go either clockwise or counter clockwise!
	//	In order to test this we must get the edges:
	//	PA, AB, PB, BC, and PC, CA
	//	If any two triangles point in different directions, we can exit early because the point cannot be
	//	inside of triangle ABC.

	//We will start with computing the normal of PAB
	glm::vec3 PA = A - point;
	glm::vec3 AB = B - A;
	glm::vec3 PABNormal = glm::cross(PA, AB);

	//Next we will compute the normal of PBC
	glm::vec3 PB = B - point;
	glm::vec3 BC = C - B;
	glm::vec3 PBCNormal = glm::cross(PB, BC);

	//If PABNormal and PBCNormal do not go in the same direction, the point is not in the triangle!
	if (glm::dot(PABNormal, PBCNormal) <= 0.0f) return false;

	//Next we will test PCA
	glm::vec3 PC = C - point;
	glm::vec3 CA = A - C;
	glm::vec3 PCANormal = glm::cross(PC, CA);

	//If PCANormal does not go in the same direction as PABNormal and PBCNormal the point is not in the triangle!
	if (glm::dot(PABNormal, PCANormal) <= 0.0f) return false;

	return true;
}

///
//Tests if an edge of a triangle is colliding with the edge of a triangle using the parametric equation of lines
//
//Parameters:
//	P1: One endpoint of edge 1
//	P2: other endpoint of edge 1
//	E1: One endpoint of the edge 2
//	E2: Another endpoint of the edge 2
//
//Returns: True if an intersection is detected, else false.
bool CheckEdges(glm::vec3 P1, glm::vec3 P2, glm::vec3 E1, glm::vec3 E2)
{
	glm::vec3 lineDir = P2 - P1;
	glm::vec3 edgeDir = E2 - E1;

	//Using the parametric equation for a line we have:
	//P1 + t * lineDir = A + s * edgeDir
	//And reducing this to a system of equations:
	//P1.x + t * lineDir.x = A.x + s * edgeDir.x
	//p1.y + t * lineDir.y = A.y + s * edgeDir.y
	//p1.z + t * lineDir.z = A.z + s * edgeDir.z
	//
	//From here we can easily solve for t and s using any system, however we must be careful not to divide by 0!

	if (lineDir.x != 0.0f)
	{
		//We will use the first equation to solve for t
		//t = (A.x + s * AB.x - P1.x) / lineDir.x
		//Next we can substitute t into one of the other equations, we will use the second equation
		//P1.y + (lineDir.y / lineDir.x) * (A.x + s * AB.x - P1.x) = A.y + s * AB.y
		float quot = lineDir.y / lineDir.x;

		//Finally, we solve for s
		//P1.y + quot * A.x + s * quot * AB.x - quot * P1.x = A.y + s * AB.y
		//P1.y - A.y + quot * A.x - quot * P1.x = s * AB.y - s * quot * AB.x
		//s * (AB.y - quot * AB.x) = P1.y - A.y + quot * A.x - quot * P1.x
		//s = (P1.y - A.y + quot * A.x - quot * P1.x) / (AB.y - quot * AB.x)
		float s = (P1.y - E1.y + quot * E1.x - quot * P1.x) / (edgeDir.y - quot * edgeDir.x);

		//Solve for t using t = (A.x + s * AB.x - P1.x) / lineDir.x
		float t = (E1.x + s * edgeDir.x - P1.x) / lineDir.x;

		//If these give us a point that is within the segments, then there is a collision, if not we must check the other edges
		if (0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f) return true;
	}
	else if (lineDir.y != 0.0f)
	{
		//We will use the second equation to solve for t
		//t = (A.y + s * AB.y - P1.y) / lineDir.y
		//Next we can substitute t into one of the other equations, we will use the first because we know lineDir.x is 0
		//P1.x + (lineDir.x / lineDir.y) * (A.y + s * AB.y - P1.y) = A.x + s * AB.x
		//P1.x = A.x + s * AB.x
		//s = (P1.x - A.x) / AB.x
		float s = (P1.x - E1.x) / edgeDir.x;

		//Solve for t using t = (A.y + s * AB.y - P1.y) / lineDir.y
		float t = (E1.y + s * edgeDir.y - P1.y) / lineDir.y;

		//If these give us a point that is within the segments, then there is a collision, if not we must check the other edges
		if (0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f) return true;
	}
	else
	{
		//We will use the third equation to solve for t
		//t = (A.z + s * AB.z - P1.z) / lineDir.z
		//Next we can substitute t into one of the other equations, we will use the first because we know lineDir.x is 0
		//P1.x + (lineDir.x / lineDir.z) * (A.z + s * AB.z - P1.z) = A.x + s * AB.x
		//P1.x = A.x + s * AB.x
		//s = (P1.x - A.x) / AB.x
		float s = (P1.x - E1.x) / edgeDir.x;

		//Solve for t using t = (A.z + s * AB.z - P1.z) / lineDir.z
		float t = (E1.z + s * edgeDir.z - P1.z) / lineDir.z;

		//If these give us a point that is within the segments, then there is a collision, if not we must check the other edges
		if (0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f) return true;
	}

	return false;
}

///
//Detects if two triangles are intersecting when the triangles are Co-Planar
//Utilizes Line segment - Line segment intersection through parametric equations
//and Point - Triangle intersection by computing the direction of the normals of the sub-triangles
//
//Parameters:
//	A1: Point 1 of triangle 1
//	B1: Point 2 of triangle 1
//	C1: Point 3 of triangle 1
//	A2: Point 1 of triangle 2
//	B2: Point 2 of triangle 2
//	C2: Point 3 of triangle 2
//
//Returns:
//	True if intersection is detected, else false.
bool Test2DCase(const glm::vec3 &A1, const glm::vec3 &B1, const glm::vec3 &C1, const glm::vec3 &A2, const glm::vec3 &B2, const glm::vec3 &C2)
{
	//First we must test all edges of Triangle 1 (A1 B1 C1) with edges of triangle 2 (A2, B2, C2) to attempt to find an intersection
	//Check A1B1 with A2B2
	if (CheckEdges(A1, B1, A2, B2)) return true;
	//Check A1B1 with B2C2
	if (CheckEdges(A1, B1, B2, C2)) return true;
	//Check A1B1 with C2A2
	if (CheckEdges(A1, B1, C2, A2)) return true;
	//Check B1C1 with A2B2
	if (CheckEdges(B1, C1, A2, B2)) return true;
	//Check B1C1 with B2C2
	if (CheckEdges(B1, C1, B2, C2)) return true;
	//Check B1C1 with C2A2
	if (CheckEdges(B1, C1, C2, A2)) return true;
	//Check C1A1 with A2B2
	if (CheckEdges(C1, A1, A2, B2)) return true;
	//Check C1A1 with B2C2
	if (CheckEdges(C1, A1, B2, C2)) return true;
	//Check C1A1 with C2A2
	if (CheckEdges(C1, A1, C2, A2)) return true;

	//If we have still not found a collision we must test whether triangle 1 fully contains a point from triangle 2.
	//We can do this by testing if 1 point from triangle 1 is contained in triangle 2 and vice versa
	if (CheckPoint(A1, B1, C1, A2)) return true;
	if (CheckPoint(A2, B2, C2, A1)) return true;

	return false;
}

///
//Tests for collisions between two triangles
//
//Overview:
//	This algorithm tests for collisions between two triangles based on the algorithm
//	outlined in "A Fast Triangle - Triangle Intersection Test" by Tomas Moller ('97)
//	found here:
//		http://web.stanford.edu/class/cs277/resources/papers/Moller1997b.pdf
//
//Parameters:
//	tri1Collider: The first triangle to test
//	tri1ModelMatrix: The first triangle's model to world transformation matrix
//	tri2Collider: The second triangle to test
//	tri2ModelMatrix: The second triangle's model to world transformation matrix
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const struct Triangle &tri1Collider, const glm::mat4 &tri1ModelMatrix, const struct Triangle &tri2Collider, const glm::mat4 &tri2ModelMatrix)
{
	//Step 1: Translate the points of each triangle into worldspace
	glm::vec3 A1 = glm::vec3(tri1ModelMatrix * glm::vec4(tri1Collider.a, 1.0f));
	glm::vec3 B1 = glm::vec3(tri1ModelMatrix * glm::vec4(tri1Collider.b, 1.0f));
	glm::vec3 C1 = glm::vec3(tri1ModelMatrix * glm::vec4(tri1Collider.c, 1.0f));

	glm::vec3 A2 = glm::vec3(tri2ModelMatrix * glm::vec4(tri2Collider.a, 1.0f));
	glm::vec3 B2 = glm::vec3(tri2ModelMatrix * glm::vec4(tri2Collider.b, 1.0f));
	glm::vec3 C2 = glm::vec3(tri2ModelMatrix * glm::vec4(tri2Collider.c, 1.0f));

	//Step 2: Compute the properties for each plane
	//The definition of a plane is:
	//normal . X = d
	//Where d1 = normal . Y for some Y on the plane.

	//We must compute the normals and d constants for both planes
	glm::vec3 AB1 = B1 - A1;
	glm::vec3 AC1 = C1 - A1;

	glm::vec3 AB2 = B2 - A2;
	glm::vec3 AC2 = C2 - A2;

	glm::vec3 normal1 = glm::cross(AB1, AC1);
	glm::vec3 normal2 = glm::cross(AB2, AC2);

	normal1 = glm::normalize(normal1);
	normal2 = glm::normalize(normal2);

	float d1 = -1.0f * glm::dot(normal1, A1);
	float d2 = -1.0f * glm::dot(normal2, A2);

	//Step 3: Compute the signed distances from the plane in which triangle 2 lies to the points of triangle 1 ,
	//And the signed distances from the plane in which triangle 1 lies to the points of triangle 2

	//The signed distances from the plane 2 to triangle 1 can be given as:
	//	distA = normal2 . A1 - d2	//Distance from plane 2 to point A1
	//	distB = normal2 . B1 - d2	//Distance from plane 2 to point B1
	//	distC = normal2 . C1 - d2	//Distance from plane 2 to point C1
	float distA1 = glm::dot(normal2, A1) + d2;
	float distB1 = glm::dot(normal2, B1) + d2;
	float distC1 = glm::dot(normal2, C1) + d2;

	//If all 3 points are on the same side (all 3 points have the same sign) there cannot be a collision.
	if (distA1 < 0.0f && distB1 < 0.0f && distC1 < 0.0f) return false;
	else if (distA1 > 0.0f && distB1 > 0.0f && distC1 > 0.0f) return false;

	//Alternatively, if all 3 points are 0, the planes are coplanar and we must resort to a special 2D case
	if (fabs(distA1) < FLT_EPSILON && fabs(distB1) < FLT_EPSILON && fabs(distC1) < FLT_EPSILON)
	{
		//Special 2D case
		return Test2DCase(A1, B1, C1, A2, B2, C2);
	}

	//Similarly, we can find the distance from plane 1 to triangle 2
	float distA2 = glm::dot(normal1, A2) + d1;
	float distB2 = glm::dot(normal1, B2) + d1;
	float distC2 = glm::dot(normal1, C2) + d1;

	//If all 3 points are on the same side (all 3 points have the same sign) there cannot be a collision.
	if (distA2 < 0.0f && distB2 < 0.0f && distC2 < 0.0f) return false;
	else if (distA2 > 0.0f && distB2 > 0.0f && distC2 > 0.0f) return false;

	//Step 4: Compute the line in which the planes intersect
	//Two planes which intersect in 3D and are not co-planar must form a line.
	//If the two triangles both have points on opposite sides of each opposing plane, it is guaranteed that the
	//line in which the two planes intersect must cross through both triangles!
	//
	//The direction of the line will be in the direction of the cross product of the normals
	glm::vec3 lineDir = glm::cross(normal1, normal2);

	//Step 5: Find the scalar projections of each point onto the line
	//
	//Note: A line by definition is X = O + t * lineDir for some scalar value t,
	//Where O is a point which exists on the line, and the equation solves for X, another point on the line.
	//However, because the whole system can be arbitrarily translated without altering the result of the scalar projections
	//we need not actually calculate O, and we can assume we have arbitrarily translated the entire system to be centered on O.
	//This leaves us with the line definition of X = 0 + t * lineDir
	//or more simply:
	//X = t * lineDir
	//
	//Because the lineDir is already normalized, we can compute the scalar projections of a point P onto the line as:
	//sProjP = P . lineDir
	//
	//There is another optimization- but I will not be using it here. If you are interested: 
	//Because we do not need to know the actual scalar projection values, and we are simply interested in whether or not the
	//intervals of the shapes on this line overlap, by projecting the line onto the closest standard (or principal) coordinate axis
	//We will not alter the results of the test!
	//
	//So instead of using the aforementioned scalar projection formula, we can simply take the X, Y, or Z coordinate of each point as the scalar projection
	//based on the largest dimension of lineDir!
	//Again, please note that this is NOT the method I will be using.

	float sProjA1 = glm::dot(lineDir, A1);
	float sProjB1 = glm::dot(lineDir, B1);
	float sProjC1 = glm::dot(lineDir, C1);
	float sProjA2 = glm::dot(lineDir, A2);
	float sProjB2 = glm::dot(lineDir, B2);
	float sProjC2 = glm::dot(lineDir, C2);

	//Step 6: Determine the outlying point on each triangle
	//During step 3 we determined that on each triangle, 2 points are on one side of the plane, and one point is on the other
	//We must now construct the edges corresponding to the ones which cross the plane.
	//We will construct these edges using the scalar projection values from step 5 rather than the points themselves.
	//The important thing is that we know which scalar projection value pairs correspond to points who's edges cross the plane of the opposing triangle
	//and the distance of the points in these pairs from the opposing plane.
	//This information will tell us which edges intersect with the line created in step 4!

	struct Edge
	{
		float sProj1;	//Scalar projection of first endpoint on edge onto the line
		float dist1;	//Distance of first endpoint from opposing plane
		float sProj2;	//Scalar projection of second endpoint on edge onto the line
		float dist2;	//Distance of second endpoint from opposing plane
	};

	struct Edge tri1E1;		//Edge 1 of triangle 1
	struct Edge tri1E2;		//Edge 2 of triangle 1
	struct Edge tri2E1;		//Edge 1 of triangle 2
	struct Edge tri2E2;		//Edge 2 of triangle 2

	//Tri1 Edges
	if (distA1 < 0.0f)
	{
		if (distB1 > 0.0f)
		{
			if (distC1 > 0.0f)
			{
				tri1E1.sProj1 = sProjA1;
				tri1E1.sProj2 = sProjB1;
				tri1E2.sProj1 = sProjA1;
				tri1E2.sProj2 = sProjC1;

				tri1E1.dist1 = distA1;
				tri1E1.dist2 = distB1;
				tri1E2.dist1 = distA1;
				tri1E2.dist2 = distC1;
			}
			else
			{
				tri1E1.sProj1 = sProjB1;
				tri1E1.sProj2 = sProjA1;
				tri1E2.sProj1 = sProjB1;
				tri1E2.sProj2 = sProjC1;

				tri1E1.dist1 = distB1;
				tri1E1.dist2 = distA1;
				tri1E2.dist1 = distB1;
				tri1E2.dist2 = distC1;
			}
		}
		else
		{
			tri1E1.sProj1 = sProjC1;
			tri1E1.sProj2 = sProjA1;
			tri1E2.sProj1 = sProjC1;
			tri1E2.sProj2 = sProjB1;

			tri1E1.dist1 = distC1;
			tri1E1.dist2 = distA1;
			tri1E2.dist1 = distC1;
			tri1E2.dist2 = distB1;
		}
	}
	else
	{
		if (distB1 < 0.0f)
		{
			if (distC1 < 0.0f)
			{
				tri1E1.sProj1 = sProjA1;
				tri1E1.sProj2 = sProjB1;
				tri1E2.sProj1 = sProjA1;
				tri1E2.sProj2 = sProjC1;

				tri1E1.dist1 = distA1;
				tri1E1.dist2 = distB1;
				tri1E2.dist1 = distA1;
				tri1E2.dist2 = distC1;
			}
			else
			{
				tri1E1.sProj1 = sProjB1;
				tri1E1.sProj2 = sProjA1;
				tri1E2.sProj1 = sProjB1;
				tri1E2.sProj2 = sProjC1;

				tri1E1.dist1 = distB1;
				tri1E1.dist2 = distA1;
				tri1E2.dist1 = distB1;
				tri1E2.dist2 = distC1;
			}
		}
		else
		{
			tri1E1.sProj1 = sProjC1;
			tri1E1.sProj2 = sProjA1;
			tri1E2.sProj1 = sProjC1;
			tri1E2.sProj2 = sProjB1;

			tri1E1.dist1 = distC1;
			tri1E1.dist2 = distA1;
			tri1E2.dist1 = distC1;
			tri1E2.dist2 = distB1;
		}
	}

	//Tri2 Edges
	if (distA2 < 0.0f)
	{
		if (distB2 > 0.0f)
		{
			if (distC2 > 0.0f)
			{
				tri2E1.sProj1 = sProjA2;
				tri2E1.sProj2 = sProjB2;
				tri2E2.sProj1 = sProjA2;
				tri2E2.sProj2 = sProjC2;

				tri2E1.dist1 = distA2;
				tri2E1.dist2 = distB2;
				tri2E2.dist1 = distA2;
				tri2E2.dist2 = distC2;
			}
			else
			{
				tri2E1.sProj1 = sProjB2;
				tri2E1.sProj2 = sProjA2;
				tri2E2.sProj1 = sProjB2;
				tri2E2.sProj2 = sProjC2;

				tri2E1.dist1 = distB2;
				tri2E1.dist2 = distA2;
				tri2E2.dist1 = distB2;
				tri2E2.dist2 = distC2;
			}
		}
		else
		{
			tri2E1.sProj1 = sProjC2;
			tri2E1.sProj2 = sProjA2;
			tri2E2.sProj1 = sProjC2;
			tri2E2.sProj2 = sProjB2;

			tri2E1.dist1 = distC2;
			tri2E1.dist2 = distA2;
			tri2E2.dist1 = distC2;
			tri2E2.dist2 = distB2;
		}
	}
	else
	{
		if (distB2 < 0.0f)
		{
			if (distC2 < 0.0f)
			{
				tri2E1.sProj1 = sProjA2;
				tri2E1.sProj2 = sProjB2;
				tri2E2.sProj1 = sProjA2;
				tri2E2.sProj2 = sProjC2;

				tri2E1.dist1 = distA2;
				tri2E1.dist2 = distB2;
				tri2E2.dist1 = distA2;
				tri2E2.dist2 = distC2;
			}
			else
			{
				tri2E1.sProj1 = sProjB2;
				tri2E1.sProj2 = sProjA2;
				tri2E2.sProj1 = sProjB2;
				tri2E2.sProj2 = sProjC2;

				tri2E1.dist1 = distB2;
				tri2E1.dist2 = distA2;
				tri2E2.dist1 = distB2;
				tri2E2.dist2 = distC2;
			}
		}
		else
		{
			tri2E1.sProj1 = sProjC2;
			tri2E1.sProj2 = sProjA2;
			tri2E2.sProj1 = sProjC2;
			tri2E2.sProj2 = sProjB2;

			tri2E1.dist1 = distC2;
			tri2E1.dist2 = distA2;
			tri2E2.dist1 = distC2;
			tri2E2.dist2 = distB2;
		}
	}

	//Step 7: Determine the parametric scalar values t1, t2, s1, and s2 determining when edge 1 of triangle 1, edge 2 of triangle 1,
	//edge 1 of triangle 2, and edge 2 of triangle 2 (respectively) intersect with the line L.
	//
	//As it turns out, the triangle created by:
	//	the first vertex on an edge, it's projection onto the opposing plane, and the point of intersection of it's edge with L
	//and the triangle created by
	//	the second vertex on an edge, it's projection onto the opposing plane, and the point of intersection of the edge with L
	//are similar triangles.
	//
	//Therefore we can solve for the parametric scalar value for where triXEY intersects line L using the following formula:
	//t = triXEY.sProj1 + (triXEY.sProj2 - triXEY.sProj1) * (triXEY.dist1 / (triXEY.dist1 - triXEY.dist2))
	float t1 = tri1E1.sProj1 + (tri1E1.sProj2 - tri1E1.sProj1) * (tri1E1.dist1 / (tri1E1.dist1 - tri1E1.dist2));
	float t2 = tri1E2.sProj1 + (tri1E2.sProj2 - tri1E2.sProj1) * (tri1E2.dist1 / (tri1E2.dist1 - tri1E2.dist2));
	float s1 = tri2E1.sProj1 + (tri2E1.sProj2 - tri2E1.sProj1) * (tri2E1.dist1 / (tri2E1.dist1 - tri2E1.dist2));
	float s2 = tri2E2.sProj1 + (tri2E2.sProj2 - tri2E2.sProj1) * (tri2E2.dist1 / (tri2E2.dist1 - tri2E2.dist2));

	//Find the min1, max1, min2, and max2
	float min1 = t1 < t2 ? t1 : t2;
	float max1 = min1 == t1 ? t2 : t1;

	float min2 = s1 < s2 ? s1 : s2;
	float max2 = min2 == s1 ? s2 : s1;

	//Step 8: Determine if the scalar values overlap
	if (max1 > min2 && min1 < max2) return true;

	return false;
}

// This runs once every physics timestep.
void update()
{

	//Check if the mouse button is being pressed
	if (isMousePressed)
	{
		//Get the current mouse position
		double currentMouseX, currentMouseY;
		glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

		//Get the difference in mouse position from last frame
		float deltaMouseX = (float)(currentMouseX - prevMouseX);
		float deltaMouseY = (float)(currentMouseY - prevMouseY);

		glm::mat4 yaw;
		glm::mat4 pitch;

		//Rotate the selected shape by an angle equal to the mouse movement
		if (deltaMouseX != 0.0f)
		{
			yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		if (deltaMouseY != 0.0f)
		{
			pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));

		}

		selectedShape->rotation = yaw * pitch * selectedShape->rotation;

		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

	}

	if (TestCollision(*triangle1Collider, triangle1->GetModelMatrix(), *triangle2Collider, triangle2->GetModelMatrix()))
	{
		//Turn red on
		hue[0][0] = 1.0f;
	}
	else
	{
		//Turn red off
		hue[0][0] = 0.0f;
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
	triangle1->Draw();
	triangle2->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == triangle1 ? triangle2 : triangle1;

		//This set of controls are used to move the selectedShape.
		if (key == GLFW_KEY_W)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, movementSpeed, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_A)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-movementSpeed, 0.0f, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_S)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -movementSpeed, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_D)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(movementSpeed, 0.0f, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_LEFT_CONTROL)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, movementSpeed)) * selectedShape->translation;
		if (key == GLFW_KEY_LEFT_SHIFT)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -movementSpeed)) * selectedShape->translation;
	}

}

///
//Inturrupt triggered by mouse buttons
//
//Parameters:
//	window: The window which recieved the mouse click event
//	button: The mouse button which was pressed
//	action: GLFW_PRESS or GLFW_RELEASE
//	mods: The modifier keys which were pressed during the mouse click event
void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	//Set the boolean indicating whether or not the mouse is pressed
	isMousePressed = button == GLFW_MOUSE_BUTTON_LEFT ?
		(action == GLFW_PRESS ? true : false)
		: false;

	//Update the previous mouse position
	glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
}

#pragma endregion util_Functions


void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Triangle - Triangle Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();



	//Generate the triangle1 mesh
	struct Vertex triVerts[3] =
	{
		{ -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f }
	};
	//Triangle1
	triangle1 = new struct Mesh(3, triVerts, GL_TRIANGLES);

	//Translate the triangle
	triangle1->translation = glm::translate(triangle1->translation, glm::vec3(0.15f, 0.0f, 0.0f));

	//Scale the triangle
	triangle1->scale = glm::scale(triangle1->scale, glm::vec3(0.1f));

	//Rotate the triangle
	//triangle-1>rotation = glm::rotate(triangle1->rotation, 60.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	//Generate triangle2 mesh
	triVerts[0].b = triVerts[1].b = triVerts[2].b = 0.0f;
	triVerts[0].g = triVerts[1].g = triVerts[2].g = 1.0f;

	//Triangle2
	triangle2 = new struct Mesh(3, triVerts, GL_TRIANGLES);

	//Translate the triangle
	triangle2->translation = glm::translate(triangle2->translation, glm::vec3(-0.15f, 0.0f, 0.0f));

	//scale the triangle
	triangle2->scale = glm::scale(triangle2->scale, glm::vec3(0.1f));

	//Set the selected shape
	selectedShape = triangle1;

	//Generate triangle1 collider
	triangle1Collider = new struct Triangle(
		glm::vec3(triVerts[0].x, triVerts[0].y, triVerts[0].z),
		glm::vec3(triVerts[1].x, triVerts[1].y, triVerts[1].z),
		glm::vec3(triVerts[2].x, triVerts[2].y, triVerts[2].z)
		);

	//Generate triangle2 collider
	triangle2Collider = new struct Triangle(
		glm::vec3(triVerts[0].x, triVerts[0].y, triVerts[0].z),
		glm::vec3(triVerts[1].x, triVerts[1].y, triVerts[1].z),
		glm::vec3(triVerts[2].x, triVerts[2].y, triVerts[2].z)
		);

	//Print controls
	std::cout << "Use WASD to move the selected shape in the XY plane.\nUse left CTRL & left shift to move the selected shape along Z axis.\n";
	std::cout << "Left click and drag the mouse to rotate the selected shape.\nUse spacebar to swap the selected shape.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

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

	delete triangle1;
	delete triangle2;

	delete triangle1Collider;
	delete triangle2Collider;

	// Frees up GLFW memory
	glfwTerminate();
}