/*
Title: Circle - Triangle (2D)
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
The demo contains two a pink moving circle, and a yellow moving triangle.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving circle to move any further.
If the moving circle reaches the right side of the screen, it will wrap around to the left side again.

The user can disable collision detection by holding spacebar.

The continuous collision detection algorithm uses the concept of Minkowski Sums
in order to test for collision between a moving circle and a moving triangle.
First we get the relative velocity of the circle with respect to the triangle.
Then we create two line segments for each triangle edge positioned at +- the radius of the circle
in the direction of the vector perpendicular to the edge. Then we test for collisions of the
line segment created by the movement of the center point of the circle & the 6 edge line segments.
If no collision is detected we must also create three circles centered at the vertices of the triangle
and test for collisions between the line segment created by the movement of the center point of the circle
and these three circles.

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

//Struct for circle collider
struct Circle
{
	float radius;
	glm::vec2 center;

	//Default constructor, creates unit circle at origin
	Circle::Circle()
	{
		center = glm::vec2(0.0f);
		radius = 1.0f;
	}

	//PArameterized constructor, creates circle from given center and radius
	Circle::Circle(const glm::vec2& c, float r)
	{
		center = c;
		radius = r;
	}
};

//Struct for triangle collider
struct Triangle
{
	glm::vec2 center;
	glm::vec2 a, b, c;

	//Default constructor, creates basic triangle positioned at origin
	Triangle::Triangle()
	{
		center = glm::vec2(0.0f);
		a = glm::vec2(-1.0f, -1.0f);
		b = glm::vec2(1.0f, -1.0f);
		c = glm::vec2(0.0f, 1.0f);
	}

	//Parameterized constructor, creates triangle with 3 points and given center
	Triangle::Triangle(const glm::vec2& pos, const glm::vec2& A, const glm::vec2 &B, const glm::vec2 &C)
	{
		center = pos;

		a = A;
		b = B;
		c = C;
	}
};

struct Mesh* circle;
struct Mesh* triangle;

struct RigidBody* circleBody;
struct RigidBody* triangleBody;

struct Circle* circleCollider;
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
float CheckLineSegmentCollision(const glm::vec2 &start1, const glm::vec2 &dir1, const glm::vec2 &start2, const glm::vec2 &dir2)
{
	float t;
	float s;

	float magProd = glm::length(dir1) * glm::length(dir2);
	if (fabs(glm::dot(dir1, dir2)) == magProd) return -1.0f;	//Lines parallel

	if (fabs(dir2.x) > FLT_EPSILON)
	{
		float m2 = dir2.y / dir2.x;
		t = (start1.y - start2.y - m2 * start1.x + m2 * start2.x) / (m2 * dir1.x - dir1.y);
		s = (start1.x - start2.x + t * dir1.x) / dir2.x;

	}
	else
	{
		float m1 = dir1.y / dir1.x;
		s = (start2.y - start1.y - m1 * start2.x + m1 * start1.x) / (m1 * dir2.x - dir2.y);
		t = (start2.x - start1.x + s * dir2.x) / dir1.x;

	}
	if (s < 0.0f || s > 1.0f || t < 0.0f || t > 1.0f) return -1.0f;

	return t;

}

///
//Checks if a circle and a line segment are colliding
//
//PArameters:
//	c: The circle
//	lineStart: The starting endpoint of the segment
//	lineDir: The direction and magnitude of the segment
//
//Returns:
//	The time of collision, -1.0f if no collision occurred
float checkCircleLineSegmentCollision(const Circle &c, const glm::vec2 &lineStart, const glm::vec2 &lineDir)
{
	//Position everything relative to the line being at the origin
	glm::vec2 circlePos = c.center - lineStart;

	//Project the circle center onto the line direction
	float projMag = glm::dot(circlePos, lineDir) / glm::dot(lineDir, lineDir);
	float ratio = c.radius / glm::length(lineDir);
	//Make sure the circle center is close enough to the line segment ends to possibly collide
	if (projMag < -ratio || projMag > 1.0f + ratio) return -1.0f;

	glm::vec2 projPos = projMag * lineDir;
	//Find the distance the circle is away from the line segment
	float dist = glm::length(circlePos - projPos);
	if (dist < c.radius)
	{
		return projMag - ratio;
	}
	return -1.0f;
}

///
//Performs a dynamic collision check between a moving circle and a triangle
//
//Overview:
//	Algorithm uses the concept of Minkowski Sums
//	in order to test for collision between a moving circle and a moving triangle.
//	First we get the relative velocity of the circle with respect to the triangle.
//	Then we create two line segments for each triangle edge positioned at + -the radius of the circle
//	in the direction of the vector perpendicular to the edge.Then we test for collisions of the
//	line segment created by the movement of the center point of the circle & the 6 edge line segments.
//	If no collision is detected we must also create three circles centered at the vertices of the triangle
//	and test for collisions between the line segment created by the movement of the center point of the circle
//	and these three circles.
//
//Parameters:
//	c: The moving circle
//	t: The static triangle
//	mvmt: The relative movement vector of circle from an observer on triangle
//	tStart: The start of the interval, 0.0f <= tStart < 1.0f
//	tEnd: The end of the interval, 0.0f < tEnd <= 1.0f
//
//Returns:
//	a t value between 0 and 1 indicating the "relative time" since the start of this frame that the collision occurred.
//	a t value of 0.0f would indicate the very start of this frame, a t value of 1.0f would indicate the very end of this frame.
//	This function will return a negative number if no collision was registered.
float CheckDynamicCollision(const Circle& c, const Triangle &t, const glm::vec2 &mvmt, float tStart, float tEnd)
{
	//Get the three edge vectors of the triangle
	glm::vec2 AB = t.b - t.a;
	glm::vec2 BC = t.c - t.b;
	glm::vec2 CA = t.a - t.c;

	struct Line
	{
		glm::vec2 start;
		glm::vec2 direction;
	};

	//Create a line segment which represents the path travelled by the center of the circle
	struct Line circleMvmt;
	circleMvmt.start = c.center;
	circleMvmt.direction = mvmt;

	//Find the perpendicular vector to edge AB
	glm::vec2 edgePerp(-AB.y, AB.x);
	edgePerp = glm::normalize(edgePerp);
	//Create two line segments parallel to the edge, positioned +- circle's radius away
	struct Line edge1, edge2;
	edge1.start = t.a + t.center + (c.radius * edgePerp);
	edge2.start = t.a + t.center - (c.radius * edgePerp);
	edge1.direction = edge2.direction = AB;

	//Determine if the circle's movement line segment intersects either of the edge segments.
	float minTimeOfIntersection = CheckLineSegmentCollision(circleMvmt.start, circleMvmt.direction, edge1.start, edge1.direction);
	float timeOfIntersection = CheckLineSegmentCollision(circleMvmt.start, circleMvmt.direction, edge2.start, edge2.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

	//Find the perpendicular vector to edge BC
	edgePerp = glm::vec2(-BC.y, BC.x);
	edgePerp = glm::normalize(edgePerp);
	//Create two line segments parallel to the edge, positioned +- circle's radius away
	edge1.start = t.b + t.center + (c.radius * edgePerp);
	edge2.start = t.b + t.center - (c.radius * edgePerp);
	edge1.direction = edge2.direction = BC;

	//Determine if the circle's movement line segment intersects either of the edge segments.
	timeOfIntersection = CheckLineSegmentCollision(circleMvmt.start, circleMvmt.direction, edge1.start, edge1.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;
	timeOfIntersection = CheckLineSegmentCollision(circleMvmt.start, circleMvmt.direction, edge2.start, edge2.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

	//Find the perpendicular vector to edge CA
	edgePerp = glm::vec2(-CA.y, CA.x);
	edgePerp = glm::normalize(edgePerp);
	//Create two line segments parallel to the edge, positioned +- circle's radius away
	edge1.start = t.c + t.center + (c.radius * edgePerp);
	edge2.start = t.c + t.center - (c.radius * edgePerp);
	edge1.direction = edge2.direction = CA;

	//Determine if the circle's movement line segment intersects either of the edge segments.
	timeOfIntersection = CheckLineSegmentCollision(circleMvmt.start, circleMvmt.direction, edge1.start, edge1.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;
	timeOfIntersection = CheckLineSegmentCollision(circleMvmt.start, circleMvmt.direction, edge2.start, edge2.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

	//Create circles at each one of the triangle vertices matching the circle
	Circle vertexCircle;
	vertexCircle.radius = c.radius;
	
	vertexCircle.center = t.center + t.a;
	timeOfIntersection = checkCircleLineSegmentCollision(vertexCircle, circleMvmt.start, circleMvmt.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

	vertexCircle.center = t.center + t.b;
	timeOfIntersection = checkCircleLineSegmentCollision(vertexCircle, circleMvmt.start, circleMvmt.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

	vertexCircle.center = t.center + t.c;
	timeOfIntersection = checkCircleLineSegmentCollision(vertexCircle, circleMvmt.start, circleMvmt.direction);
	if (timeOfIntersection != -1.0f && (timeOfIntersection < minTimeOfIntersection || minTimeOfIntersection == -1.0f)) minTimeOfIntersection = timeOfIntersection;

	return minTimeOfIntersection;
}


// This runs once every physics timestep.
void update(float dt)
{
	float t;

	//If the user presses spacebar, use non-continuous collision detection
	if (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS)
	{
		//Determine relative velocity of circle 1 from circle 2
		glm::vec3 relV = circleBody->velocity - triangleBody->velocity;
		t = CheckDynamicCollision(*circleCollider, *triangleCollider, glm::vec2(relV)* dt, 0.0f, 1.0f);

		//If there is no collision, move the entire way.
		if (t == -1.0f)
		{
			t = 1.0f;

		}
	}

	//Move the circlebody
	circleBody->position += circleBody->velocity * dt * t;
	triangleBody->position += triangleBody->velocity * dt * t;


	//Move collider
	circleCollider->center = glm::vec2(circleBody->position);
	triangleCollider->center = glm::vec2(triangleBody->position);

	//Once we have solved for the bodys position, we should translate the mesh to match it
	circle->translation = glm::translate(glm::mat4(1.0f), circleBody->position);
	triangle->translation = glm::translate(glm::mat4(1.0f), triangleBody->position);

	//If the position goes off of the right edge of the screen, loop it back to the left
	if (circleBody->position.x > 1.0f)
	{

		circleBody->position.x = -1.0f;
		circleCollider->center = glm::vec2(circleBody->position);
		//Once we have solved for the bodys position, we should translate the mesh to match it
		circle->translation = glm::translate(glm::mat4(1.0f), circleBody->position);


	}
	if (triangleBody->position.x < -1.0f)
	{
		triangleBody->position.x = 1.0f;
		triangleCollider->center = glm::vec2(triangleBody->position);
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
	circle->Draw();
	triangle->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Circle - Triangle (2D Dynamic Collision Detection)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate the circle mesh
	float scale = 0.1f;
	int numVertices = 72;
	struct Vertex circleVerts[72];
	float stepSize = 2.0f * 3.14159 / (numVertices / 3.0f);
	int vertexNumber = 0;
	for (int i = 0; i < numVertices; i++)
	{
		circleVerts[i] = { cosf(vertexNumber * stepSize), sinf(vertexNumber * stepSize), 0.0f, 1.0f, 1.0f, 0.0f, 1.0 };
		++i;
		++vertexNumber;
		circleVerts[i] = { cosf(vertexNumber * stepSize), sinf(vertexNumber * stepSize), 0.0f, 1.0f, 1.0f, 0.0f, 1.0 };
		++i;
		circleVerts[i] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	}

	//circle1 creation
	circle = new struct Mesh(numVertices, circleVerts, GL_TRIANGLES);

	//Generate the triangle mesh
	struct Vertex triVerts[3] = 
	{
		{-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}
	};

	//triangle creation
	triangle = new struct Mesh(3, triVerts, GL_TRIANGLES);


	//Scale the shapes
	circle->scale = glm::scale(circle->scale, glm::vec3(scale));
	triangle->scale = glm::scale(triangle->scale, glm::vec3(scale));


	//Generate the rigidbodies
	circleBody = new struct RigidBody(
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
	circle->translation = glm::translate(circle->translation, circleBody->position);
	triangle->translation = glm::translate(triangle->translation, triangleBody->position);

	//Generate the colliders
	circleCollider = new Circle(glm::vec2(circleBody->position), scale);
	triangleCollider = new Triangle(glm::vec2(triangleBody->position), 
		scale * glm::vec2(triVerts[0].x, triVerts[0].y), scale * glm::vec2(triVerts[1].x, triVerts[1].y), scale * glm::vec2(triVerts[2].x, triVerts[2].y));

	//Print controls
	std::cout << "Controls:\nPress and hold spacebar to disable continuous collision detection.\nWhen two boxes collide, continue the simulation by toggling continuous collision detection off.\n";

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

	delete circle;
	delete triangle;
	delete circleBody;
	delete triangleBody;
	delete circleCollider;
	delete triangleCollider;

	// Frees up GLFW memory
	glfwTerminate();
}