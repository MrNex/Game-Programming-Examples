/*
Title: Forces - Springs & Hooke's Law
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
This is a demonstration of forces. It uses forces to simulate spring mechanics.
There is a triangle which will have a spring force pulling it to the mouse position. The demo will use 
second order Newton - Euler integration to solve for it's and it's linear velocity & position each frame.

The demo does this by using Hooke's Law,
F = -k * (X - O)

You can reset an object's position and velocity by pressing Spacebar.

**NOTE**: 
	Spring motion is oscillatory. Euler integration is TERRIBLE for Oscillatory.
	I would not recommend using (any order) euler integration for Oscillatory motion.
	I am simply using it here for simplicities sake. If you leave the simulation alone for long enough,
	You will be able to see the numerical instability of euler integration for oscillatory motion.
	Your spring will blow up.

	For fixes to this problem, see Velocity Verlet integration, or Runge-Kutta 4 integration.
	Runge-Kutta 2 will absolutely improve the behavior as well.


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

//Struct for kinematics
struct RigidBody
{
	float inverseMass;			//ProTip: Using inverse mass allows you avoid an expensive division operation when it comes
	//	time to calculate the acceleration due to the force.
	//	(Thanks Schwartz!)

	glm::vec3 position;			//Position of the rigidbody
	glm::vec3 velocity;			//The velocity of the rigidbody
	glm::vec3 acceleration;		//The acceleration of the rigidbody

	glm::vec3 netForce;				//Change in linear acceleration over time by factor of mass

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		inverseMass = 1.0f;

		position = glm::vec3(0.0f);
		velocity = glm::vec3(0.0f);
		acceleration = glm::vec3(0.0f);
	}

	///
	//Parameterized constructor, creates rigidbody at certain position
	RigidBody::RigidBody(float mass, const glm::vec3 &pos, const glm::vec3& vel, const glm::vec3& acc)
	{
		inverseMass = mass != 0.0f ? (1.0f / mass) : 0.0f;	//0 mass == infinite mass! Send 0 if an object should never move.

		position = pos;
		velocity = vel;
		acceleration = acc;

	}
};

struct Mesh* triangle;
struct RigidBody* triangleBody;

float k = 1.0f;				//Spring constant

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.


#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions
//Read Shader source
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

//Create shader from source
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

	//Generate uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Integrates the linear kinematics of the triangle using second order Newton - Euler integration
//For a full explanation of this function see:
//Second Order Newton - Euler Integration (Linear)
//
//NOTE: Spring motion is oscillatory. Euler integration is TERRIBLE for Oscillatory.
//I would not recommend using (any order) euler integration for Oscillatory motion.
//I am simply using it here for simplicities sake. If you leave the simulation alone for long enough,
//You will be able to see the numerical instability of euler integration for oscillatory motion.
//Your spring will blow up.
//
//For fixes to this problem, see Velocity Verlet integration, or Runge-Kutta 4 integration.
//Runge-Kutta 2 will absolutely improve the behavior as well.
//
//Parameters:
//	dt: The time since last physics update
void integrateLinear(float dt)
{
	//Calculate new position with
	//	X = X0 + V0*dt + (1/2) * A * dt^2
	glm::vec3 v0dT = dt * triangleBody->velocity;						//Solve for the first degree term
	glm::vec3 aT2 = (0.5f) * triangleBody->acceleration * powf(dt, 2);	//Solve for the second degree term
	triangleBody->position += v0dT + aT2;								//Take sum

	//determine the new velocity
	triangleBody->velocity += dt * triangleBody->acceleration;


	//Once we have solved for the bodys position, we should translate the mesh to match it
	triangle->translation = glm::translate(glm::mat4(1.0f), triangleBody->position);
}


///
// Applys a spring force  to the triangle & calculates acceleration each time step and then performs linear integration
void update(float dt)
{
	//Lets get the mouse position!
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	//And lets get it in normalized device coordinates!
	float NDCX = (x / 400.0f) - 1.0f;
	float NDCY = -(y / 400.0f) + 1.0f;

	glm::vec3 mousePos(NDCX, NDCY, 0.0f);	//Even though example is in 2D, I tend to always use 3D. It makes dealing with rotation easier (though none of that here..)

	//If the user presses spacebar, reset the object's velocity to 0 and set position to mouse pos
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		triangleBody->position = mousePos;
		triangleBody->velocity = glm::vec3(0.0f);
	}
	//A spring force is determined by:
	//	F = -k * dX
	//
	//Where k is the spring stiffness, and dX is the displacement of the spring X  from the origin O of the spring system, given by:
	//	dX = (X - O)
	//
	//Lets say that the origin of the spring system is the mouse position!


	//Let us calculate the spring force
	triangleBody->netForce = -k * (triangleBody->position - mousePos);

	//Remember F = m * A?
	//Well lets use it to solve for the acceleration of our triangle
	triangleBody->acceleration = triangleBody->netForce * triangleBody->inverseMass;

	//Integrate over timestep
	integrateLinear(dt);
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
	triangle->Draw();
}

#pragma endregion util_Functions


void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Forces - Springs & Hookes Law", nullptr, nullptr);
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
	triangle = new struct Mesh(3, triVerts, GL_TRIANGLES);

	//Scale the triangle
	triangle->scale = glm::scale(triangle->scale, glm::vec3(0.1f));

	//Generate the triangle's rigidbody
	triangleBody = new struct RigidBody(
		1.0f,						//Mass.
		glm::vec3(0.0f),			//Start in center of screen
		glm::vec3(0.0f),			//Start from rest
		glm::vec3(0.0f)				//no linear acceleration
		);

	//Print controls
	std::cout << "Controls:\nPress Spacebar to reset the object's linear velocity & position\n";

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

	delete triangle;
	delete triangleBody;

	// Frees up GLFW memory
	glfwTerminate();
}