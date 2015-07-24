/*
Title: Gift Wrapping
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
This is a demonstration of implementing an algorithm known as Gift Wrapping.
The Gift Wrapping algorithm is a method of computing the smallest convex hull which contains 
a set of points in 3D. It is the 3D analog of the popular Jarvis March algorithm.

The Gift-Wrapping algorithm works by first computing an initial edge which is known to be on the convex hull.
The initial edge is calculated using the Jarvis March algorithm. Then, The algorithm 
creates faces attached to the existing edges which contain all other points on one side of them.

References:
A Direct Convex Hull Algorithm by John Henckel:
	http://poorfox.com/tru-physics/hull.html
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
*/

#include "GLIncludes.h"
#include <iterator>

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

//Edge made up of two elements in the rigidbody array
struct Edge
{
	int index1, index2;

	bool Edge::operator==(const struct Edge& RHS)
	{
		return (this->index1 == RHS.index1 && this->index2 == RHS.index2);
	}
};

//Struct defining a plane
struct Plane
{
	glm::vec3 normal;
	float dist;
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * this->numVertices, this->vertices, GL_DYNAMIC_DRAW);

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

//Struct for rigidbody kinematics
struct RigidBody
{


	glm::vec3 position;				//Position of the rigidbody
	glm::vec3 velocity;				//The velocity of the rigidbody

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	///
	//Parameterized constructor, creates rigidbody with specified initial values
	//
	//Parameters:
	//	pos: Initial position
	//	vel: Initial velocity
	//	acc: Initial acceleration
	//	mass: The mass of the rigidbody (0.0f for infinite mass)
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel)	
	{
		position = pos;
		velocity = vel;
	}
};

struct Mesh* point;
std::vector<RigidBody> bodies;
std::vector<struct Edge> hull;

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.

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
	glm::mat4 proj = glm::ortho(-1.0f,1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	VP = proj * view;

	//Create uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPointSize(5.0f);
	glEnable(GL_POINT_SMOOTH);
}


#pragma endregion Helper_functions

///
//Performs second order euler integration for linear motion
//
//Parameters:
//	dt: The timestep
//	body: The rigidbody being integrated
void IntegrateLinear(float dt, RigidBody &body)
{
	body.position += body.velocity * dt;
}

///
//Stops the bodies from escaping the edge of the screen
//
//Parameters:
//	body: The body to check & if necessary prevent from leaving the screen
void Wrap(RigidBody& body)
{
	if(body.position.x < -1.0f)
	{
		body.position.x = -1.0f;
		body.velocity.x *= -1.0f;
	}
	if(body.position.x > 1.0f)
	{
		body.position.x = 1.0f;
		body.velocity.x *= -1.0f;
	}

	if(body.position.y < -1.0f)
	{
		body.position.y = -1.0f;
		body.velocity.y *= -1.0f;
	}
	if(body.position.y > 1.0f)
	{
		body.position.y = 1.0f;
		body.velocity.y *= -1.0f;
	}

	if(body.position.z < -1.0f)
	{
		body.position.z = -1.0f;
		body.velocity.z *= -1.0f;
	}
	if(body.position.z > 1.0f)
	{
		body.position.z = 1.0f;
		body.velocity.z *= -1.0f;
	}
}

