/*
Title: Detecting the Point of Collision (Circle - 2D)
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
This is a demonstration resolving a strictly linear collision between two colliding circles in 2D.
There are Pink and Yellow circles. When the circles collide the collision is resolved in a physically accurate manner.
The yellow circle has twice the mass of the pink circle. Both circles have perfect elasticity resulting in
perfectly elastic collisions.

This demo resolves a collision by calculating the collision impulse needed to apply to both objects in opposite directions (Newtons third law)
in order to simulate a collision between two rigidbodies. We calculate this impulse by using the definition of an impulse- a change in momentum.
In order to calculate the impulse we must first find a final velocity after the collision which we can do using The user can move a circle using WASD.
Newton's law of Restitution. To make all of this easier, we first translate the system to a case where only one of the objects is moving
and relatively the other is still.

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

//Struct for circle collider
struct Circle
{
	float radius;
	glm::vec3 center;

	//Default constructor, creates unit circle at origin
	Circle::Circle()
	{
		center = glm::vec3(0.0f);
		radius = 1.0f;
	}

	//PArameterized constructor, creates circle from given center and radius
	Circle::Circle(const glm::vec3& c, float r)
	{
		center = c;
		radius = r;
	}
};

//Struct for linear kinematics
struct RigidBody
{
	float inverseMass;			//I tend to use inverse mass as opposed to mass itself. It saves lots of divides when forces are involved.
	float restitution;			//How elastic this object is (1.0f is perfectly elastic, 0.0f is perfectly inelastic)

	glm::vec3 position;			//Position of the rigidbody
	glm::vec3 velocity;			//The velocity of the rigidbody
	glm::vec3 acceleration;		//The acceleration of the rigidbody

	glm::vec3 netForce;			//Forces over time
	glm::vec3 netImpulse;		//Instantaneous forces


	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		inverseMass = 1.0f;
		restitution = 1.0f;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);
	}

	///
	//Parameterized constructor, creates rigidbody at certain position with certain velocity and acceleration
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, float mass, float coeffOfRestitution)
	{
		inverseMass = mass == 0.0f ? 0.0f : 1.0f / mass;
		restitution = coeffOfRestitution;

		position = pos;
		velocity = vel;
		acceleration = acc;

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);
	}
};

struct Mesh* circle1;
struct Mesh* circle2;

struct Circle* circle1Collider;
struct Circle* circle2Collider;

struct RigidBody* circle1Body;
struct RigidBody* circle2Body;

struct Circle* selectedCollider;

glm::vec2 minimumTranslationVector;
float overlap;

glm::vec2 pointOfCollision;

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.

//Declaration of function which is called on key press.
void OnKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods);

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
//Resolves a collision between two rigidbodies
//
//Overview:
//	Resolves a collision by calculating the collision impulse needed to apply to both objects in opposite directions (Newtons third law)
//	in order to simulate a collision between two rigidbodies. We calculate this impulse by using the definition of an impulse- a change in momentum.
//	In order to calculate the impulse we must first find a final velocity after the collision which we can do using 
//	Newton's law of Restitution. To make all of this easier, we first translate the system to a case where only one of the objects is moving
//	and relatively the other is still.
//
//Parameters:
//	body1: the rigidbody which the MTV points toward
//	body2: The rigidbody which the MTV points away
//	MTV: the minimum translation vector
//	pointOfCollision: The collision point between the two objects (Actually not needed in the strictly linear case).
void ResolveCollision(RigidBody &body1, RigidBody &body2, const glm::vec2 &MTV, const glm::vec2 &pointOfCollision)
{
	//Step 1: Compute the relative velocity of object 1 from the point of collision on object 2
	//The point of collision on object 2 must be moving with object 2's velocity. Therefore if we subtract the velocity of object 2 from
	//the velocity of object 1 we can relatively determine how fast object 1 is travelling from the point on object 2.
	//This effectively reduces our scenario to object 2 being stationary, and object 1 moving with this relative speed!
	glm::vec3 relativeVelocity = body1.velocity - body2.velocity;

	//Step 2: Determine the magnitude of the relative velocity in the direction of the collision.
	//Say you are riding a bike about 20 MPH, and you are riding past a long building. You take one hand off of the handle bars and slide it along
	//the wall as you ride by. Does it feel like you hit the wall going 20 MPH? No! Of Course not.
	//This is because, even though you are going 20 MPH relative to the point of collision on the wall, you are going ~0 MPH in the direction
	//of the collision normal-- or in the direction of the wall!
	//
	//We can determine this value by using the geometric properties of the dot product, because we know the MTV is already normalized.
	float relativeVelocityPerp = glm::dot(relativeVelocity, glm::vec3(MTV, 0.0f));	//Keep in mind, because the MTV by our convention always points toward object 1, and
	//We found the relative velocity of object 1, this should be negative if object 1 is moving
	//towards object 2 relatively (The objects are converging)

	//Step 3: Calculate the relative velocity after the collision
	//Newtons law of restitution states that e = |V2perp| / |V1perp|
	//Or more simply, the coefficient of restitution (e) of an object hitting a stationary object is the ratio of the 
	//final speed in the direction of the collision (|V2perp|) to the initial speed in the direction of the collision (|V1|).
	//This holds
	//We know our coefficient of restitution by the design of the game, so we can use this to compute the final relative velocity!
	//It should be noted that the final coefficient of restitution is the product of each objects created by design.
	float e = body1.restitution * body2.restitution;
	float finalRelativeVelocityPerp = -e * relativeVelocityPerp;	//Remember, the final velocity will have it's direction switched (So make e negative!) 

	//Step 4: Calculate the collision impulse
	//Assume there is some collision impulse, j, which is in the direction of the MTV that is going to alter the velocity of our rigidbodies.
	//And impulse is a change in momentum:
	//	j = m * dV 
	//	j = m * (VAfter - VBefore)
	//And we simply solved for VAfter.
	//
	//We can calculate the velocity after the collision for body1, V1after, as:
	//	V1After = body1.velocity + body1.inverseMass * j
	//
	//It is also worth noting, V2After, the velocity of body2 after the collision, will be nearly the same-- except in the opposite direction so we would have:
	//	V2After = body2.velocity - body2.inverseMass * j
	//
	//Furthermore, this relationship holds true for the relative velocity & the sum of the masses!
	//	j = (m1 + m2) * (VRelAfter - VRelBefore)
	//And because our impulse is only going to be in the direction of the collision, like we discussed earlier:
	//	j = (m1 + m2) * (finalRelativeVelocityPerp - relativeVelocityPerp)
	float j = (finalRelativeVelocityPerp - relativeVelocityPerp) / (body1.inverseMass + body2.inverseMass);

	//Newtons third law says for every action there is an equal and opposite reaction. This means that the impulse, j, we calculated is
	//the correct impulse for both objects-- simply the direction we apply it in changes!

	//Step 5: Apply the impulse
	//Remember, the MTV always points away from object 2, toward object 1 by our convention.
	//We can apply the impulse to body1 along the MTV, and the impulse to body2 away from the MTV.
	//(The MTV is our collision normal)
	glm::vec3 impulse = glm::vec3(j * MTV, 0.0f);
	body1.netImpulse += impulse;

	impulse *= -1.0f;
	body2.netImpulse += impulse;

}

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


///
//Determines the collision point of two circles in worlspace
//
//Parameters:
//	c1: The circle which the MTV points towards (By the established convention, first circle in collision)
//	MTV: The minimum translation vector of the collision
//	mag: The magnitude of overlap along the minimum translation vector
//
//Returns:
//	a vec2 containing the minimum translation vector in worldspace
glm::vec2 DetermineCollisionPoint(struct Circle &c1, const glm::vec2 &MTV, const float mag)
{
	glm::vec2 mtv = glm::vec2(c1.center) - c1.radius * MTV;
	return mtv;
}

///
//Separates two intersecting objects back to a state of contact
//
//Parameters:
//	c1: The first of the intersecting circles (Which the MTV points toward by our convention)
//	c2: The second of the intersecting circles (Which the MTV points away by our convention)
//	MTV: The axis of minimal translation needed to separate the circles
//	mag: The magnitude of overlap along the minimum translation vector
void DecoupleObjects(struct Circle &c1, struct Circle &c2, const struct RigidBody &body1, const struct RigidBody &body2, const glm::vec2 &MTV, const float &mag)
{
	//The first step in decoupling a pair of objects is to figure how much you must move each one.
	//you can do this by taking the sum of the magnitudes of their velocities along the MTV
	//And performing a ratio of each individual velocities magnitude along the MTV to that sum.
	//
	//For example, if we wanted to figure out how much to move circle c1:
	float individual1 = fabs(glm::dot(glm::vec2(body1.velocity), MTV));
	float individual2 = fabs(glm::dot(glm::vec2(body2.velocity), MTV));
	
	float sum = individual1 + individual2;
	float ratio1 = individual1/sum;
	float ratio2 = individual2/sum;

	//From here, we can figure out the magnitude of of how much to move poly 1 along the MTV by taking the product of the ratio with the magnitude of the overlap
	//	mag1 = ratio * mag
	float mag1, mag2;
	mag1 = ratio1 * mag;
	mag2 = ratio2 * mag;

	//Now, remember, the MTV always points toward object1, by the convention we established. So we want to move circle1 along the MTV, and circle2 opposite the MTV!
	c1.center += glm::vec3(mag1 * MTV, 0.0f);
	c2.center -= glm::vec3(mag2 * MTV, 0.0f);
}

///
//Determines if a collision needs to be resolved
//
//Parameters:
//	body1: The rigidbody of the object that the minimum translation vector points toward
//	body2: The rigidbody of the object that the minimum translation vector points away from
//	MTV: the minimum translation vector
bool IsResolutionNeeded(struct RigidBody &body1, struct RigidBody &body2, glm::vec2 &MTV)
{
	//Determine the relative velocity of object 2 from the Center of Mass of object 1
	glm::vec2 relativeVelocity = glm::vec2(body2.velocity - body1.velocity);
	//Check it's dot product with the MTV to make sure it is in the direction of the MTV
	if(glm::dot(MTV, relativeVelocity) > 0.0f)
	{
		return true;
	}
	//Else, the objects will resolve their own collision (They are moving away from each other)
	else
	{
		return false;
	}
}
///
//Checks for collision between two circles by seeing if the distance between them is less
//than the sum of the radii. If a collision is detected, stores the minimum translation vector and magnitude of overlap
//
//Parameters:
//	c1: The first circle to test
//	c2: The second circle to test
//	MTV: A reference to where to store the minimum translation vector in case of a collision
//	mag: A reference to where to store the magnitude of overlap along the minimum translation vector in case of an overlap
//
//Returns:
//	true if colliding, else false.
bool CheckCollision(const Circle &c1, const Circle &c2, glm::vec2 &MTV, float &mag)
{
	float dist = glm::length(c1.center - c2.center);
	if(dist < c1.radius + c2.radius)
	{
		//We have a collision, determine the Minimum Translation Vector
		MTV = glm::normalize(glm::vec2(c1.center - c2.center));
		mag = (c1.radius + c2.radius) - dist;
		return true;
	}
	return false;
}

///
//Wraps a moving circle around the edges of the screen
//
//Parameters:
//	body: The rigidbody of the circle
//	circle: The collider of the circle
void Wrap(struct RigidBody &body, struct Circle &circle)
{
	if(body.position.x + circle.radius < -1.0f)
		body.position.x = 1.0f + circle.radius;
	if(body.position.x - circle.radius > 1.0f)
		body.position.x = -1.0f - circle.radius;
	if(body.position.y + circle.radius < -1.0f)
		body.position.y = 1.0f + circle.radius;
	if(body.position.y - circle.radius > 1.0f)
		body.position.y = -1.0f - circle.radius;
}

// This runs once every physics timestep.
void update(float dt)
{	
	//Update the rigidbodies
	IntegrateLinear(dt, *circle1Body);
	IntegrateLinear(dt, *circle2Body);

	//Move the colliders
	circle1Collider->center = circle1Body->position;
	circle2Collider->center = circle2Body->position;

	//Check for collision
	if(CheckCollision(*circle1Collider, *circle2Collider, minimumTranslationVector, overlap))
	{
		//Check if collision resolution is necessary
		//If two objects are already moving away from each other, but in contact, we let them resolves themselves.
		if(IsResolutionNeeded(*circle1Body, *circle2Body, minimumTranslationVector))
		{

			//Decouple and find point of collision
			DecoupleObjects(*circle1Collider, *circle2Collider, *circle1Body, *circle2Body, minimumTranslationVector, overlap);
			pointOfCollision = DetermineCollisionPoint(*circle1Collider, minimumTranslationVector, overlap);

			//Reposition rigidbodies
			circle1Body->position = circle1Collider->center;
			circle2Body->position = circle2Collider->center;

			//Resolve collision
			ResolveCollision(*circle1Body, *circle2Body, minimumTranslationVector, pointOfCollision);

			
		}
	}

	Wrap(*circle1Body, *circle1Collider);
	Wrap(*circle2Body, *circle2Collider);

	//Move appearance to new position
	circle1->translation = glm::translate(glm::mat4(1.0f), circle1Body->position);
	circle2->translation = glm::translate(glm::mat4(1.0f), circle2Body->position);

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
	circle1->Draw();
	circle2->Draw();


}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Resolving Collisions (Linear - 2D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate the circle mesh
	float circleScale = 0.15f;
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
	circle1 = new struct Mesh(numVertices, circleVerts, GL_TRIANGLES);
	//Alter mesh for circle2
	for (int i = 0; i < numVertices; i++)
	{
		circleVerts[i].g = 0.0f;
		circleVerts[i].b = 1.0f;
	}
	//Circle2 creation
	circle2 = new struct Mesh(numVertices, circleVerts, GL_TRIANGLES);


	//Scale the circles
	circle1->scale = glm::scale(circle1->scale, glm::vec3(circleScale));
	circle2->scale = glm::scale(circle2->scale, glm::vec3(circleScale * 0.5f));


	//Generate the circle rigidbodies
	circle1Body = new RigidBody(
		glm::vec3(-0.75f, 0.05f, 0.0f),		//Initial position
		glm::vec3(0.2f, 0.0f, 0.0f),		//Initial velocity
		glm::vec3(0.0f),					//Zero acceleration
		1.0f,								//Mass
		1.0f								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		);
	circle2Body = new RigidBody(
		glm::vec3(0.75f, 0.0f, 0.0f), 		//Initial position
		glm::vec3(-0.2f, 0.0f, 0.0f), 		//Initial velocity
		glm::vec3(0.0f), 					//Zero acceleration
		0.5f, 								//Mass
		1.0f								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		);

	//Generate the circles colliders
	circle1Collider = new Circle(circle1Body->position, circleScale);
	circle2Collider = new Circle(circle2Body->position, circleScale * 0.5f);

	selectedCollider = circle1Collider;



	//Position circles
	circle1->translation = glm::translate(circle1->translation, circle1Collider->center);
	circle2->translation = glm::translate(circle2->translation, circle2Collider->center);

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

	delete circle1;
	delete circle2;
	delete circle1Collider;
	delete circle2Collider;
	delete circle1Body;
	delete circle2Body;

	// Frees up GLFW memory
	glfwTerminate();
}