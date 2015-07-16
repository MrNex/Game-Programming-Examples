/*
Title: Mass Spring Softbody (1D)
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
This is a demonstration of using mass spring systems to simulate soft body physics.
The demo contains a blue elastic rope which is fixed at one end to the mouse pointer.
The rope is made up of 11 point masses with 10 springs between them.

Each physics timestep the mass spring system is solved to determine the force on each
individual point mass in the system. This is done using Hooke's law. The springs also contain 
dampening forces to help relax the system upon purterbation. It should
be noted that the physics timestep had to be turned down in order
to maintain stability of the spring system while using Newton Euler integration.
In order to avoid this, one should use another integration scheme such as 
Order 4 Runge-Kutta.

The user can move the mouse to displace one end of the rope. The user can also
left click to cause wind coming from the left, and right click to cause wind to come from the right.

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

	void Mesh::RefreshData(void)
	{
		//BEcause we are changing the vertices themselves and not transforming them
		//We must write the new vertices over the old on the GPU.
		glBindVertexArray(VAO);

		GLvoid* memory = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(memory, vertices, numVertices * sizeof(Vertex));
		glUnmapBuffer(GL_ARRAY_BUFFER);
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
	float mass;						//We will need to use both mass and inverse mass extensively here.
	float inverseMass;				//I tend to use inverse mass as opposed to mass itself. It saves lots of divides when forces are involved.

	glm::vec3 position;				//Position of the rigidbody
	glm::vec3 velocity;				//The velocity of the rigidbody
	glm::vec3 acceleration;			//The acceleration of the rigidbody

	glm::vec3 netForce;				//Forces over time
	glm::vec3 netImpulse;			//Instantaneous forces

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		mass = inverseMass = 1.0f;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);
	}

	///
	//Parameterized constructor, creates rigidbody with specified initial values
	//
	//Parameters:
	//	pos: Initial position
	//	vel: Initial velocity
	//	acc: Initial acceleration
	//	mass: The mass of the rigidbody (0.0f for infinite mass)
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, float m)	
	{
		mass = m;
		inverseMass = m == 0.0f ? 0.0f : 1.0f / m;

		position = pos;
		velocity = vel;
		acceleration = acc;

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);
	}
};

//A struct for 1D Mass-Spring softbody physics
struct SoftBody
{
	//The rigidbodies of the point masses which make up the softbody mass-spring system
	unsigned int numRigidBodies;
	std::vector<struct RigidBody> rigidBodies;

	float coefficient;	//The spring coefficients between the point masses in the system
	float restLength;	//The resting length of the springs
	float dampening;	//The dampening coefficient of the springs

	SoftBody::SoftBody()
	{
		numRigidBodies = 0;
		coefficient = 0.0f;
		restLength = 0.0f;
		dampening = 0.0f;
	}

	SoftBody::SoftBody(struct Mesh& m, float coeff, float length, float damp)
	{
		numRigidBodies = m.numVertices;
		coefficient = coeff;
		restLength = length;
		dampening = damp;

		for(int i = 0; i < numRigidBodies; i++)
		{
			rigidBodies.push_back(struct RigidBody(
				glm::vec3(m.vertices[i].x, m.vertices[i].y, m.vertices[i].z),
				glm::vec3(0.0f),
				glm::vec3(0.0f),
				1.0f
				))
				;
		}
	}
};

struct Mesh* rope;

struct SoftBody* body;

glm::vec3 gravity(0.0f, -0.98f, 0.0f);

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.001; // This is the number of milliseconds we intend for the physics to update.

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
	//Calculate the current acceleration
	body.acceleration = body.inverseMass * body.netForce;
	
	//Calculate new position with
	//	X = X0 + V0*dt + (1/2) * A * dt^2
	glm::vec3 v0dT = dt * body.velocity;						//Solve for the first degree term
	glm::vec3 aT2 = (0.5f) * body.acceleration * powf(dt, 2);	//Solve for the second degree term
	body.position += v0dT + aT2;								//Take sum

	//determine the new velocity
	body.velocity += dt * body.acceleration + body.inverseMass * body.netImpulse;

	//Zero the net impulse and net force!
	body.netForce = body.netImpulse = glm::vec3(0.0f);
}

// This runs once every physics timestep.
void update(float dt)
{	
	//Get the current mouse position
	double currentMouseX, currentMouseY;
	glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

	//Translate to normalized device coordinates
	//Since the cursor posiiton is represented in pixel values, 
	//dividing it by the window dimension will reduce the position to 0.0 - 1.0 range.
	//multiplying it with 2 and then subtracting it with 1 wil bring the 
	//mouse position to -1 ot 1.
	glm::vec2 mousePos(
		(((float)currentMouseX / 800.0f) * 2.0f) - 1.0f,
		1.0f - (((float)currentMouseY / 800.0f) * 2.0f)
		);

	//Move the position of the first body to the mouse
	body->rigidBodies[0].position.x = mousePos.x;
	body->rigidBodies[0].position.y = mousePos.y;

	//Create wind based on mouse movement
	glm::vec3 externalForce(0.0f);
	if(glfwGetMouseButton(window, 0) == GLFW_PRESS)
	{
		externalForce.x += 1.0f;

	}
	if(glfwGetMouseButton(window, 1) == GLFW_PRESS)
	{
		externalForce.x -= 1.0f;
	}

	//Apply acceleration due to gravity to the objects
	//We will start at rigidBody 1 because the first rigidbody will be pinned to the mouse position.
	for(int i = 1; i < body->numRigidBodies; i++)
	{
		//Calculate force body[i-1] is applying to body[i]
		//Get displacement from rigidBody[i-1] to rigidBody[i]
		glm::vec3 displacement = body->rigidBodies[i - 1].position - body->rigidBodies[i].position;
		//Extract the direction and the magnitude from this displacement
		glm::vec3 direction = glm::normalize(displacement);
		float mag = glm::length(displacement);

		//Calculate and Add the applied force according the Hooke's law
		//Fspring = -k(dX)
		//And from that we must add the dampening force:
		//Fdamp = -V * C 
		//Where C is the dampening constant
		body->rigidBodies[i].netForce += body->coefficient * (mag - body->restLength) * direction - body->rigidBodies[i].velocity * body->dampening;

		//If there is a next rigidbody, perform the same computations on it.
		if(i < body->numRigidBodies-1)
		{
			displacement = body->rigidBodies[i + 1].position - body->rigidBodies[i].position;
			direction = glm::normalize(displacement);
			mag = glm::length(displacement);

			body->rigidBodies[i].netForce += body->coefficient * (mag - body->restLength) * direction - body->rigidBodies[i].velocity * body->dampening;
		}

		//Add any and all external forces below here here
		body->rigidBodies[i].netForce += gravity * body->rigidBodies[i].mass + externalForce;
	}


	//Update the rigidbodies & mesh
	//For each rigidbody
	for(int i = 1; i < body->numRigidBodies; i++)
	{
		//Integrate kinematics
		IntegrateLinear(dt, body->rigidBodies[i]);
		//Get it's position assuming the 0th body is the origin of the system
		glm::vec3 relativePosition =  body->rigidBodies[i].position - body->rigidBodies[0].position;
		//And change the mesh's vertices to match this
		rope->vertices[i].x = relativePosition.x;
		rope->vertices[i].y = relativePosition.y;
		rope->vertices[i].z = relativePosition.z;
	}
	//translate origin to first rigidbody in the system
	rope->translation = glm::translate(glm::mat4(1.0f), body->rigidBodies[0].position);

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

	//Refresh the rope vertices
	rope->RefreshData();
	// Draw the Gameobjects
	rope->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Mass Spring Softbody (1D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate the rope mesh
	float ropeArr[] = { 
		0.0f, 0.01f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 0.008f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 0.006f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 0.004f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 0.002f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, -0.002f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, -0.004f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, -0.006f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, -0.008f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, -0.010f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
	};


	Vertex *ropeVerts = (Vertex*)ropeArr;

	//rope creation
	rope = new struct Mesh(11, ropeVerts, GL_LINE_STRIP);

	//Scale the rope
	rope->scale = glm::scale(rope->scale, glm::vec3(1.0f));

	//Set spring constant, rest length, and dampening constant
	float coeff = 100.0f;
	float rest = 0.01f;
	float damp = 1.0f;

	//Generate the softbody
	body = new SoftBody(*rope, coeff, rest, damp);

	//Print controls
	printf("Controls:\nMove mouse to displace one end of rope.\nLeft click to cause wind to the right.\nRight click to cause wind to the left.\n");

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

	delete rope;
	delete body;


	// Frees up GLFW memory
	glfwTerminate();
}