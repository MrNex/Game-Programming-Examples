/*
Title: Mass Spring Softbody (2D Fast)
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
The demo contains a blue cloth made of a 50x50 grid of masses with springs connecting them.

Each physics timestep the mass spring system is solved to determine the force on each
individual point mass in the system. This is done using Hooke's law. The springs also contain 
dampening forces to help relax the system upon purterbation.

The user can apply forces to the bottom edge of the cloth.
Hold the left mouse button to apply a force along the positive X axis.
Hold the right mouse button to apply a force along the negative X axis.
Hold Left shift to switch the axis to the Y axis.

It should be noted that this is a more complex example focused on increasing the speed of 
the iterative algorithm. For a straight forward demonstation see the Mass Spring (2D) example.

References:
Game Physics by David Eberly
NGenVS by Nicholas Gallagher
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
	GLuint EBO;
	GLuint VAO;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	int numVertices;
	int numIndices;
	struct Vertex* vertices;
	GLuint* indices;
	GLenum primitive;

	Mesh::Mesh(int numVert, struct Vertex* vert, int numInd, GLuint* inds, GLenum primType)
	{

		glm::mat4 translation = glm::mat4(1.0f);
		glm::mat4 rotation = glm::mat4(1.0f);
		glm::mat4 scale = glm::mat4(1.0f);

		this->numVertices = numVert;
		this->vertices = new struct Vertex[this->numVertices];
		memcpy(this->vertices, vert, this->numVertices * sizeof(struct Vertex));

		this->numIndices = numInd;
		this->indices = new GLuint[this->numIndices];
		memcpy(this->indices, inds, this->numIndices * sizeof(GLuint));

		this->primitive = primType;

		//Generate VAO
		glGenVertexArrays(1, &this->VAO);
		//bind VAO
		glBindVertexArray(VAO);

		//Generate VBO & EBO
		//We must use an element buffer here so that we do not need to worry about duplicate vertices
		//while we are repositioning the vertices of the mesh
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);

		//Configure VBO & EBO
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * this->numVertices, this->vertices, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * this->numIndices, this->indices, GL_STATIC_DRAW);

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
		//glDrawArrays(this->primitive, 0, this->numVertices);
		glDrawElements(this->primitive, this->numIndices, GL_UNSIGNED_INT, 0);
	}
};

//Struct for rigidbody kinematics
struct RigidBody
{
	float mass;						//We will need to use both mass and inverse mass extensively here.
	float inverseMass;				//I tend to use inverse mass as opposed to mass itself. It saves lots of divides when forces are involved.

	float* position;				//Position of the rigidbody
	float* velocity;				//The velocity of the rigidbody
	float* acceleration;			//The acceleration of the rigidbody

	float* netForce;				//Forces over time
	float* netImpulse;			//Instantaneous forces

	///
	//Parameterized constructor, creates rigidbody with specified initial values
	//
	//Parameters:
	//	pos: Initial position
	//	vel: Initial velocity
	//	acc: Initial acceleration
	//	mass: The mass of the rigidbody (0.0f for infinite mass)
	RigidBody::RigidBody(float* pos, float vel[3], float acc[3], float m)	
	{
		mass = m;
		inverseMass = m == 0.0f ? 0.0f : 1.0f / m;

		position = pos;
		velocity = new float[3];
		acceleration = new float[3];
		memcpy(velocity, vel, sizeof(float) * 3);
		memcpy(acceleration, acc, sizeof(float) * 3);

		netForce = new float[3];
		netImpulse = new float[3];
		memset(netForce, 0, sizeof(float) * 3);
		memset(netImpulse, 0, sizeof(float) * 3);
	}

	RigidBody::~RigidBody()
	{
		delete[] netForce;
		delete[] netImpulse;
		delete[] acceleration;
		delete[] velocity;
	}
};

//A struct for 1D Mass-Spring softbody physics
struct SoftBody
{
	int subdivisionsX; 
	int subdivisionsY;

	float restHeight;
	float restWidth;

	//The rigidbodies of the point masses which make up the softbody mass-spring system
	unsigned int numRigidBodies;
	struct RigidBody*** bodies;

	float coefficient;	//The spring coefficients between the point masses in the system
	//float restLength;	//The resting length of the springs
	float dampening;	//The dampening coefficient of the springs

	SoftBody::SoftBody()
	{
		numRigidBodies = 0;
		coefficient = 0.0f;
		//restLength = 0.0f;
		dampening = 0.0f;

		subdivisionsX = subdivisionsY = 0;
		restHeight = restWidth = 0;
	}

	SoftBody::SoftBody(
		const Mesh& m,
		float rWidth, float rHeight, 
		int subX, int subY,
		float coeff, float damp
		)
	{
		subdivisionsX = subX;
		subdivisionsY = subY;

		numRigidBodies = subX * subY;
		coefficient = coeff;
		//restLength = rest;
		dampening = damp;

		restHeight = rHeight;
		restWidth = rWidth;

		float zero[3] = {0.0f, 0.0f, 0.0f};

		bodies = new struct RigidBody**[subY];
		for(int i = 0; i < subdivisionsY; ++i)
		{
			bodies[i] = new struct RigidBody*[subX];
			for(int j = 0; j < subdivisionsX; ++j)
			{
				bodies[i][j] = new struct RigidBody((float*)(m.vertices + i * subdivisionsY + j), zero, zero, 1.0f);
			}
		}


	}

	SoftBody::~SoftBody()
	{
		for(int i = 0; i < subdivisionsY; ++i)
		{
			for(int j = 0; j < subdivisionsX; ++j)
			{
				delete bodies[i][j];
			}
			delete[] bodies[i];
		}
		delete[] bodies;
	}
};

struct Mesh* lattice;

struct SoftBody* body;

//glm::vec3 gravity(0.0f, -0.98f, 0.0f);

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
	*(body.acceleration) = body.inverseMass * *(body.netForce);
	*(body.acceleration + 1) = body.inverseMass * *(body.netForce + 1);
	*(body.acceleration + 2) = body.inverseMass * *(body.netForce + 2);


	//Calculate new position with
	//	X = X0 + V0*dt + (1/2) * A * dt^2
	float v0dT[3] = {dt * *(body.velocity), dt * *(body.velocity + 1), dt * *(body.velocity + 2)};	//Solve for the first degree term
	float hdt2 = 0.5f * dt * dt;
	float aT2[3] = {*(body.acceleration) * hdt2, *(body.acceleration + 1) * hdt2, *(body.acceleration + 2) * hdt2};	//Solve for the second degree term
	//Take sum
	*(body.position) += *(v0dT) + *(aT2);
	*(body.position + 1) += *(v0dT + 1) + *(aT2 + 1);
	*(body.position + 2) += *(v0dT + 2) + *(aT2 + 2);

	//determine the new velocity
	*(body.velocity) += dt * *(body.acceleration) + body.inverseMass * *(body.netImpulse);
	*(body.velocity + 1) += dt * *(body.acceleration + 1) + body.inverseMass * *(body.netImpulse + 1);
	*(body.velocity + 2) += dt * *(body.acceleration + 2) + body.inverseMass * *(body.netImpulse + 2);


	//Zero the net impulse and net force!
	memset(body.netForce, 0, 12);
	memset(body.netImpulse, 0, 12);
}

// This runs once every physics timestep.
void update(float dt)
{	

	//This is the external force we will apply based on which keys are pressed.
	float externalForce[3] = {0.0f, 0.0f, 0.0f};


	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if(glfwGetMouseButton(window, 0) == GLFW_PRESS)
		{
			*(externalForce + 1) = 5.0f;
		}
		if(glfwGetMouseButton(window, 1) == GLFW_PRESS)
		{
			*(externalForce + 1) = -5.0f;
		}
	}
	else
	{
		if(glfwGetMouseButton(window, 0) == GLFW_PRESS)
		{
			*(externalForce) = 5.0f;
		}
		if(glfwGetMouseButton(window, 1) == GLFW_PRESS)
		{
			*(externalForce) = -5.0f;
		}
	}

	float displacement[3];	//The displacement between nodes
	float direction[3];
	float mag;

	int i = 0, j = 0;

	//Apply forces to each rigidbody making up the softbody
	while(i < body->subdivisionsY)
	{



		//If there is a rigidbody above this one, calculate the spring force between this rigidbody and the rigidbody above
		if(i > 0)
		{
			int finalI = i - 1, finalJ = j;
			//Get displacement from rigidBody[i-1] to rigidBody[i]
			*(displacement) = *((*(*(body->bodies + finalI) + finalJ))->position) - *((*(*(body->bodies + i) + j))->position);
			*(displacement + 1) = *((*(*(body->bodies + finalI) + finalJ))->position + 1) - *((*(*(body->bodies + i) + j))->position + 1);
			*(displacement + 2) = *((*(*(body->bodies + finalI) + finalJ))->position + 2) - *((*(*(body->bodies + i) + j))->position + 2);
			//Extract the direction and the magnitude from this displacement
			mag = sqrtf(*(displacement)**(displacement)+*(displacement+1)**(displacement+1)+*(displacement+2)**(displacement+2));
			*(direction) = *(displacement)/mag;
			*(direction + 1) = *(displacement + 1)/mag;
			*(direction + 2) = *(displacement + 2)/mag;



			//Calculate and Add the applied force according the Hooke's law
			//Fspring = -k(dX)
			//And from that we must add the dampening force:
			//Fdamp = -V * C 
			//Where C is the dampening constant
			*((*(*(body->bodies + i) + j))->netForce) += body->coefficient * (*(displacement) -  body->restHeight * *(direction)) - *((*(*(body->bodies + i) + j))->velocity) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 1) += body->coefficient * (*(displacement + 1) -  body->restHeight * *(direction + 1)) - *((*(*(body->bodies + i) + j))->velocity + 1) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 2) += body->coefficient * (*(displacement + 2) -  body->restHeight * *(direction + 2)) - *((*(*(body->bodies + i) + j))->velocity + 2) * body->dampening;

		}
		else
		{
			*((*(*(body->bodies + i) + j))->netForce) += *(externalForce);
			*((*(*(body->bodies + i) + j))->netForce + 1) += *(externalForce + 1);
			*((*(*(body->bodies + i) + j))->netForce + 2) += *(externalForce + 2);
		}

		//If there is a rigidbody below this one, calculate the spring force between this rigidbody and the one below
		if(i < body->subdivisionsY - 1)
		{
			int finalI = i + 1, finalJ = j;
			//Get displacement from rigidBody[i-1] to rigidBody[i]
			*(displacement) = *((*(*(body->bodies + finalI) + finalJ))->position) - *((*(*(body->bodies + i) + j))->position);
			*(displacement + 1) = *((*(*(body->bodies + finalI) + finalJ))->position + 1) - *((*(*(body->bodies + i) + j))->position + 1);
			*(displacement + 2) = *((*(*(body->bodies + finalI) + finalJ))->position + 2) - *((*(*(body->bodies + i) + j))->position + 2);
			//Extract the direction and the magnitude from this displacement
			mag = sqrtf(*(displacement)**(displacement)+*(displacement+1)**(displacement+1)+*(displacement+2)**(displacement+2));
			*(direction) = *(displacement)/mag;
			*(direction + 1) = *(displacement + 1)/mag;
			*(direction + 2) = *(displacement + 2)/mag;



			//Calculate and Add the applied force according the Hooke's law
			//Fspring = -k(dX)
			//And from that we must add the dampening force:
			//Fdamp = -V * C 
			//Where C is the dampening constant
			*((*(*(body->bodies + i) + j))->netForce) += body->coefficient * (*(displacement) -  body->restHeight * *(direction)) - *((*(*(body->bodies + i) + j))->velocity) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 1) += body->coefficient * (*(displacement + 1) -  body->restHeight * *(direction + 1)) - *((*(*(body->bodies + i) + j))->velocity + 1) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 2) += body->coefficient * (*(displacement + 2) -  body->restHeight * *(direction + 2)) - *((*(*(body->bodies + i) + j))->velocity + 2) * body->dampening;
		}

		//If there is a rigidbody left of this one, calculate the spring force between this rigidbody and the one below
		if(j > 0)
		{
			int finalI = i, finalJ = j - 1;
			//Get displacement from rigidBody[i-1] to rigidBody[i]
			*(displacement) = *((*(*(body->bodies + finalI) + finalJ))->position) - *((*(*(body->bodies + i) + j))->position);
			*(displacement + 1) = *((*(*(body->bodies + finalI) + finalJ))->position + 1) - *((*(*(body->bodies + i) + j))->position + 1);
			*(displacement + 2) = *((*(*(body->bodies + finalI) + finalJ))->position + 2) - *((*(*(body->bodies + i) + j))->position + 2);
			//Extract the direction and the magnitude from this displacement
			mag = sqrtf(*(displacement)**(displacement)+*(displacement+1)**(displacement+1)+*(displacement+2)**(displacement+2));
			*(direction) = *(displacement)/mag;
			*(direction + 1) = *(displacement + 1)/mag;
			*(direction + 2) = *(displacement + 2)/mag;



			//Calculate and Add the applied force according the Hooke's law
			//Fspring = -k(dX)
			//And from that we must add the dampening force:
			//Fdamp = -V * C 
			//Where C is the dampening constant
			*((*(*(body->bodies + i) + j))->netForce) += body->coefficient * (*(displacement) -  body->restWidth * *(direction)) - *((*(*(body->bodies + i) + j))->velocity) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 1) += body->coefficient * (*(displacement + 1) -  body->restWidth * *(direction + 1)) - *((*(*(body->bodies + i) + j))->velocity + 1) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 2) += body->coefficient * (*(displacement + 2) -  body->restWidth * *(direction + 2)) - *((*(*(body->bodies + i) + j))->velocity + 2) * body->dampening;
		}

		//If there is a rigidbody right of this one, calculate the spring force between this rigidbody and the one below
		if(j < body->subdivisionsX - 1)
		{
			int finalI = i, finalJ = j + 1;
			//Get displacement from rigidBody[i-1] to rigidBody[i]
			*(displacement) = *((*(*(body->bodies + finalI) + finalJ))->position) - *((*(*(body->bodies + i) + j))->position);
			*(displacement + 1) = *((*(*(body->bodies + finalI) + finalJ))->position + 1) - *((*(*(body->bodies + i) + j))->position + 1);
			*(displacement + 2) = *((*(*(body->bodies + finalI) + finalJ))->position + 2) - *((*(*(body->bodies + i) + j))->position + 2);
			//Extract the direction and the magnitude from this displacement
			mag = sqrtf(*(displacement)**(displacement)+*(displacement+1)**(displacement+1)+*(displacement+2)**(displacement+2));
			*(direction) = *(displacement)/mag;
			*(direction + 1) = *(displacement + 1)/mag;
			*(direction + 2) = *(displacement + 2)/mag;



			//Calculate and Add the applied force according the Hooke's law
			//Fspring = -k(dX)
			//And from that we must add the dampening force:
			//Fdamp = -V * C 
			//Where C is the dampening constant
			*((*(*(body->bodies + i) + j))->netForce) += body->coefficient * (*(displacement) -  body->restWidth * *(direction)) - *((*(*(body->bodies + i) + j))->velocity) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 1) += body->coefficient * (*(displacement + 1) -  body->restWidth * *(direction + 1)) - *((*(*(body->bodies + i) + j))->velocity + 1) * body->dampening;
			*((*(*(body->bodies + i) + j))->netForce + 2) += body->coefficient * (*(displacement + 2) -  body->restWidth * *(direction + 2)) - *((*(*(body->bodies + i) + j))->velocity + 2) * body->dampening;
		}	

		//Note: We lose some realism by performing the integration in this loop.
		//For a more realistic simulation this should be in a separate loop, however we save a lot of time.
		IntegrateLinear(dt, (*(*(*(body->bodies + i) + j))));


		++j;
		if(j == body->subdivisionsX) 
		{
			++i; 
			j = 0;
		}

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
	glLineWidth(1.0f);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	//Refresh the rope vertices
	lattice->RefreshData();
	// Draw the Gameobjects
	lattice->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Mass Spring Softbody (2D Fast)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	const int subX = 100;
	const int subY = 100;

	//Generate the rope mesh
	float latticeArr[subX * subY * (sizeof(struct Vertex) / sizeof(float))];
	for(int i = 0; i < subY; i++)
	{
		for(int j = 0; j < subX; j++)
		{
			int offset = sizeof(struct Vertex) / sizeof(float);
			int index = (i * subX + j) * offset;
			Vertex* v = (Vertex*)(latticeArr + index);
			v->x = (1.0f / subX) * (float)j;
			v->y = (1.0f / subY) * (float)i;
			v->z = 0.0f;

			v->r = 0.0f;
			v->g = 1.0f;
			v->b = 1.0f;
			v->a = 1.0f;
		}
	}

	Vertex *latticeVerts = (Vertex*)latticeArr;

	const int numIndices = (subX - 1) * (subY - 1) * 4;
	GLuint latticeElems[numIndices];
	for(int i = 0; i < subY - 1; i++)
	{
		for(int j = 0; j < subX - 1; j++)
		{
			int element = (i * subX + j) * 4 - 4 * i;
			latticeElems[element] = (i * subX) + j;
			latticeElems[element + 1] = (i * subX) + (j + 1);
			latticeElems[element + 2] = ((i + 1) * subX) + (j + 1);
			latticeElems[element + 3] = ((i + 1) * subX) + j;
		}
	}

	//lattice creation
	lattice = new struct Mesh(subX * subY, latticeVerts, numIndices, latticeElems, GL_QUADS);

	//Scale the lattice
	lattice->scale = glm::scale(lattice->scale, glm::vec3(1.0f));

	//Set spring constant, rest length, and dampening constant
	float coeff = 100.0f;
	float damp = 0.75f;

	//Generate the softbody
	body = new SoftBody(*lattice, 1.0f/subX, 1.0f/subY, subX, subY, coeff, damp);

	//Print controls
	printf("Controls:\nPress and hold the left mouse button to cause a positive constant force\n along the selected axis.\n");
	printf("Press and hold the right mouse button to cause a negative constant force\n along the selected axis.\n");
	printf("The selected axis by default is the X axis\n");
	printf("Hold Left Shift to change the selected axis to the Y axis\n");


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

	delete lattice;
	delete body;


	// Frees up GLFW memory
	glfwTerminate();
}