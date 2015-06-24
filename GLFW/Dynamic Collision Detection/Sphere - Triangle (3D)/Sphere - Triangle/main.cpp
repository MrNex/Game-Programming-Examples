/*
Title: Sphere - Triangle (3D)
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
The demo contains two a pink moving sphere, and a yellow moving triangle.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving shapes to move any further.
If a moving shape reaches the side of the screen, it will wrap around to the other side again.

The user can disable collision detection by holding spacebar.

Uses a plethora of different algorithms to detect collision between a sphere and a triangle, including:
Line segment - Sphere
Line segment - Cylinder
Point - Triangle
and Sphere - Point

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson

*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;
GLuint uniHue;

// Matrix for storing the View Projection transformation
glm::mat4 VP;

// This is a matrix to be sent to the shaders which can control global hue alteration
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

struct Line
{
	glm::vec3 start;
	glm::vec3 direction;
};

struct Cylinder
{
	glm::vec3 start;
	glm::vec3 direction;
	float radius;
};

//Struct for sphere collider
struct Sphere
{
	float radius;
	glm::vec3 center;

	//Default constructor, creates unit sphere at origin
	Sphere::Sphere()
	{
		center = glm::vec3(0.0f);
		radius = 1.0f;
	}

	//PArameterized constructor, creates sphere from given center and radius
	Sphere::Sphere(const glm::vec3& c, float r)
	{
		center = c;
		radius = r;
	}
};

//Struct for triangle collider
struct Triangle
{
	glm::vec3 center;
	glm::vec3 a, b, c;

	//Default constructor, creates basic triangle positioned at origin
	Triangle::Triangle()
	{
		center = glm::vec3(0.0f);
		a = glm::vec3(-1.0f, -1.0f, 0.0f);
		b = glm::vec3(1.0f, -1.0f, 0.0f);
		c = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	//Parameterized constructor, creates triangle with 3 points and given center
	Triangle::Triangle(const glm::vec3& pos, const glm::vec3& A, const glm::vec3 &B, const glm::vec3 &C)
	{
		center = pos;

		a = A;
		b = B;
		c = C;
	}
};

struct Mesh* sphere;
struct Mesh* triangle;

struct RigidBody* sphereBody;
struct RigidBody* triangleBody;

struct Sphere* sphereCollider;
struct Triangle* triangleCollider;


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.5; // This is the number of milliseconds we intend for the physics to update.


#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Read shader source
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

//Creates shader program
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

	// Create shaders
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

	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

///
//Generates a sphere mesh with a given radius
void GenerateSphereMesh(float radius, int subdivisions)
{
	std::vector<Vertex> vertexSet;

	float pitch, yaw;
	yaw = 0.0f;
	pitch = 0.0f;
	int i, j;
	float pitchDelta = 360.0f / subdivisions;
	float yawDelta = 360.0f / subdivisions;

	float PI = 3.14159f;

	Vertex p1, p2, p3, p4;

	for (i = 0; i < subdivisions; i++)
	{
		for (j = 0; j < subdivisions; j++)
		{

			p1.x = radius * sin((pitch)* PI / 180.0f) * cos((yaw)* PI / 180.0f);
			p1.y = radius * sin((pitch)* PI / 180.0f) * sin((yaw)* PI / 180.0f);
			p1.z = radius * cos((pitch)* PI / 180.0f);
			p1.r = 1.0f;
			p1.g = 0.0f;
			p1.b = 1.0f;
			p1.a = 1.0f;

			p2.x = radius * sin((pitch)* PI / 180.0f) * cos((yaw + yawDelta)* PI / 180.0f);
			p2.y = radius * sin((pitch)* PI / 180.0f) * sin((yaw + yawDelta)* PI / 180.0f);
			p2.z = radius * cos((pitch)* PI / 180.0f);
			p2.r = 1.0f;
			p2.g = 0.0f;
			p2.b = 1.0f;
			p2.a = 1.0f;

			p3.x = radius * sin((pitch + pitchDelta)* PI / 180.0f) * cos((yaw + yawDelta)* PI / 180.0f);
			p3.y = radius * sin((pitch + pitchDelta)* PI / 180.0f) * sin((yaw + yawDelta)* PI / 180.0f);
			p3.z = radius * cos((pitch + pitchDelta)* PI / 180.0f);
			p3.r = 1.0f;
			p3.g = 0.0f;
			p3.b = 1.0f;
			p3.a = 1.0f;

			p4.x = radius * sin((pitch + pitchDelta)* PI / 180.0f) * cos((yaw)* PI / 180.0f);
			p4.y = radius * sin((pitch + pitchDelta)* PI / 180.0f) * sin((yaw)* PI / 180.0f);
			p4.z = radius * cos((pitch + pitchDelta)* PI / 180.0f);
			p4.r = 1.0f;
			p4.g = 0.0f;
			p4.b = 1.0f;
			p4.a = 1.0f;

			vertexSet.push_back(p1);
			vertexSet.push_back(p2);
			vertexSet.push_back(p2);
			vertexSet.push_back(p3);
			vertexSet.push_back(p3);
			vertexSet.push_back(p4);
			vertexSet.push_back(p4);
			vertexSet.push_back(p1);

			yaw = yaw + yawDelta;
		}

		pitch += pitchDelta;
	}

	sphere = new Mesh(vertexSet.size(), &vertexSet[0], GL_LINES);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions
///
//Checks if two line segments are intersecting
//See Line Segment - Line Segment (2D) for explanation of algorithm
//
//Parameters:
//	start1: the starting endpoint of segment 1
//	dir1: The direction (and magnitude) of segment 1
//	start2: The starting endpoint of segment 2
//	dir2: The direction (and magnitude) of segment 2
//
//Returns:
//	The time of collision, -1.0f if no collision occurred
float CheckCylinderSegmentCollision(const struct Cylinder &cylinder, const struct Line &line)
{
	float t;

	//Get a vector from the cylinder to the line
	glm::vec3 cylinderToLine = line.start - cylinder.start;

	//Take dot product of vector from cylinder to line and the direction of cylinder
	float angleBetween = glm::dot(cylinderToLine, cylinder.direction);
	float angleBetweenDirections = glm::dot(cylinder.direction, line.direction);
	float cylDirMagSq = glm::dot(cylinder.direction, cylinder.direction);

	//segment is fully outside of either endcap
	if (angleBetween < 0.0f && angleBetween + angleBetweenDirections < 0.0f) return -1.0f;
	if (angleBetween > cylDirMagSq  && angleBetween + angleBetweenDirections > cylDirMagSq) return -1.0f;

	float lineDirMagSq = glm::dot(line.direction, line.direction);
	float otherAngleBetween = glm::dot(cylinderToLine, line.direction);

	float a = cylDirMagSq * lineDirMagSq - angleBetweenDirections * angleBetweenDirections;
	float k = glm::dot(cylinderToLine, cylinderToLine) - cylinder.radius * cylinder.radius;
	float c = cylDirMagSq * k - angleBetween * angleBetween;

	if (fabs(a) < FLT_EPSILON)
	{
		if (c > 0.0f) return -1.0f;
		if (angleBetween < 0.0f) t = -otherAngleBetween / lineDirMagSq;
		else if (angleBetween > cylDirMagSq) t = (angleBetweenDirections - otherAngleBetween);
		else t = 0.0f;
		return t;
	}

	float b = cylDirMagSq * otherAngleBetween - angleBetweenDirections * angleBetween;
	float discr = b * b - a * c;
	if (discr < 0.0f) return 0;	//No real roots
	t = (-b - sqrt(discr)) / a;
	if (t < 0.0f || t > 1.0f) return -1.0f;
	if (angleBetween + t * angleBetweenDirections < 0.0f) return -1.0f;
	if (angleBetween + t * angleBetweenDirections > cylDirMagSq) return -1.0f;
	return t;


}

///
//Checks if a sphere and a line segment are colliding
//
//PArameters:
//	s: The sphere
//	lineStart: The starting endpoint of the segment
//	lineDir: The direction and magnitude of the segment
//
//Returns:
//	The time of collision, -1.0f if no collision occurred
float checkSphereLineSegmentCollision(const Sphere &s, const glm::vec3 &lineStart, const glm::vec3 &lineDir)
{
	//Position everything relative to the line being at the origin
	glm::vec3 spherePos = s.center - lineStart;

	//Project the circle center onto the line direction
	float projMag = glm::dot(spherePos, lineDir) / glm::dot(lineDir, lineDir);
	float ratio = s.radius / glm::length(lineDir);
	//Make sure the circle center is close enough to the line segment ends to possibly collide
	if (projMag < -ratio || projMag > 1.0f + ratio) return -1.0f;

	glm::vec3 projPos = projMag * lineDir;
	//Find the distance the circle is away from the line segment
	float dist = glm::length(spherePos - projPos);
	if (dist < s.radius)
	{
		return projMag - ratio;
	}
	return -1.0f;
}

///
//Detects collision between a point and a triangle
//See Point - Triangle (Normal Method) for explanation
//
//Parameters:
//	tri: The triangle to test
//	point: the point to test
bool checkPointTriangleCollision(const Triangle &tri, const glm::vec3 &point)
{
	glm::vec3 PA = (tri.center + tri.a) - point;
	glm::vec3 AB = (tri.center + tri.a) - (tri.center + tri.b);
	glm::vec3 PABNormal = glm::cross(PA, AB);

	glm::vec3 PB = (tri.center + tri.b) - point;
	glm::vec3 BC = (tri.center + tri.b) - (tri.center + tri.c);
	glm::vec3 PBCNormal = glm::cross(PB, BC);
	if (glm::dot(PABNormal, PBCNormal) < 0.0f) return false;

	glm::vec3 PC = (tri.center + tri.c) - point;
	glm::vec3 CA = (tri.center + tri.c) - (tri.center + tri.a);
	glm::vec3 PCANormal = glm::cross(PC, CA);
	if (glm::dot(PABNormal, PCANormal) < 0.0f) return false;

	return true;

}

//Function to get the closest point on a triangle, to a given point.
glm::vec3 ClosestPointTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 bc = c - b;

	// Compute parametric position s for projection p' of P on AB,
	// P' = A + s*AB, s = snom/(snom + sdenom)
	float snom = glm::dot(p - a, ab), sdenom = glm::dot(p - b, a - b);

	// Compute parametric position s for projection P' of P on AC,
	// P' = A + s*AC, s = tnom/(tnom + tdenom)
	float tnom = glm::dot(p - a, ac), tdenom = glm::dot(p - c, a - c);

	if (snom <= 0.0f && tnom <= 0.0f) return a;		//vertex region early out.

	// Compute parametric position u for projection P' of P on BC,
	// P' = B + u*BC, u = unom/(unom + udenom)
	float unom = glm::dot(p - b, bc), udenom = glm::dot(p - c, b - c);

	if (sdenom <= 0.0f && unom <= 0.0f) return b;	//vertex region early out.
	if (tdenom <= 0.0f && udenom <= 0.0f) return c;	//vertex region early out.

	// P is outside (or on) AB if the triple scalar product [N PA PB] <= 0
	glm::vec3 n = glm::cross(b - a, c - a);
	float vc = glm::dot(n, glm::cross(a - p, b - p));

	// if P is outside AB and within feature region of AB
	// return projection of P onto AB
	if (vc < 0.0f && snom >= 0.0f && sdenom >= 0.0f)
		return a + snom / (snom + sdenom) * ab;

	// P is outside (or on) BC if the triple scalar product [N PA PB] <= 0
	float va = glm::dot(n, glm::cross(b - c, c - p));

	//if P outside BC and within feature region of BC,
	// return projection of P onto BC
	if (va <= 0.0f && unom >= 0.0f && udenom >= 0.0f)
		return b + unom / (unom + udenom) * bc;

	// P is outside (or on) CA if the triple scalar product [N PC PA] <= 0
	float vb = glm::dot(n, glm::cross(c - p, a - p));

	// if P outside CA and within feature region of CA,
	// return projection of P onto CA
	if (vb <= 0.0f && tnom >= 0.0f && tdenom >= 0.0f)
	{
		return a + tnom / (tnom + tdenom) * ac;
	}

	//p must project inside face region, Compute Q using barycentric coordinates
	float u = va / (va + vb + vc);
	float v = vb / (va + vb + vc);
	float w = 1.0f - u - v;		// = vc / (va + vb + vc)

	return u*a + v*b + w*c;
}

///
//Performs a dynamic collision check between a moving sphere and a triangle
//
//Overview:
//	Uses a plethora of different algorithms to detect collision between a sphere and a triangle, including:
//	Line segment - Sphere
//	Line segment - Cylinder
//	Point - Triangle
//	and Sphere - Point
//
//Parameters:
//	s: The moving sphere
//	tri: The static triangle
//	mvmt: The relative movement vector of sphere from an observer on triangle
//	tStart: The start of the interval, 0.0f <= tStart < 1.0f
//	tEnd: The end of the interval, 0.0f < tEnd <= 1.0f
//
//Returns:
//	a t value between 0 and 1 indicating the "relative time" since the start of this frame that the collision occurred.
//	a t value of 0.0f would indicate the very start of this frame, a t value of 1.0f would indicate the very end of this frame.
//	This function will return a negative number if no collision was registered.
float CheckDynamicCollision(const struct Sphere& s, const Triangle &tri, const glm::vec3 &mvmt, float tStart, float tEnd)
{
	//Get the three edge vectors of the triangle
	glm::vec3 AB = tri.b - tri.a;
	glm::vec3 BC = tri.c - tri.b;
	glm::vec3 CA = tri.a - tri.c;

	//Get normal of plane which triangle lies in
	glm::vec3 triangleNormal = glm::normalize(glm::cross(AB, -CA));

	//Determine if sphere is travelling parallel to triangle plane
	if (fabs(glm::dot(mvmt, triangleNormal)) > FLT_EPSILON)
	{
		//Not travelling parallel to plane
	
		//Find the point on the sphere which will hit the triangle first
		glm::vec3 pointOnSphere = s.center;
		//Determine which side of plane triangle is on
		glm::vec3 triToSphere = s.center - tri.center;
		if (glm::dot(triToSphere, triangleNormal) < 0.0f)
		{
			pointOnSphere += s.radius * triangleNormal;
		}
		else
		{
			pointOnSphere -= s.radius * triangleNormal;
		}

		//Consider the line segment this point creates over the movement vector
		struct Line circleMvmt;
		circleMvmt.start = pointOnSphere;
		circleMvmt.direction = mvmt;

		//Find the time which this line segment collides with the triangle's plane
		float dist = glm::dot(triangleNormal, tri.a);
		float t = (dist - glm::dot(triangleNormal, circleMvmt.start)) / glm::dot(triangleNormal, circleMvmt.direction);
		
		//If t is not within this timestep, there cannot be a collision
		if (t < 0.0f || t > 1.0f) return -1.0f;

		//Else, determine the point along the line which collides with the plane
		glm::vec3 pointOfPossibleCollision = circleMvmt.start + t * circleMvmt.direction;

		//If this point is contained within the triangle there is a collision
		if (checkPointTriangleCollision(tri, pointOfPossibleCollision)) return t;

		//Else, we can still have a collision. We must raycast from the closest point on the triangle to the sphere along -mvmt and see if it hits.
		//Determine the closest point on triangle t to sphere
		glm::vec3 closestPoint = ClosestPointTriangle(s.center, tri.a, tri.b, tri.c);
		struct Line ray;
		ray.start = closestPoint;
		ray.direction = -circleMvmt.direction;

		return checkSphereLineSegmentCollision(s, ray.start, ray.direction);
	}
	else
	{
		//Travelling parallel to plane

		//Create a line segment which represents the path travelled by the center of the circle
		struct Line circleMvmt;
		circleMvmt.start = s.center;
		circleMvmt.direction = mvmt;

		//Create A cylinder at each triangle edge with a radius of the sphere
		struct Cylinder cyl;
		cyl.start = tri.a + tri.center;
		cyl.direction = AB;
		cyl.radius = s.radius;

		//Determine if the sphere's movement line segment intersects the cylinder
		float minTimeOfIntersection = CheckCylinderSegmentCollision(cyl, circleMvmt);

		//Cylinder along BC
		cyl.start = tri.b + tri.center;
		cyl.direction = BC;

		//Determine if the sphere's movement line segment intersects the cylinder
		float timeOfIntersection = CheckCylinderSegmentCollision(cyl, circleMvmt);
		if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

		//Cylinder along CA
		cyl.start = tri.b + tri.center;
		cyl.direction = CA;


		//Determine if the sphere's movement line segment intersects the cylinder
		timeOfIntersection = CheckCylinderSegmentCollision(cyl, circleMvmt);
		if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

		//Create spheres at each one of the triangle vertices matching the circle
		Sphere vertexSphere;
		vertexSphere.radius = s.radius;

		vertexSphere.center = tri.center + tri.a;
		timeOfIntersection = checkSphereLineSegmentCollision(vertexSphere, circleMvmt.start, circleMvmt.direction);
		if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

		vertexSphere.center = tri.center + tri.b;
		timeOfIntersection = checkSphereLineSegmentCollision(vertexSphere, circleMvmt.start, circleMvmt.direction);
		if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

		vertexSphere.center = tri.center + tri.c;
		timeOfIntersection = checkSphereLineSegmentCollision(vertexSphere, circleMvmt.start, circleMvmt.direction);
		if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

		return minTimeOfIntersection;
	}
	
}


// This runs once every physics timestep.
void update(float dt)
{
	float t = 1.0f;

	//If the user presses spacebar, use non-continuous collision detection
	if (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS)
	{
		//Determine relative velocity of circle 1 from circle 2
		glm::vec3 relV = sphereBody->velocity - triangleBody->velocity;
		t = CheckDynamicCollision(*sphereCollider, *triangleCollider, relV * dt, 0.0f, 1.0f);

		//If there is no collision, move the entire way.
		if (t == -1.0f)
		{
			t = 1.0f;

		}
	}

	//Move the spherebody
	sphereBody->position += sphereBody->velocity * dt * t;
	triangleBody->position += triangleBody->velocity * dt * t;


	//Move collider
	sphereCollider->center = sphereBody->position;
	triangleCollider->center = triangleBody->position;

	//Once we have solved for the bodys position, we should translate the mesh to match it
	sphere->translation = glm::translate(glm::mat4(1.0f), sphereBody->position);
	triangle->translation = glm::translate(glm::mat4(1.0f), triangleBody->position);

	//If the position goes off of the right edge of the screen, loop it back to the left
	if (sphereBody->position.x > 1.0f)
	{

		sphereBody->position.x = -1.0f;
		sphereCollider->center = sphereBody->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		sphere->translation = glm::translate(glm::mat4(1.0f), sphereBody->position);


	}
	if (triangleBody->position.x < -1.0f)
	{
		triangleBody->position.x = 1.0f;
		triangleCollider->center = triangleBody->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		triangle->translation = glm::translate(glm::mat4(1.0f), triangleBody->position);
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

		timebase = time; // set new last updated time

		// Limit dt
		if (dt > 0.25)
		{
			dt = 0.25;
		}
		accumulator += dt;

		// Update physics necessary amount
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
	sphere->Draw();
	triangle->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Sphere - Triangle (3D Dynamic Collision Detection)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate the sphere mesh
	float scale = 0.1f;
	
	GenerateSphereMesh(1.0f, 40);

	//Generate the triangle mesh
	struct Vertex triVerts[3] = 
	{
		{-1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}
	};

	//triangle creation
	triangle = new struct Mesh(3, triVerts, GL_TRIANGLES);


	//Scale the shapes
	sphere->scale = glm::scale(sphere->scale, glm::vec3(scale));
	triangle->scale = glm::scale(triangle->scale, glm::vec3(scale));


	//Generate the rigidbodies
	sphereBody = new struct RigidBody(
		glm::vec3(-1.0f, 0.0f, 0.0f),		//Start on left side of screen
		glm::vec3(1.0f, 0.0f, 0.0f),		//constant right velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	triangleBody = new struct RigidBody(
		glm::vec3(0.75f, 0.0f, 0.0f),		//Start on right side
		glm::vec3(-0.5f, 0.0f, 0.0f),		//Constant left velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	//Position shapes
	sphere->translation = glm::translate(sphere->translation, sphereBody->position);
	triangle->translation = glm::translate(triangle->translation, triangleBody->position);

	//Generate the colliders
	sphereCollider = new Sphere(sphereBody->position, scale);
	triangleCollider = new Triangle(triangleBody->position, 
		scale * glm::vec3(triVerts[0].x, triVerts[0].y, triVerts[0].z), scale * glm::vec3(triVerts[1].x, triVerts[1].y, triVerts[1].z), scale * glm::vec3(triVerts[2].x, triVerts[2].y, triVerts[2].z));

	//Print controls
	std::cout << "Controls:\nPress and hold spacebar to disable continuous collision detection.\nWhen two shapes collide, continue the simulation by toggling continuous collision detection off.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		//Check time will update the programs clock and determine if & how many times the physics must be updated
		checkTime();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	delete sphere;
	delete triangle;
	delete sphereBody;
	delete triangleBody;
	delete sphereCollider;
	delete triangleCollider;

	// Frees up GLFW memory
	glfwTerminate();
}