/*
Title: Rolling (2D - Angular to Linear)
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
This is a demonstration of calculating and applying a linear frictional force to colliding rigidbodies in
such a way that rolling can be facilitated. The demo contains a yellow circle & a pink rectangle. The pink rectangle is
supposed to be the ground, which in turn has infinite mass. The demo uses the Coulomb Impulse-Based model of friction to
simulate frictional forces between colliding bodies.

The algorithm Calculates & applies a linear frictional impulse according to Coulomb's impulse based model of friction.
This includes calculations of the linear velocity at the point of collision due to angular motion of an object to facilitate a rolling motion.
This algorithm only takes into account translating angular momentum to linear momentum, not the other way around.
I.E.: A Spinning block will begin to roll (translationally slide) while a sliding block will not begin to spin.

References:
Gravitas: An extensible physics engine framework using object-oriented and design pattern-driven software architecture principles by Colin Vella, supervised by Dr. Ing. Adrian Muscat
NGen by Nicholas Gallagher
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

struct AABB
{
	float width, height;
	glm::vec2 center;

	AABB::AABB()
	{
		width = height = 1.0f;
		center = glm::vec2(0.0f);
	}

	AABB::AABB(const glm::vec2 &c, float w, float h)
	{
		center = c;
		width = w;
		height = h;
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

	//Note: The following is only for robustness.. You could get away with leaving these out.
	glm::vec3 previousNetForce;		//Forces over time from prior to the most recent physics update
	glm::vec3 previousNetImpulse;	//Instantaneous forces from prior to the most recent physics update

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
		netTorque = 0.0f;
		netAngularImpulse = 0.0f;
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
		netTorque = 0.0f;
		netAngularImpulse = 0.0f;

		dynamicFriction = dynamicC;
		staticFriction = staticC;
	}
};

struct Mesh* circle;
struct Mesh* ground;

struct Circle* circleCollider;
struct AABB* groundCollider;

struct RigidBody* circleBody;
struct RigidBody* groundBody;

glm::vec2 minimumTranslationVector;
float overlap;
glm::vec2 pointOfCollision;

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

///
//Calculates the moment of inertia around the Z axis of a thin solid rectangle with given dimensions and mass
float CalculateMomentOfInertiaOfRectangle(float width, float height, float m)
{
	return  m * (powf(width, 2.0f)  + powf(height, 2.0f)) / 12.0f;
}

#pragma endregion Helper_functions

///
//Calculates & applies a linear frictional impulse according to Coulomb's impulse based model of friction
//Includes calculations of the linear velocity at the point of collision due to angular motion of an object to facilitate a rolling motion.
//This algorithm only takes into account translating angular momentum to linear momentum, not the other way around.
//I.E.: A Spinning block will begin to roll (translationally slide) while a sliding block will not begin to spin.
//
//Parameters:
//	body1: The first rigidbody involved in the collision (And the rigidbody which the MTV points toward by our convention)
//	body2: The second rigidbody involved in the collision (And the rigidbody which the MTV points away from by our convention)
//	MTV: the minimum translation vector (Or collision normal)
//	collisionPoint: The point of collision
void ApplyLinearFriction(RigidBody &body1, RigidBody &body2, const glm::vec2 &MTV, const glm::vec2 &collisionPoint)
{
	//Step 0: Due to complications of simulating friction we must come up with a minimum velocity low enough for objects to be considered static at.
	//It will be very difficult for an object's velocity to hit exactly zero obeying strict laws of friction. However, if we
	//are able to consider a small range of velocities which we consider static we can easily deal with the problem.
	static float tolerance = 0.01f;	//Anything this speed or less will be considered as non moving.

	//Step 1: compute the dynamic and static coefficients of restitution between the two objects
	//I like to compute the coefficients as the average, but there are many other ways. Some people prefer to use pythagorean's theorem, others prefer to just multiply them.
	float dynamicCoefficient = 0.5f * (body1.dynamicFriction + body2.dynamicFriction);
	float staticCoefficient = 0.5f * (body1.staticFriction + body2.staticFriction);

	//Step 2: Find the direction of a tangent vector which is tangential to the surrface of collision in the direction of movement (or impending movement if there is no movement yet)
	glm::vec2 unitTangentVector;

	//Determine the relative velocity of body2 from body1
	//To do this we must determine the total velocity at the point of collision on both objects (Including linear velocity due to angular motion)
	glm::vec2 totalVel1, totalVel2;
	glm::vec2 radius1, radius2;
	radius1 = collisionPoint - glm::vec2(body1.position);
	radius2 = collisionPoint - glm::vec2(body2.position);

	totalVel2 = glm::vec2(body2.velocity + glm::cross(body2.angularVelocity, glm::vec3(radius2, 0.0f)));
	totalVel1 = glm::vec2(body1.velocity + glm::cross(body1.angularVelocity, glm::vec3(radius1, 0.0f)));

	glm::vec2 relativeVelocity = totalVel2 - totalVel1;

	//Determine the relative velocity perpendicular to the surface
	glm::vec2 perpRelVelocity = glm::dot(relativeVelocity, MTV) * MTV;
	//Determine the relative velocity tangent to the surface
	unitTangentVector = relativeVelocity - perpRelVelocity;

	//If there is no relative motion parallel to the surface
	if(glm::length(unitTangentVector) < FLT_EPSILON)
	{
		//We must use the relative sum of all external impulses on body2 with respect to body1.
		glm::vec2 relativeImpulse;
		relativeImpulse += glm::vec2(body2.previousNetImpulse);
		relativeImpulse -= glm::vec2(body1.previousNetImpulse);

		//Determine the relative impulse which is perpendicular to the surface
		glm::vec2 perpRelImpulse = glm::dot(relativeImpulse, MTV) * MTV;
		//Determine the relative impulse which is parallel to the surface
		unitTangentVector = relativeImpulse - perpRelImpulse;

		//If this is zero, we must finally resort to using the net forces
		if(glm::length(unitTangentVector) < FLT_EPSILON)
		{
			glm::vec2 relativeForces;
			relativeForces += glm::vec2(body2.previousNetForce);
			relativeForces -= glm::vec2(body1.previousNetForce);

			//Determine the relative force which is perpendicular to the surface
			glm::vec2 perpRelForces = glm::dot(relativeForces, MTV) * MTV;
			unitTangentVector = relativeForces - perpRelForces;
		}
	}

	//Normalize the unit tangent vector
	if(glm::length(unitTangentVector) > FLT_EPSILON) unitTangentVector = glm::normalize(unitTangentVector);
	else unitTangentVector = glm::vec2(0.0f);	//If tangent is 0, there is no motion or impending motion- and therefore no friction.

	//Step 2) Compute the static and dynamic frictional force magnitudes
	//Compute the collision force perpendicular to the surface
	float reactionMag = fabs(glm::dot(glm::vec2(body1.netImpulse), MTV));

	//Compute the magnitudes using |Ff| = coefficient * |fNormal|
	float dynamicMag = dynamicCoefficient * reactionMag;
	float staticMag = staticCoefficient * reactionMag;

	//Step 3) Compute and apply the frictional impulse
	//Start by computing the magnitude of velocity parallel to the surface
	float relVelocityTangential = glm::dot(relativeVelocity, unitTangentVector);


	//First calculate and apply object 1's impulse
	glm::vec2 frictionalImpulse;
	float angularFrictionalImpulse;
	//From Ff = coefficient * fNormal we must first determine if we are dealing with static or dynamic friction
	//In an impulse based model we have jFriction = coefficient * jNormal for both the static and dynamic cases.
	//
	//If the rigid body is not yet moving we have a static friction case
	if(fabs(relVelocityTangential) < tolerance)
	{
		//An impulse, j, can be related to velocity by:
		//	j = mv
		//So we can calculate the impulse along a surface by:
		//	mv . unitTangentVector
		//or, equivilently
		//	relVelocityTangential * mass
		float impulseMag = body1.inverseMass == 0.0f ? 0.0f : relVelocityTangential / body1.inverseMass;
		//if the magnitude of impending motion along the surface is less than the static magnitude of friction then we must negate the motion with the proper impulse
		if(impulseMag < staticMag)
		{
			frictionalImpulse = unitTangentVector * impulseMag;
		}
		else
		{
			//If the impending motion overcomes the static magnitude
			//Apply an impulse equal to the static magnitude in the proper direction
			frictionalImpulse = unitTangentVector * staticMag;
		}
	}
	else
	{
		//If there is notion, we have a dynamic case. We can simply apply an impulse with magnitude
		//equal to the dynamic mag in the proper direction

		//If this impulse will overcome and change the direction of the current velocity, we must limit it.
		float impulseMag = body1.inverseMass == 0.0f ? 0.0f : relVelocityTangential / body1.inverseMass;
		if(impulseMag < dynamicMag)
		{
			frictionalImpulse = unitTangentVector * impulseMag;
		}
		else
		{
			frictionalImpulse = unitTangentVector * dynamicMag;
		}
	}
	body1.netImpulse += glm::vec3(frictionalImpulse, 0.0f);

	//Repeat for body2, but opposite direction!
	if(fabs(relVelocityTangential) < tolerance)
	{
		float impulseMag = body2.inverseMass == 0.0f ? 0.0f : relVelocityTangential / body2.inverseMass;
		if(impulseMag < staticMag)
		{
			frictionalImpulse = unitTangentVector * impulseMag;
		}
		else
		{
			frictionalImpulse = unitTangentVector * staticMag;
		}
	}
	else
	{
		//If this impulse will overcome and change the direction of the current velocity, we must limit it.
		float impulseMag = body1.inverseMass == 0.0f ? 0.0f : relVelocityTangential / body1.inverseMass;
		if(impulseMag < dynamicMag)
		{
			frictionalImpulse = unitTangentVector * impulseMag;
		}
		else
		{
			frictionalImpulse = unitTangentVector * dynamicMag;
		}
	}
	body2.netImpulse -= glm::vec3(frictionalImpulse, 0.0f);
}

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
void ResolveCollision(RigidBody &body1, RigidBody &body2, const glm::vec2 &MTV, const glm::vec2 &collisionPoint)
{
	//Step 1: Compute the relative velocity of object 1 from the point of collision on object 2
	glm::vec3 radius1 = glm::vec3(collisionPoint, 0.0f) - body1.position;
	glm::vec3 radius2 = glm::vec3(collisionPoint, 0.0f) - body2.position;

	glm::vec3 velTotal1 = body1.velocity + glm::cross(body1.angularVelocity, radius1);
	glm::vec3 velTotal2 = body2.velocity + glm::cross(body2.angularVelocity, radius2);

	glm::vec3 relativeVelocity = velTotal1 - velTotal2;

	//Step 2: Determine the magnitude of the relative velocity in the direction of the collision.
	float relativeVelocityPerp = glm::dot(relativeVelocity, glm::vec3(MTV, 0.0f));


	//Step 3: Calculate the relative velocity after the collision
	float e = body1.restitution * body2.restitution;
	float finalRelativeVelocityPerp = -e * relativeVelocityPerp;	//Remember, the final velocity will have it's direction switched (So make e negative!) 

	//Step 4: Calculate the collision impulse
	glm::vec3 perpRadius1 = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), radius1);
	glm::vec3 perpRadius2 = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), radius2);
	float j = (finalRelativeVelocityPerp - relativeVelocityPerp) / (body1.inverseMass + body2.inverseMass + (powf(glm::dot(perpRadius1, glm::vec3(MTV, 0.0f)), 2.0f) * body1.inverseMomentOfInertia + ( powf(glm::dot(perpRadius2, glm::vec3(MTV, 0.0f)), 2.0f) * body2.inverseMomentOfInertia)));

	//Step 5: Apply the impulse
	glm::vec3 impulse = glm::vec3(j * MTV, 0.0f);
	body1.netImpulse += impulse;

	//And determine the net angular impulse from this
	//Torque = radius x force
	body1.netAngularImpulse += glm::cross(radius1, impulse).z;

	impulse *= -1.0f;
	body2.netImpulse += impulse;

	//Determine angular impulse for body 2
	body2.netAngularImpulse += glm::cross(radius2, impulse).z;

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

	//Set the previous net impulse and net force
	body.previousNetForce = body.netForce;
	body.previousNetImpulse = body.netImpulse;

	//Zero the net impulse and net force!
	body.netForce = body.netImpulse = glm::vec3(0.0f);
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
	//body.angularVelocity += dt * body.angularAcceleration + body.netAngularImpulse / body.momentOfInertia;
	body.angularVelocity += dt * body.angularAcceleration + glm::vec3(0.0f, 0.0f, body.netAngularImpulse * body.inverseMomentOfInertia);
	body.netTorque = body.netAngularImpulse = 0.0f;
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
//	body1: The first of the colliding rigidbodies (Which the MTV points toward by our convention-- this will be the circle)
//	body2: The second of the intersecting circles (Which the MTV points away by our convention-- this will be the box)
//	MTV: The axis of minimal translation needed to separate the circles
//	mag: The magnitude of overlap along the minimum translation vector
void DecoupleObjects(struct RigidBody &body1, struct RigidBody &body2, const glm::vec2 &MTV, const float &mag)
{
	//The box will not move here, because the box is the ground.
	//Now, remember, the MTV always points toward object1, by the convention we established. So we want to move circle along the MTV!
	body1.position += glm::vec3(mag * MTV, 0.0f);
}

///
//Determines if a collision needs to be resolved
//
//Parameters:
//	body1: The rigidbody of the object that the minimum translation vector points toward
//	body2: The rigidbody of the object that the minimum translation vector points away from
//	MTV: the minimum translation vector
bool IsResolutionNeeded(struct RigidBody &body1, struct RigidBody &body2, glm::vec2 &MTV, glm::vec2 collisionPoint)
{
	//Find the relative velocity of the point of collision on object 2 from the point of collision on object 1
	glm::vec3 radius1 = glm::vec3(collisionPoint, 0.0f) - body1.position;
	glm::vec3 radius2 = glm::vec3(collisionPoint, 0.0f) - body2.position;

	glm::vec3 velTotal1 = body1.velocity + glm::cross(body1.angularVelocity, radius1);
	glm::vec3 velTotal2 = body2.velocity + glm::cross(body2.angularVelocity, radius2);

	glm::vec2 relativeVelocity = glm::vec2(velTotal2 - velTotal1);
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

// This function return the value between min and mx with the least distance value to x. This is called clamping.
float ClampOnRange(float x, float min, float max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;

	return x;
}

// This function clamps a vector on a rectangle. It basically finds the closest point on the rectangle to the point p.
glm::vec2 ClampOnRectangle(glm::vec2 p, AABB r)
{
	glm::vec2 closest_point;
	closest_point.x = ClampOnRange(p.x,
		r.center.x - (r.width / 2.0f),
		r.center.x + (r.width / 2.0f));

	closest_point.y = ClampOnRange(p.y,
		r.center.y - (r.height / 2.0f),
		r.center.y + (r.height / 2.0f));

	return closest_point;
}

//This function checks if the circle is colliding with the rectangle while
//determining the minimum translation vector and the magnitude of the overlap
//along that vector
bool CheckCollision(Circle c, AABB r, glm::vec2 &MTV, float &mag)
{
	//Gets the closest point on the box to the circle's center.
	glm::vec2 closest_point = ClampOnRectangle(c.center, r);

	//gets the distance between the circle's center and the point on box.
	float distance = glm::distance(closest_point, c.center);

	// if the point lies on/inside the circle; return true . else false.
	if (distance <= c.radius)
	{
		//Get the overlap & mtv
		MTV = closest_point - (c.center + c.radius * (glm::normalize(closest_point - c.center)));
		mag = glm::length(MTV);
		MTV = glm::normalize(MTV);
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
void Wrap(struct RigidBody &body)
{
	if(body.position.x  < -1.0f)
		body.position.x = 1.0f;
	if(body.position.x > 1.0f)
		body.position.x = -1.0f;
	if(body.position.y < -1.0f)
		body.position.y = 1.0f;
	if(body.position.y > 1.0f)
		body.position.y = -1.0f;
}

// This runs once every physics timestep.
void update(float dt)
{	
	//Apply acceleration due to gravity to the objects
	circleBody->netForce += glm::vec3(0.0f, -0.981f / circleBody->inverseMass, 0.0f);

	//Update the rigidbodies
	IntegrateLinear(dt, *circleBody);
	IntegrateAngular(dt, *circleBody);

	IntegrateLinear(dt, *groundBody);
	IntegrateAngular(dt, *groundBody);

	//Move the colliders
	circleCollider->center = glm::vec2(circleBody->position);
	groundCollider->center = glm::vec2(groundBody->position);

	//Check for collision
	if(CheckCollision(*circleCollider, *groundCollider, minimumTranslationVector, overlap))
	{
		//Decouple and find point of collision
		DecoupleObjects(*circleBody, *groundBody, minimumTranslationVector, overlap);
		pointOfCollision = DetermineCollisionPoint(*circleCollider, minimumTranslationVector, overlap);

		//Check if collision resolution is necessary
		//If two objects are already moving away from each other, but in contact, we let them resolves themselves.
		if(IsResolutionNeeded(*circleBody, *groundBody, minimumTranslationVector, pointOfCollision))
		{

			//Resolve collision
			ResolveCollision(*circleBody, *groundBody, minimumTranslationVector, pointOfCollision);

			//Apply friction
			ApplyLinearFriction(*circleBody, *groundBody, minimumTranslationVector, pointOfCollision);
		}
	}

	Wrap(*circleBody);
	Wrap(*groundBody);

	//Reposition colliders
	circleCollider->center = glm::vec2(circleBody->position);
	circleCollider->center = glm::vec2(groundBody->position);

	//Move appearance to new position
	circle->translation = glm::translate(glm::mat4(1.0f), circleBody->position);
	ground->translation = glm::translate(glm::mat4(1.0f), groundBody->position);

	circle->rotation = glm::mat4(circleBody->rotation);
	ground->rotation = glm::mat4(groundBody->rotation);

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
	ground->Draw();


}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Rolling (2D - Angular to Linear)", nullptr, nullptr);
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
	circle = new struct Mesh(numVertices, circleVerts, GL_LINE_LOOP);

	glm::vec3 groundScale(2.0f, 0.2f, 1.0f);
	struct Vertex* groundVerts;
	float arr[46] = 
	{
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f
	};

	groundVerts = (Vertex*)arr;

	//Circle2 creation
	ground = new struct Mesh(46, groundVerts, GL_TRIANGLES);


	//Scale the circles
	circle->scale = glm::scale(circle->scale, glm::vec3(circleScale));
	ground->scale = glm::scale(ground->scale, groundScale);


	//Generate the circle rigidbodies
	circleBody = new RigidBody(
		glm::vec3(-0.75f, 0.5f, 0.0f),	//Initial position
		glm::vec3(0.0f, 0.0f, 0.0f),		//Initial velocity
		glm::vec3(0.0f),					//Zero acceleration
		glm::mat3(1.0f),					//Initial orientation
		glm::vec3(0.0f, 0.0f, -1.0f),		//Initial angular velocity
		glm::vec3(0.0f),					//Initial Angular Acceleration
		1.0f,								//Mass
		1.0f,								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		0.8f,								//Dynamic coefficient of Friction
		1.0f								//Static coefficient of friction
		);
	circleBody->inverseMomentOfInertia = circleBody->inverseMass == 0.0f ?
		0.0f : 1.0f/CalculateMomentOfInertiaOfCircle(circleScale, 1.0f / circleBody->inverseMass);

	groundBody = new RigidBody(
		glm::vec3(0.0f, -0.8f, 0.0f), 		//Initial position
		glm::vec3(0.0f, 0.0f, 0.0f), 		//Initial velocity
		glm::vec3(0.0f), 					//Zero acceleration
		glm::mat3(1.0f),					//Initial orientation
		glm::vec3(0.0f),					//Initial angular velocity
		glm::vec3(0.0f),					//Initial Angular Acceleration
		0.0f, 								//Mass
		0.8f,								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		0.8f,								//Dynamic coefficient of friction
		1.0f								//Static coefficient of friction
		);
	groundBody->inverseMomentOfInertia = groundBody->inverseMass == 0.0f ? 
		0.0f : 1.0f/CalculateMomentOfInertiaOfRectangle(2.0f * groundScale.x, 2.0f * groundScale.y, 1.0f / groundBody->inverseMass);

	//Generate the circles colliders
	circleCollider = new Circle(glm::vec2(circleBody->position), circleScale);
	groundCollider = new AABB(glm::vec2(groundBody->position), 2.0f * groundScale.x, 2.0f * groundScale.y);


	//Position circles
	circle->translation = glm::translate(circle->translation, circleBody->position);
	ground->translation = glm::translate(ground->translation, groundBody->position);

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
	delete ground;
	delete circleCollider;
	delete groundCollider;
	delete circleBody;
	delete groundBody;

	// Frees up GLFW memory
	glfwTerminate();
}