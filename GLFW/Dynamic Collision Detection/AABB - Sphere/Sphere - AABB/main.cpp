/*
Title: Sphere - AABB
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
The demo contains two a pink moving sphere, and a yellow moving cube.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving shapes to move any further.
If a moving shape reaches the side of the screen, it will wrap around to the other side again.

The user can disable collision detection by holding spacebar.

This algorithm uses a modified version of the Separating Axis test to detect collision.
It is worth noting that this demo does not resolve the collision to the point of contact,
however as stated it will detect dynamic collision to prevent tunnelling.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
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

//Struct for AABB collider
struct AABB
{
	glm::vec3 center;
	glm::vec3 dimensions;

	//Default constructor, creates basic triangle positioned at origin
	AABB::AABB()
	{
		center = glm::vec3(0.0f);
		dimensions = glm::vec3(2.0f, 2.0f, 2.0f);
	}

	//Parameterized constructor, creates triangle with 3 points and given center
	AABB::AABB(const glm::vec3& pos, const glm::vec3 &dim)
	{
		center = pos;

		dimensions = dim;
	}
};

struct Mesh* sphere;
struct Mesh* cube;

struct RigidBody* sphereBody;
struct RigidBody* cubeBody;

struct Sphere* sphereCollider;
struct AABB* cubeCollider;


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
//Performs a dynamic collision check between a moving sphere and an axis aligned bounding box
//
//Overview:
//	Performs a modified version of the Dynamic Separating Axis Test as shown in
//	the dynamic Convex Polyhedron - Convex Polyhedron collision detection test.
//	this modified version helps the algorithm work efficiently with spheres.
//
//Parameters:
//	s: The moving sphere
//	box: The static AABB
//	mvmt: The relative movement vector of sphere from an observer on the AABB
//	tStart: The start of the interval, 0.0f <= tStart < 1.0f
//	tEnd: The end of the interval, 0.0f < tEnd <= 1.0f
//
//Returns:
//	a t value between 0 and 1 indicating the "relative time" since the start of this frame that the collision occurred.
//	a t value of 0.0f would indicate the very start of this frame, a t value of 1.0f would indicate the very end of this frame.
//	This function will return a negative number if no collision was registered.
float CheckDynamicCollision(const struct Sphere& sphere, const AABB &box, const glm::vec3 &mvmt, float tStart, float tEnd)
{
	float tFirst = 0.0f;	//First time of collision
	float tLast = 1.0f;	//Last time of collision
	float tCurrent = 0.0f;	//Time of collision for current test

	//Get the points of the AABB in worldspace
	std::vector<glm::vec3> aabbPts;
	aabbPts.push_back(box.center + box.dimensions / 2.0f);
	aabbPts.push_back(glm::vec3(box.center.x - box.dimensions.x / 2.0f, box.center.y + box.dimensions.y / 2.0f, box.center.z + box.dimensions.z / 2.0f));
	aabbPts.push_back(glm::vec3(box.center.x + box.dimensions.x / 2.0f, box.center.y - box.dimensions.y / 2.0f, box.center.z + box.dimensions.z / 2.0f));
	aabbPts.push_back(glm::vec3(box.center.x + box.dimensions.x / 2.0f, box.center.y + box.dimensions.y / 2.0f, box.center.z - box.dimensions.z / 2.0f));
	aabbPts.push_back(glm::vec3(box.center.x - box.dimensions.x / 2.0f, box.center.y - box.dimensions.y / 2.0f, box.center.z + box.dimensions.z / 2.0f));
	aabbPts.push_back(glm::vec3(box.center.x - box.dimensions.x / 2.0f, box.center.y + box.dimensions.y / 2.0f, box.center.z - box.dimensions.z / 2.0f));
	aabbPts.push_back(glm::vec3(box.center.x + box.dimensions.x / 2.0f, box.center.y - box.dimensions.y / 2.0f, box.center.z - box.dimensions.z / 2.0f));
	aabbPts.push_back(box.center - box.dimensions / 2.0f);
	
	//Create a list of the AABB normals
	std::vector<glm::vec3> aabbNorms;					//Do note: for an AABB, these are also the edges!
	aabbNorms.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
	aabbNorms.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	aabbNorms.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

	//For every AABB axis
	int nSize = aabbNorms.size();
	for (int i = 0; i < nSize; i++)
	{
		glm::vec3 currentNorm = aabbNorms[i];

		//Determine the projection bounds of aabb on this axis
		float min1 = box.center[i] - box.dimensions[i] / 2.0f;
		float max1 = box.center[i] + box.dimensions[i] / 2.0f;

		//Determine the bounds of sphere on this axis
		std::vector<glm::vec3> spherePoints;
		spherePoints.push_back(sphere.center + sphere.radius * currentNorm);
		spherePoints.push_back(sphere.center - sphere.radius * currentNorm);
		spherePoints.push_back(sphere.center + mvmt + sphere.radius * currentNorm);
		spherePoints.push_back(sphere.center + mvmt - sphere.radius * currentNorm);
		int pSize = spherePoints.size();
		float min2, max2;
		min2 = max2 = glm::dot(currentNorm, spherePoints[0]);
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(currentNorm, spherePoints[j]);
			if (sProj < min2) min2 = sProj;
			if (sProj > max2) max2 = sProj;
		}

		//Is there overlap on this axis?
		if (min1 <= max2 && max1 >= min2)
		{
			//Order the values from least to greatest (lBound < lMid < uMid < uBound)
			float uMid, lMid;
			float uBound = max1 > max2 ? (uMid = max2, max1) : (uMid = max1, max2);
			float lBound = min1 < min2 ? (lMid = min2, min1) : (lMid = min1, min2);

			//Determine scalar projection of mvmt on this axis
			float sProjMvmt = glm::dot(mvmt, currentNorm);
			//Check the direction
			if (sProjMvmt < -FLT_EPSILON)
			{
				if (max1 < min2) return -1.0f;

				//uMid is first point of intersection
				tCurrent = (uMid - lBound) / sProjMvmt;
				if (tCurrent > tFirst) tFirst = tCurrent;

				//lMid is last point of intersection
				tCurrent = (lMid - lBound) / sProjMvmt;
				if (tCurrent < tLast) tLast = tCurrent;
			}
			else if (sProjMvmt > FLT_EPSILON)
			{
				if (min1 > max2) return -1.0f;

				//lMid is first point of intersection
				tCurrent = (lMid - lBound) / sProjMvmt;
				if (tCurrent > tFirst) tFirst = tCurrent;

				//uMid is last point of intersection
				tCurrent = (uMid - lBound) / sProjMvmt;
				if (tCurrent < tLast) tLast = tCurrent;
			}

		}
		else
		{
			return -1.0f;	//No overlap
		}
	}

	//Next we must test the axes representing the cross product of the edges
	for (int i = 0; i < nSize; i++)
	{
		glm::vec3 currentNorm = glm::cross(aabbNorms[i], glm::normalize(mvmt));
		int pSize = aabbPts.size();
		//Determine the projection bounds of aabb on this axis
		float min1, max1;
		min1 = max1 = glm::dot(currentNorm, aabbPts[0]);
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(currentNorm, aabbPts[j]);
			if (sProj < min1) min1 = sProj;
			if (sProj > max1) max1 = sProj;
		}
		

		//Determine the bounds of sphere on this axis
		std::vector<glm::vec3> spherePoints;
		spherePoints.push_back(sphere.center + sphere.radius * currentNorm);  
		spherePoints.push_back(sphere.center - sphere.radius * currentNorm);
		spherePoints.push_back(sphere.center + mvmt + sphere.radius * currentNorm);
		spherePoints.push_back(sphere.center + mvmt - sphere.radius * currentNorm);
		pSize = spherePoints.size();
		float min2, max2;
		min2 = max2 = glm::dot(currentNorm, spherePoints[0]);
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(currentNorm, spherePoints[j]);
			if (sProj < min2) min2 = sProj;
			if (sProj > max2) max2 = sProj;
		}

		//Is there overlap on this axis?
		if (!(min1 <= max2 && max1 >= min2))
		{
			return -1.0f;
		}
	}

	if (tLast < tFirst) return -1.0f;

	return tFirst;
}


// This runs once every physics timestep.
void update(float dt)
{
	float t = 1.0f;

	//If the user presses spacebar, use non-continuous collision detection
	if (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS)
	{
		//Determine relative velocity of circle from AABB
		glm::vec3 relV = sphereBody->velocity - cubeBody->velocity;
		t = CheckDynamicCollision(*sphereCollider, *cubeCollider, relV * dt, 0.0f, 1.0f);

		//If there is no collision, move the entire way.
		if (t == -1.0f)
		{
			t = 1.0f;

		}
	}

	//Move the spherebody
	sphereBody->position += sphereBody->velocity * dt * t;
	cubeBody->position += cubeBody->velocity * dt * t;


	//Move collider
	sphereCollider->center = sphereBody->position;
	cubeCollider->center = cubeBody->position;

	//Once we have solved for the bodys position, we should translate the mesh to match it
	sphere->translation = glm::translate(glm::mat4(1.0f), sphereBody->position);
	cube->translation = glm::translate(glm::mat4(1.0f), cubeBody->position);

	//If the position goes off of the right edge of the screen, loop it back to the left
	if (sphereBody->position.x > 1.0f)
	{

		sphereBody->position.x = -1.0f;
		sphereCollider->center = sphereBody->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		sphere->translation = glm::translate(glm::mat4(1.0f), sphereBody->position);


	}
	if (cubeBody->position.x < -1.0f)
	{
		cubeBody->position.x = 1.0f;
		cubeCollider->center = cubeBody->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		cube->translation = glm::translate(glm::mat4(1.0f), cubeBody->position);
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
	cube->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Sphere - AABB (3D Dynamic Collision Detection)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate the sphere mesh
	float scale = 0.1f;
	
	GenerateSphereMesh(1.0f, 40);

	//Generate the box mesh
	struct Vertex boxVerts[24] =
	{
		{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
	};

	//triangle creation
	cube = new struct Mesh(24, boxVerts, GL_LINES);


	//Scale the shapes
	sphere->scale = glm::scale(sphere->scale, glm::vec3(scale));
	cube->scale = glm::scale(cube->scale, glm::vec3(scale));


	//Generate the rigidbodies
	sphereBody = new struct RigidBody(
		glm::vec3(-1.0f, 0.0f, 0.0f),		//Start on left side of screen
		glm::vec3(1.0f, 0.0f, 0.0f),		//constant right velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	cubeBody = new struct RigidBody(
		glm::vec3(0.75f, 0.0f, 0.0f),		//Start on right side
		glm::vec3(-0.5f, 0.0f, 0.0f),		//Constant left velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	//Position shapes
	sphere->translation = glm::translate(sphere->translation, sphereBody->position);
	cube->translation = glm::translate(cube->translation, cubeBody->position);

	//Generate the colliders
	sphereCollider = new Sphere(sphereBody->position, scale);
	cubeCollider = new AABB(cubeBody->position, glm::vec3(scale * 2.0f));

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
	delete cube;
	delete sphereBody;
	delete cubeBody;
	delete sphereCollider;
	delete cubeCollider;

	// Frees up GLFW memory
	glfwTerminate();
}