///
//Computes an edge which is guaranteed to be on the 3D convex hull
//This edge can be used as a start point for generating the rest of the 3D convex hull
//
//Parameters:
//	rigidBodies: the list of points (moving pointmasses in my case)
//	initial: A reference to an edge to store the result in
void FindInitialEdge(const std::vector<RigidBody> &rigidBodies, struct Edge& initial)
{
	//Step 1: Choose a random unit vector
	float x = 2.0f * ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f);
	float y = 2.0f * ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f);
	float z = 2.0f * ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f);
	glm::vec3 direction = glm::normalize(glm::vec3(x, y, z));

	//Step 2: Find the extreme point in the random direction
	int size = rigidBodies.size();
	int currentIndex = 0;
	float currentMax = glm::dot(rigidBodies[currentIndex].position, direction);
	float currentDist = 0.0f;
	for(int i = 1; i < size; i++)
	{
		if((currentDist = glm::dot(rigidBodies[i].position, direction)) > currentMax)
		{
			currentMax = currentDist;
			currentIndex = i;
		}
	}

	//Step 3: Make the plane perpendicular to D which contains the current index & store the current index in the initial edge
	Plane p;
	p.normal = direction;
	p.dist = currentMax;

	initial.index1 = currentIndex;

	currentIndex = initial.index1 == 0 ? 1 : 0;
	float min = fabs(glm::dot(glm::normalize(bodies[currentIndex].position - bodies[initial.index1].position), p.normal));
	float current;
	//Step 4: Find the point which creates an edge with the point at the current index
	//which has the smallest angle with the plane than all other points
	for(int i = currentIndex + 1; i < size; i++)
	{
		if(i == initial.index1) continue;
		current = fabs(glm::dot(glm::normalize(bodies[i].position - bodies[initial.index1].position), p.normal));
		if(current < min)
		{
			min = current;
			currentIndex = i;
		}
	}

	//Step 5: Store the second index inside of the edge
	initial.index2 = currentIndex;
}

///
//Uses the Gift Wrapping algorithm to determine the smallest convex hull around
//a 3D set of points.
//
//Parameters:
//	edgeList: The list to store all edges on the hull
//	rigidBodies: The list of all pointmasses which we are forming the hull around
void GiftWrap(std::vector<struct Edge> &edgeList, const std::vector<RigidBody> &rigidBodies)
{
	//Step 1: Get the number of input points
	int numInput = rigidBodies.size();

	//Error on degenerate cases
	if(numInput < 3)
	{
		std::cout << "Degenerate hull." << std::endl;
		return;
	}

	//Step 3: Initialize 3 contains to hold Confirmed Edges, confirmed Faces, and Edges which need to be confirmed
	std::vector<Edge> todoList;

	//Step 4: We must find the initial edge to begin performing the algorithm from and add it to the TODO list
	struct Edge initial;
	FindInitialEdge(rigidBodies, initial);
	todoList.push_back(initial);

	//Step 5: For every edge in the TODO list
	while(todoList.size() > 0)
	{
		//Step a: Get the next edge in the list
		Edge edge = todoList.back();
		todoList.pop_back();

		//Step b: Create a plane containing the edge and the first non-colinear point in the set
		int index3;
		Plane p;
		p.normal = glm::vec3(0.0f);
		int i = 0;
		while(glm::length(p.normal) < FLT_EPSILON && i < numInput)
		{
			index3 = i;
			glm::vec3 direction = glm::cross(rigidBodies[edge.index1].position - rigidBodies[edge.index2].position, rigidBodies[index3].position - rigidBodies[edge.index2].position);
			p.normal = direction;
			i++;
		}

		glm::normalize(p.normal);
		p.dist = glm::dot(p.normal, rigidBodies[edge.index2].position);

		//Step c: For every remaining point, find the point with the largest signed distance, 
		//each time re-creating the plane to use that point instead of the point at index3
		for(i = 0; i < numInput; i++)
		{
			if(i == index3 || i == edge.index2 || i == edge.index1) continue;
			float current = glm::dot(rigidBodies[i].position - rigidBodies[edge.index2].position, p.normal);
			//if(current > p.dist)
			if(current > FLT_EPSILON)
			{
				index3 = i;
				p.dist = current;
				p.normal = glm::normalize(glm::cross(rigidBodies[edge.index1].position - rigidBodies[edge.index2].position, rigidBodies[index3].position - rigidBodies[edge.index2].position));
			}
		}

		//We now know that p is a plane which contains a face made up of the edge and index3.
		//If you wanted a list of faces in the hull instead of / aswell as edges, you would add the face to the
		//list of faces in the convex hull here!!
		//
		//For this implementation that information is not needed, I simply want to draw the edges of the convex hull
		//rather than compute collision with it. In most cases you will need this though

		//step d: For each edge in the face create the reversal of the edge, and edit the ToDo & edge list
		struct Edge edgeReverse;
		edgeReverse.index1 = edge.index2;
		edgeReverse.index2 = edge.index1;

		struct Edge edge2;
		edge2.index1 = edge.index2;
		edge2.index2 = index3;

		struct Edge edge2Reverse;
		edge2Reverse.index1 = edge2.index2;
		edge2Reverse.index2 = edge2.index1;

		struct Edge edge3;
		edge3.index1 = index3;
		edge3.index2 = edge.index1;

		struct Edge edge3Reverse;
		edge3Reverse.index1 = edge3.index2;
		edge3Reverse.index2 = edge3.index1;


		//If the reverse is in the edge list
		if(std::find(edgeList.begin(), edgeList.end(), edgeReverse) != edgeList.end())
		{
			std::vector<Edge>::iterator it = std::find(todoList.begin(), todoList.end(), edge);
			//If the edge is in the ToDo list remove it, as it has been completed
			if(it != todoList.end())
			{
				todoList.erase(it);
			}
		}
		else
		{
			//Add the edge to the edge list
			edgeList.push_back(edge);
			//Add edge reverse to the todo list
			todoList.push_back(edgeReverse);

		}


		//Repeat for edge 2 and 3
		if(std::find(edgeList.begin(), edgeList.end(), edge2Reverse) != edgeList.end())
		{
			std::vector<Edge>::iterator it = std::find(todoList.begin(), todoList.end(), edge2);
			if(it != todoList.end())
			{
				todoList.erase(it);
			}

		}
		else
		{
			edgeList.push_back(edge2);
			todoList.push_back(edge2Reverse);
		}


		if(std::find(edgeList.begin(), edgeList.end(), edge3Reverse) != edgeList.end())
		{
			std::vector<Edge>::iterator it = std::find(todoList.begin(), todoList.end(), edge3);
			//If the edge is in the ToDo list remove it
			if(it != todoList.end())
			{
				todoList.erase(it);
			}
		}
		else
		{
			edgeList.push_back(edge3);
			todoList.push_back(edge3Reverse);
		}

	}
}

