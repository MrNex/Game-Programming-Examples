/*
Title: AABB - AABB (3D)
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
The demo contains two moving cubes, one pink, and one yellow.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving boxes to move any further.
when a moving box reaches one side of the screen, it will wrap around to the other side again.

The user can disable the continuous collision detection by holding spacebar.
This will cause the program to run static collision detection at the end of every physics timestep.
This will not prevent tunnelling. When two cubes collide the user can cause the simulation
to continue by toggling continuous and noncontinuous collision (Release spacebar if pressed, tap and hold spacebar,
then release).

This algorithm detects potentially missed collisions by performing a moving version of the
separating axis test. First we must determine the distances along each axis signifying
the distance to begin collision (dFirst) & the distance to separate from that collision (dLast). Then
we can easily determine the time at which these distances will be reached by dividing them by the magnitude of the
velocity along the axis (tFirst / tLast). If we keep the largest tFirst and the smallest tLast from all axes,
we will determine the time interval which the cubes will be intersecting! If tLast < tFirst, the boxes will not overlap.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader program
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
//Shader uniforms
GLuint uniMVP;
GLuint uniHue;
glm::mat4 hue;
glm::mat4 VP;
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

//Struct for AABB collider
struct AABB
{
	glm::vec3 center;
	glm::vec3 dimensions;

	//Default constructor, creates AABB of sidelength 2 centered at origin
	AABB::AABB()
	{
		center = glm::vec3(0.0f);
		dimensions = glm::vec3(2.0f);
	}

	//Parameterized constructor, creates AABB with specified center of specified dimension
	AABB::AABB(glm::vec3 c, glm::vec3 dim)
	{
		center = c;
		dimensions = dim;
	}
};

struct Mesh* box1;
struct Mesh* box2;

struct RigidBody* box1Body;
struct RigidBody* box2Body;

struct AABB* AABB1;
struct AABB* AABB2;


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.5; // This is the number of milliseconds we intend for the physics to update.


#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Reads a text file containing shader code
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

//Creates shaders from source code
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
	//Initialize glew
	glewInit();

	//create shader
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);	
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	// End of shader and program creation

	//Set Calculate view projection matrix
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	VP = proj * view;

	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set rendering options
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);



}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Checks for collision between two axis aligned bounding box by seeing 
//if they overlap on each axis.
//
//Parameters:
//	aabb1: The first axis aligned bounding box to test
//	aabb2: The second axis aligned bounding box to test
//
//Returns:
//	true if colliding, else false.
bool CheckCollision(const AABB &aabb1, const AABB &aabb2)
{
	//Find the max and min points for each box
	glm::vec3 min1(aabb1.center - (aabb1.dimensions*0.5f));
	glm::vec3 max1(aabb1.center + (aabb1.dimensions*0.5f));
	glm::vec3 min2(aabb2.center - (aabb2.dimensions*0.5f));
	glm::vec3 max2(aabb2.center + (aabb2.dimensions*0.5f));

	//If there is overlap on each axis, we have a collision
	if (max1.x > min2.x && min1.x < max2.x)
		if (max1.y > min2.y && min1.y < max2.y)
			if (max1.z > min2.z && min1.z < max2.z)
				return true;

	return false;
}

///
//Performs a dynamic collision check between a moving AABB and a static AABB.
//
//Overview:
//	This algorithm detects potentially missed collisions by performing a moving version of the 
//	separating axis test. First we must determine the distances along each axis signifying
//	the distance to begin collision (dFirst) & the distance to separate from that collision (dLast). Then
//	we can easily determine the time at which these distances will be reached by dividing them by the magnitude of the
//	velocity along the axis (tFirst / tLast). If we keep the largest tFirst and the smallest tLast from all axes,
//	we will determine the time interval which the boxes will be intersecting! If tLast < tFirst, the boxes will not overlap.
//	Alternatively, if tFirst > 1.0f,  the boxes will not overlap!
//
//Parameters:
//	aabb1: The moving aabb
//	aabb2: The static aabb
//	mvmt: The relative movement vector of box 1 from an observer on box 2
//
//Returns:
//	a t value between 0 and 1 indicating the "relative time" since the start of this frame that the collision occurred.
//	a t value of 0.0f would indicate the very start of this frame, a t value of 1.0f would indicate the very end of this frame.
float CheckDynamicCollision(const AABB &aabb1, const AABB &aabb2, const glm::vec3 &mvmt)
{
	//If we start out colliding, we do not need to perform the test.
	if (CheckCollision(aabb1, aabb2))
	{
		return 0.0f;
	}

	float tFirst = 0.0f;
	float tLast = 1.0f;
	float tCurrent = 0.0f;

	//Get the minimum and maximum dimensions in world space for each box
	glm::vec2 min1(aabb1.center - (aabb1.dimensions*0.5f));
	glm::vec2 max1(aabb1.center + (aabb1.dimensions*0.5f));
	glm::vec2 min2(aabb2.center - (aabb2.dimensions*0.5f));
	glm::vec2 max2(aabb2.center + (aabb2.dimensions*0.5f));

	//For every axis (X, and Y axes in this case)
	for (int i = 0; i < 3; i++)
	{
		//Check the direction of the projection of the movement vector on this axis
		if (mvmt[i] < 0.0f)
		{
			//In this case object 1 is moving in the negative direction along axis from an observer on object 2.
			//So if object 1 is more negative in direction than object 2, they will not collide on this axis.
			if (max1[i] < min2[i]) return -1.0f;
			//Is the "low part" object 1 higher than the "high part" object 2 along this axis?
			if (min1[i] > max2[i])
			{
				//If so, the shapes are not yet colliding on this axis, so determine when they first will collide on this axis
				tCurrent = (max2[i] - min1[i]) / mvmt[i];		//We solve for a negative distance here, because we are dividing by a negative velocity to get a positive time
				//This strange ordering prevents us from needing to make a call to fabs (absolute value function)
				//If it is larger than the current tFirst, change tFirst
				if (tCurrent > tFirst) tFirst = tCurrent;
			}
			//Is the "High Part" of object 1 higher than the low part of object 2 along this axis?
			if (max1[i] > min2[i])
			{
				//If so, the shapes have not yet separated on this axis, so determine when they will finish colliding
				tCurrent = (min2[i] - max1[i]) / mvmt[i];		//Note the wierd ordering again, for the same reason as above
				//If it is smaller than current tLast, update tLast
				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
		else if (mvmt[i] > 0.0f)
		{
			//If object 1 is more positive along the axis than object 2, they will not collide
			if (min1[i] > max2[i]) return -1.0f;
			//Is the "High part" of object 1 lower than the "low part" of object 2 along this axis?
			if (max1[i] < min2[i])
			{
				//If so, the shapes are not yet colliding on this axis so determine when they will first collide on this axis
				tCurrent = (min2[i] - max1[i]) / mvmt[i];
				//If it is larger than the current tFirst, update tFirst
				if (tCurrent > tFirst) tFirst = tCurrent;
			}
			//Is the "Low part" of object 1 lower than the "high part" of object 2 along this axis?
			if (min1[i] < max2[i])
			{
				//If so, the shapes have not yet separated on this axis, so determine when they will finish collidiing
				tCurrent = (max2[i] - min1[i]) / mvmt[i];
				//If it is smaller than the current tLast, update tLast
				if (tCurrent < tLast) tLast = tCurrent;
			}
		}
	}

	//If there was no overlap
	if (tLast < tFirst) return -1.0f;

	return tFirst;

}

//Updates the scene with continuous collision disabled
void noncontinuousCollisionUpdate(float dt)
{
	//Save previous position
	glm::vec3 prev1Pos = box1Body->position;
	glm::vec3 prev2Pos = box2Body->position;


	//Integrate and get the new position of box1Body
	box1Body->position += box1Body->velocity * dt;
	//And box2Body
	box2Body->position += box2Body->velocity * dt;


	//Move the AABBs
	AABB1->center = box1Body->position;
	AABB2->center = box2Body->position;

	//Move appearance to new position
	box1->translation = glm::translate(glm::mat4(1.0f), box1Body->position);
	box2->translation = glm::translate(glm::mat4(1.0f), box2Body->position);

	//If the position goes off of the right edge of the screen, loop it back to the left
	if (box1Body->position.x > 1.0f)
	{

		box1Body->position.x = -1.0f;
		AABB1->center = box1Body->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		box1->translation = glm::translate(glm::mat4(1.0f), box1Body->position);


	}
	if (box2Body->position.x < -1.0f)
	{
		box2Body->position.x = 1.0f;
		AABB2->center = box2Body->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		box2->translation = glm::translate(glm::mat4(1.0f), box2Body->position);
	}

	//Check for collision
	if (CheckCollision(*AABB1, *AABB2))
	{
		//If there is a collision, revert to last known positions
		//box1 revert
		box1Body->position = prev1Pos;
		AABB1->center = box1Body->position;

		//box2 revert
		box2Body->position = prev2Pos;
		AABB2->center = box2Body->position;
	}
}

//Updates the scene with continuous collision enabled
void continuousCollisionUpdate(float dt)
{

	//Determine relative velocity of box 1 from stationary box 2
	glm::vec3 relV = box1Body->velocity - box2Body->velocity;
	float t = CheckDynamicCollision(*AABB1, *AABB2, relV * dt);

	if (t >= 0.0f)
	{
		//Reposition at point of intersection
		box1Body->position += box1Body->velocity * dt * t;
		box2Body->position += box2Body->velocity * dt * t;

	}
	else
	{
		//Move the box rigid bodies
		box1Body->position += box1Body->velocity * dt;
		box2Body->position += box2Body->velocity * dt;
	}



	//Move colliders
	AABB1->center = box1Body->position;
	AABB2->center = box2Body->position;

	//If the position goes off of the right edge of the screen, loop it back to the left
	if (box1Body->position.x > 1.0f)
	{

		box1Body->position.x = -1.0f;
		AABB1->center = box1Body->position;
	}
	if (box2Body->position.x < -1.0f)
	{
		box2Body->position.x = 1.0f;
		AABB2->center = box2Body->position;
	}

	//Once we have solved for the bodys position, we should translate the mesh to match it
	box1->translation = glm::translate(glm::mat4(1.0f), box1Body->position);
	box2->translation = glm::translate(glm::mat4(1.0f), box2Body->position);
}

// This runs once every physics timestep.
void update(float dt)
{
	//If the user presses spacebar, use non-continuous collision detection
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		noncontinuousCollisionUpdate(dt);
	}
	//Else, use continuous collision detection
	else
	{
		continuousCollisionUpdate(dt);
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
	box1->Draw();
	box2->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "AABB - AABB (3D Dynamic Collision Detection)", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//disable vsync
	glfwSwapInterval(0);

	// Initializes openGL options
	init();


	//Generate the box mesh
	float boxScale = 0.1f;
	struct Vertex boxVerts[24] =
	{
		{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f }, 
		{ -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f },
	};

	//box1 creation
	box1 = new struct Mesh(24, boxVerts, GL_LINES);
	//Alter mesh for box2
	for (int i = 0; i < 24; i++)
	{
		boxVerts[i].g = 0.0f;
		boxVerts[i].b = 1.0f;
	}
	//box2 creation
	box2 = new struct Mesh(24, boxVerts, GL_LINES);


	//Scale the boxes
	box1->scale = glm::scale(box1->scale, glm::vec3(boxScale));
	box2->scale = glm::scale(box2->scale, glm::vec3(boxScale));


	//Generate the box's rigidbody
	box1Body = new struct RigidBody(
		glm::vec3(-1.0f, 0.0f, 0.0f),		//Start on left side of screen
		glm::vec3(1.0f, 0.0f, 0.0f),		//constant right velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	box2Body = new struct RigidBody(
		glm::vec3(0.75f, 0.0f, 0.0f),		//Start on right side
		glm::vec3(-0.5f, 0.0f, 0.0f),		//Constant left velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	//Position boxes
	box1->translation = glm::translate(box1->translation, box1Body->position);
	box2->translation = glm::translate(box2->translation, box2Body->position);

	//Generate the boxes' colliders
	AABB1 = new AABB(box1Body->position, boxScale * glm::vec3(2.0f));
	AABB2 = new AABB(box2Body->position, boxScale * glm::vec3(2.0f));

	//Print controls
	std::cout << "Controls:\nPress and hold spacebar to disable continuous collision detection.\nWhen two boxes collide, continue the simulation by toggling continuous collision detection on and off.\n";
	std::cout << "(Release spacebar if pressed, tap and hold spacebar, then release.)\n";

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

	delete box1;
	delete box2;
	delete box1Body;
	delete box2Body;
	delete AABB1;
	delete AABB2;

	// Frees up GLFW memory
	glfwTerminate();
}