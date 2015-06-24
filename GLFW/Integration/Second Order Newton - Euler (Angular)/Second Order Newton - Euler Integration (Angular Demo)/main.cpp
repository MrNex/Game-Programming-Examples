/*
Title: Second Order Newton - Euler Integration (Angular)
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
This is a demonstration of Second order Newton - Euler integration.
There is a triangle with a constant angular acceleration. The demo will use 
Second order Newton - Euler integration to solve for it's angular velocity & orientation each
frame. Second order Newton - Euler integration is relatively simple to implement and
has a higher stability than it's first order counterpart. Though in certain cases it is still 
very unstable and inaccurate.

You can reset the object's angular velocity to 0 by pressing spacebar.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shaders
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

//Struct for kinematics
struct RigidBody
{
	glm::vec3 position;			//Position of the rigidbody
	glm::vec3 velocity;			//The velocity of the rigidbody
	glm::vec3 acceleration;		//The acceleration of the rigidbody

	glm::mat4 rotation;				//Note: This can be a 3x3 matrix, but because GLM generates mat4's by default, it will be easier to work with it rather than against it.
	glm::vec3 angularVelocity;		//Direction is the axis of rotation, magnitude is the Counter Clockwise speed of rotation around axis
	glm::vec3 angularAcceleration;	//Direction represents the axis, magnitude represents the Counter Clockwise acceleration around axis

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		position = glm::vec3(0.0f);
		velocity = glm::vec3(0.0f);
		acceleration = glm::vec3(0.0f);

		rotation = glm::mat4(1.0f);
		angularVelocity = glm::vec3(0.0f);
		angularAcceleration = glm::vec3(0.0f);
	}

	///
	//Parameterized constructor, creates rigidbody at certain position and rotation with certain linear and angular velocity and acceleration
	RigidBody::RigidBody(const glm::vec3 &pos, const glm::vec3& vel, const glm::vec3& acc, const glm::mat4& rot, const glm::vec3& aVel, const glm::vec3& aAcc)
	{
		position = pos;
		velocity = vel;
		acceleration = acc;

		rotation = rot;
		angularVelocity = aVel;
		angularAcceleration = aAcc;
	}
};

struct Mesh* triangle;
struct RigidBody* triangleBody;

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.


#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//REads shader source
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

// Creates shader from source
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

	// create shader program
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
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

// This runs once every physics timestep.
void update(float dt)
{
	//If the user presses spacebar, reset the object's velocity to 0
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) triangleBody->angularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

	//Second order Newton - Euler integration is still fairly simple to grasp and implement, and has much better stability than 
	//the first order equivalent! It does, however, require the use of integration.
	//We know from physics and mathematics that angular acceleration is the derivative of angular velocity, which is to say
	//	aA = d(aV)/dt
	//
	//We also know that angular velocity is in fact the derivative of orientation:
	//	aV = (1/dt) * dU
	//
	//Where U is a unit vector comprised of three angles indicating the rotation of an object around the X, Y, and Z axes.
	//This means that angular Acceleration must in fact be the second derivative of orientation
	//	aA = d^2U/dt^2
	//
	//And using this fact instead of only using the first derivatives in our computation is what makes up the second order Newton - Euler integration.
	//If we consider the angular acceleration at time interval dt
	//	aA = d(aV)/dt
	//	d(aV) = aA * dt
	//
	//And we know d(aV) = aV - aV0, so solving for aV
	//	aV = aV0 + aA * dt
	//
	//Now, if we want to determine the orientation based on both angular acceleration and angular velocity, we can integrate this equation with respect to dt
	//	U = aV0*dt + (1/2) * aA * dt^2 + C
	//
	//Here the C term is very important because it in fact signifies the orientation before the time interval dt!
	//We can have an infinite number of "correct" answers if we do not start from the correct orientation.
	//This brings us to:
	//	U = U0 + aV0*dt + (1/2) * aA * dt^2
	//
	//Let us solve for the second half of the equation
	glm::vec3 dr = dt * triangleBody->angularVelocity + 0.5f * powf(dt, 2.0f) * triangleBody->angularAcceleration;

	//However, here we are using Euler Angles to express our position. I personally prefer to use one rotation matrix which I just constantly update.
	//We can convert U to a rotation matrix by taking the product of 3 different rotation matrices (around the X, Y, and Z axes):
	//	R = Rx * Ry * Rz
	//(Note, the orientation in the rigidbody struct was already set up this way-- I was using the vector U and euler integration as a means of explaining the mathematics)
	//
	//Furthermore, our dr represents an axis of rotation signified by it's direction, and a magnitude of rotation signified by it's magnitude.
	//Using Rodriguez's formula we can construct a rotation matrix dR which will represent a rotation matrix of mag(dr) degrees (Though I prefer radians) around the axis created by dr
	float magR = glm::length(dr);
	glm::mat4 R = glm::rotate(glm::mat4(1.0f), magR, dr);
	
	//So let us now calculate our new orientation
	triangleBody->rotation = R * triangleBody->rotation;

	//It is important that after calculating our new orientation we determine our new angular velocity, and NOT before!
	//The reason being is the orientation equation calls for aV0, the angular velocity at the beginning of this time interval.
	//If we solved for velocity first we would not have aV0, we would have aV1.
	//
	//Now we will use
	//	aV = aV0 + aA * dt
	//to determine the new velocity
	triangleBody->angularVelocity += dt * triangleBody->angularAcceleration;

	//Once we have solved for the bodys position, we should translate the mesh to match it
	triangle->rotation = triangleBody->rotation;
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
	//Create window
	window = glfwCreateWindow(800, 800, "Second Order Newton-Euler Integration (Angular Demo)", nullptr, nullptr);
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
		glm::vec3(0.0f, 0.0f, 0.0f),		//Start on left side of screen
		glm::vec3(0.0f, 0.0f, 0.0f),		//Start from rest
		glm::vec3(0.0f, 0.0f, 0.0f),		//Start with no linear acceleration
		glm::mat4(1.0f),					//Upright orientation
		glm::vec3(0.0f),					//No angular velocity
		glm::vec3(0.0f, 0.0f, 1.0f)			//Start with a small positive angular acceleration around Z axis
		);

	//Print controls
	std::cout << "Controls:\nPress spacebar to reset the object's angular velocity to 0.\n";

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