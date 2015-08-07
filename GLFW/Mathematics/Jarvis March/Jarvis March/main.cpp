/*
Title: Jarvis March
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
This is a demonstration of implementing an algorithm known as Jarvis March.
The Jarvis March algorithm is a popular & simple method of computing the
smallest convex hull which contains a set of points in 2D.

The Jarvis March algorithm works by first finding the left-most
point in the set. Then, it tests all edges to find the counter-clockwise
most one. This continues until it wraps around to the starting edge again.

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
std::vector<int> hull;

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
}

///
//Determines if edge 1 is counter clockwise with respect to edge 2
//
//Parameters:
//	e1: The edge to check if it is counter clockwise
//	e2: The edge we are checking with respect to
//
//Returns:
//	true if edge1 is CCW with respect to edge 2, else false
bool IsCounterClockwise(const glm::vec2& e1, const glm::vec2& e2)
{
	if(e1.x * e2.y - e2.x * e1.y > FLT_EPSILON) return true;
	else return false;
}

///
//Uses the Jarvis March algorithm to determine the smallest convex hull around
//a 2D set of points in the XY plane.
//
//Parameters:
//	hullList: The list to store all points on the hull
//	rigidBodies: The list of all pointmasses which we are forming the hull around
void JarvisMarch(std::vector<int> &hullList, const std::vector<RigidBody> &rigidBodies)
{
	//Step 1: Find the left-most point
	int pointOnHull = 0;
	int size = rigidBodies.size();
	for(int i = 1; i < size; i++)
	{
		if(rigidBodies[i].position.x < rigidBodies[pointOnHull].position.x) pointOnHull = i;
	}

	//Step 2: Create second variable to hold prospective points on the hull which can still be outruled
	int endPoint = 0;

	//Until we loop back around to the left-most point
	while(hullList.size() <= 0 || endPoint != hullList[0])
	{
		//Add the current point to the hull list
		hullList.push_back(pointOnHull);
		
		//Set the current prospective point as the first point
		endPoint = 0;

		for(int i = 1; i < size; i++)
		{
			//Construct an edge from the last point on the hull to the ith point
			glm::vec2 edge1 = glm::vec2(rigidBodies[i].position - rigidBodies[pointOnHull].position);
			//Construct an edge from the last point on the hull to the current prospective point
			glm::vec2 edge2 = glm::vec2(rigidBodies[endPoint].position - rigidBodies[pointOnHull].position);

			//If the edge to the ith point is counter clockwise with respect to the edge to the prospective point
			//Set the prospective point to the point at i
			if(endPoint == pointOnHull || IsCounterClockwise(edge1, edge2))
			{
				endPoint = i;
			}
		}

		//Set the point on hull
		pointOnHull = endPoint;
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
	JarvisMarch(hull, bodies);

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
		glVertex3fv(glm::value_ptr(bodies[hull[i]].position));
		glVertex3fv(glm::value_ptr(bodies[hull[((i+1) < size ? (i+1) : 0)]].position));
	}
	glEnd();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Jarvis March", nullptr, nullptr);
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

	//Generate 25 rigidbodies
	for(int i = 0; i < 15; i++)
	{
		float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;
		float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) - 0.5f;

		bodies.push_back(RigidBody(glm::vec3(0.0f), glm::vec3(x, y, 0.0f)));
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