// This runs once every physics timestep.
void update(float dt)
{	
	int size = bodies.size();

	for(int i = 0; i < size; i++)
	{
		IntegrateLinear(dt, bodies[i]);
		Wrap(bodies[i]);
	}

	//Clear the current hull
	hull.clear();
	//Calculate the new hull
	GiftWrap(hull, bodies);

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
	glLineWidth(1.0f);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Draw the Gameobjects

	//Draw all points
	int size = bodies.size();
	for(int i = 0; i < size; i++)
	{
		point->translation = glm::translate(glm::mat4(1.0f), bodies[i].position);
		point->Draw();
	}

	//Draw all lines

	glm::mat4 mvp = VP * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(mvp));

	size = hull.size();
	glBegin(GL_LINES);
	for(int i = 0; i < size; i++)
	{
		glVertex3fv(glm::value_ptr(bodies[hull[i].index1].position));
		glVertex3fv(glm::value_ptr(bodies[hull[i].index2].position));
	}
	glEnd();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Gift Wrapping", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate the point mesh
	Vertex pointVertex;
	pointVertex.x = pointVertex.y = pointVertex.z = pointVertex.r = 0.0f;
	pointVertex.g = pointVertex.b = pointVertex.a = 1.0f;

	//rope creation
	point = new struct Mesh(1, &pointVertex, GL_POINTS);

	//Scale the rope
	point->scale = glm::scale(point->scale, glm::vec3(1.0f));

	//Generate 5 rigidbodies
	for(int i = 0; i < 5; i++)
	{
		float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;
		float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;
		float z = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;

		float vx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;
		float vy = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;
		float vz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;

		bodies.push_back(RigidBody(glm::vec3( x, y, z), glm::vec3(vx, vy, vz)));
	}

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

	delete point;

	// Frees up GLFW memory
	glfwTerminate();
}