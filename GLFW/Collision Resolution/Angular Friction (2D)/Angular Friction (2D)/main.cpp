/*
Title: Angular Friction (2D)
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
This is a demonstration of calculating and applying an angular frictional force to rigidbodies in
such a way that a rotation which is around an axis perpendicular to a surface can be diminished through friction. 
The demo contains a yellow circle which is supposed to be laying flat on the ground. The demo uses a modified version 
of the Coulomb Impulse-Based model of friction to simulate frictional forces between colliding bodies.

The algorithm Calculates & applies an angular frictional impulse similar to Coulomb's impulse based model of friction
however it has been adopted for angular friction. The algorithm calculates the friction due to flat-spin on a surface.
Because there is only one degree of angular freedom in a 2D game, we assume that the body is resting on another object and the surface
normal is in the Z direction.

The user can press and hold the spacebar to apply a constant torque to the circle.

References:
Gravitas: An extensible physics engine framework using object-oriented and design pattern-driven software architecture principles by Colin Vella, supervised by Dr. Ing. Adrian Muscat
NGen by Nicholas Gallagher
A Coulomb Based Model for Simulating Angular Friction Normal to a Surface by Nicholas Gallagher
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
	float inverseMass;				//I tend to use inverse mass as opposed to mass itself. It saves lots of divides when forces are involved.
	float inverseMomentOfInertia;	//An objects resistance to rotation
	float restitution;				//How elastic this object is (1.0f is perfectly elastic, 0.0f is perfectly inelastic)
	float dynamicFriction;			//Dynamic coefficient of Friction
	float staticFriction;			//Static coefficient of Friction

	glm::vec3 position;				//Position of the rigidbody
	glm::vec3 velocity;				//The velocity of the rigidbody
	glm::vec3 acceleration;			//The acceleration of the rigidbody

	glm::mat3 rotation;				//Orientation of rigidbody
	glm::vec3 angularVelocity;		//Angular velocity of rigidbody
	glm::vec3 angularAcceleration;	//Angular acceleration of rigidbody

	glm::vec3 netForce;				//Forces over time
	glm::vec3 netImpulse;			//Instantaneous forces
	float netTorque;				//Torque over time (2D -- only torque which exists is around Z axis)
	float netAngularImpulse;		//Instantaneous torque.

	//Note: The following is only for robustness.. You could easily get away with leaving these out and not taking them into account.
	glm::vec3 previousNetForce;		//Forces over time from prior to the most recent physics update
	glm::vec3 previousNetImpulse;	//Instantaneous forces from prior to the most recent physics update
	float previousNetTorque;		//Torques over time from prior to the most recent physics update
	float previousNetAngularImpulse;//Instantaneous torques from prior to the most recent physics update

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		inverseMass = 1.0f;
		restitution = 1.0f;

		dynamicFriction = 1.0f;
		staticFriction = 1.0f;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

		rotation = glm::mat3(1.0f);
		angularVelocity = glm::vec3(0.0f);
		angularAcceleration = glm::vec3(0.0f);

		previousNetForce = netForce = glm::vec3(0.0f);
		previousNetImpulse = netImpulse = glm::vec3(0.0f);
		previousNetTorque = netTorque = 0.0f;
		previousNetAngularImpulse = netAngularImpulse = 0.0f;
	}

	///
	//Parameterized constructor, creates rigidbody with specified initial values
	//
	//Parameters:
	//	pos: Initial position
	//	vel: Initial velocity
	//	acc: Initial acceleration
	//	rot: Initial orientation
	//	aVel: Initial angular velocity
	//	aAcc: Initial angular acceleration
	//	mass: The mass of the rigidbody (0.0f for infinite mass)
	//	coeffOfRestitution: How elastic is the rigidbody (1.0f for perfectly elastic, 0.0f for perfectly inelastic)
	//	dynamicC: The dynamic coefficient of friction
	//	staticC: the static coefficient of friction
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, glm::mat3 rot, glm::vec3 aVel, glm::vec3 aAcc,  float mass, float coeffOfRestitution, float dynamicC, float staticC)
	{
		inverseMass = mass == 0.0f ? 0.0f : 1.0f / mass;
		restitution = coeffOfRestitution;

		position = pos;
		velocity = vel;
		acceleration = acc;

		rotation = rot;
		angularVelocity = aVel;
		angularAcceleration = aAcc;

		previousNetForce = netForce = glm::vec3(0.0f);
		previousNetImpulse = netImpulse = glm::vec3(0.0f);
		previousNetTorque = netTorque = 0.0f;
		previousNetAngularImpulse = netAngularImpulse = 0.0f;

		dynamicFriction = dynamicC;
		staticFriction = staticC;
	}
};

struct Mesh* circle;
struct RigidBody* circleBody;


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
}

///
//Calculates the moment of inertia around the Z axis of a thin solid disk with a given radius and mass
float CalculateMomentOfInertiaOfCircle(float radius, float m)
{
	return  m * powf(radius, 2.0f) * 0.5f;
}

#pragma endregion Helper_functions

///
//Calculates & applies an angular frictional impulse similat to Coulomb's impulse based model of friction
//however it has been adopted for angular friction. The algorithm calculates the friction due to flat-spin on a surface.
//Because there is only one degree of angular freedom in a 2D game, we assume that the body is resting on another object and the surface
//normal is in the Z direction.
//
//Parameters:
//	body: The rigidbody to calculate angular friction on
//	dt: The physics timestep.
void ApplyAngularFriction(RigidBody &body1, float dt)
{
	//Step 1: find relative angular velocity of the "floor" with object
	glm::vec3 relativeAngularVelocity = -body1.angularVelocity;

	//Step 2: Determine the normal force magnitude between the two objects (We will assume this is top down-2D, and the normal force will be equal to the object's mass * force due to gravity
	//(If this were not a top down game then you would not have angular friction.. You would have what is called "Rolling Resistance" in a 2D side scroller where something is spinning.
	//Angular friction is the idea of slowing down a flatspin on a surface)
	float normalForceMag = (1.0f * dt) / body1.inverseMass;	//Hardcoded acceleration due to gravity to 1.0f

	//Step 3: Use the normal force to determine the magnitudes of static and dynamic friction
	float staticMag = normalForceMag * body1.staticFriction;
	float dynamicMag = normalForceMag * body1.dynamicFriction;

	//Step 4: Determine the relative angular velocity in the direction of the collision normal (Perpendicular to the surface)
	//In a 2D game the collision normal is the Z axis.. And the Z axis would contain the only rotation..
	float relativeAngularVelocityPerpendicular = relativeAngularVelocity.z;

	//Step 5: Compute the angular momentum
	float angularMomentum = relativeAngularVelocityPerpendicular / body1.inverseMomentOfInertia;

	//Step 6: Determine whether to apply static or dynamic friction
	if(fabs(angularMomentum) < fabs(staticMag))
	{
		//Cancel out motion
		body1.netAngularImpulse += angularMomentum;

	}
	else
	{
		//Apply impulse equal to dynamic mag opposing direction of motion
		body1.netAngularImpulse += dynamicMag * (fabs(angularMomentum) / angularMomentum);
	}

}

void IntegrateAngular(float dt, RigidBody &body)
{

	//Calculate new angular acceleration
	body.angularAcceleration = glm::vec3(0.0f, 0.0f, body.netTorque * body.inverseMomentOfInertia);

	//Find change in position with W0 * dt + (1/2)aA * dt ^ 2
	//Where W0 is initial angular velocity, and aA is angular acceleration
	glm::vec3 dr = dt * body.angularVelocity + 0.5f * powf(dt, 2.0f) * body.angularAcceleration;

	float magR = glm::length(dr);
	if(magR > 0.0f)
	{
		glm::mat3 R = glm::mat3(glm::rotate(glm::mat4(1.0f), magR, dr));

		body.rotation = R * body.rotation;

	}
	body.angularVelocity += dt * body.angularAcceleration + glm::vec3(0.0f, 0.0f, body.netAngularImpulse * body.inverseMomentOfInertia);

	//Set previous torque and angular impulse
	body.previousNetTorque = body.netTorque;
	body.previousNetAngularImpulse = body.netAngularImpulse;

	body.netTorque = body.netAngularImpulse = 0.0f;
}

// This runs once every physics timestep.
void update(float dt)
{	

	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		//Apply a torque when spacebar is pressed
		circleBody->netTorque -= 0.05f;
	}


	//Update the rigidbodies
	IntegrateAngular(dt, *circleBody);

	//In a top down 2D game, we will assume the object is on the floor.
	//Apply angular friction
	ApplyAngularFriction(*circleBody, dt);

	//translate appearance to new position/orientation
	circle->translation = glm::translate(glm::mat4(1.0f), circleBody->position);
	circle->rotation = glm::mat4(circleBody->rotation);

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
	circle->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Angular Friction (2D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate the circle mesh
	float circleScale = 0.4f;
	int numVertices = 72;
	struct Vertex circleVerts[72];
	float stepSize = 2.0f * 3.14159 / (numVertices/3.0f);
	int vertexNumber = 0;
	for (int i = 0; i < numVertices; i++)
	{
		circleVerts[i].x = cosf(vertexNumber * stepSize);
		circleVerts[i].y = sinf(vertexNumber * stepSize);
		circleVerts[i].z = 0.0f;
		circleVerts[i].r = 1.0f;
		circleVerts[i].g = 1.0f;
		circleVerts[i].b = 0.0f;
		circleVerts[i].a = 1.0;
		++i;
		++vertexNumber;
		circleVerts[i].x = cosf(vertexNumber * stepSize);
		circleVerts[i].y = sinf(vertexNumber * stepSize);
		circleVerts[i].z = 0.0f;
		circleVerts[i].r = 1.0f;
		circleVerts[i].g = 1.0f;
		circleVerts[i].b = 0.0f;
		circleVerts[i].a = 1.0f;
		++i;
		circleVerts[i].x = 0.0f;
		circleVerts[i].y = 0.0f;
		circleVerts[i].z = 0.0f;
		circleVerts[i].r = 1.0f;
		circleVerts[i].g = 1.0f;
		circleVerts[i].b = 0.0f;
		circleVerts[i].a = 1.0f;
	}

	//circle1 creation
	circle = new struct Mesh(numVertices, circleVerts, GL_LINE_LOOP);


	//Generate the circle rigidbodies
	circleBody = new RigidBody(
		glm::vec3(0.0f, 0.0f, 0.0f),		//Initial position
		glm::vec3(0.0f, 0.0f, 0.0f),		//Initial velocity
		glm::vec3(0.0f),					//Zero acceleration
		glm::mat3(1.0f),					//Initial orientation
		glm::vec3(0.0f, 0.0f, 0.0f),		//Initial angular velocity
		glm::vec3(0.0f),					//Initial Angular Acceleration
		0.1f,								//Mass
		1.0f,								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		0.3f,								//Dynamic coefficient of Friction
		0.5f								//Static coefficient of friction
		);
	circleBody->inverseMomentOfInertia = circleBody->inverseMass == 0.0f ?
		0.0f : 1.0f/CalculateMomentOfInertiaOfCircle(circleScale, 1.0f / circleBody->inverseMass);



	//Position circle
	circle->translation = glm::translate(circle->translation, circleBody->position);

	std::cout << "Controls:\nPress and hold spacebar to apply a constant torque to the circle.\n";

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
	delete circleBody;

	// Frees up GLFW memory
	glfwTerminate();
}