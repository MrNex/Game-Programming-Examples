/*
Title: Circle - Circle (2D)
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
The demo contains two moving circles, one pink, and one yellow.
The physics timestep has been raised to only run once per half second.
This causes the movement jump over very large intervals per timestep.
When the program detects a collision, it will not allow the moving circle to move any further.
If the moving circle reaches the right side of the screen, it will wrap around to the left side again.

The user can disable the continuous collision detection by holding spacebar. 
This will cause the program to run static collision detection at the end of every physics timestep.
This will not prevent tunnelling. When two circles collide the user can cause the simulation
to continue by toggling continuous and noncontinuous collision (Release spacebar if pressed, tap and hold spacebar, 
then release).

The continuous collision detection algorithm used employs a technique known as interval halving.
First it is necessary that we get the relative movement, such that one circle is moving at X speed
relative to the other being still. Following this, we perform the interval halving by starting the algorithm over the entire movement interval.
Over the given movement interval, this algorithm will surround the extent of the movement of the moving circle
with a bounding circle. If this bounding circle still collides with the static circle, the interval
is split into two halves and the function calls itself recursively on the smaller intervals.
Once the interval being tested gets to a range which is smaller or equal to a set interval epsilon based on desired accuracy
the function will exit, returning the end time (0.0f <= t <= 1.0f) of the smallest interval which first occurred.
If at any point the function does not detect a collision between the static circle and the bounding circle
in full interval before the exit condition is met, the function registers no collision.

References:
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson

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

struct Mesh* circle1;
struct Mesh* circle2;

struct RigidBody* circle1Body;
struct RigidBody* circle2Body;

struct Circle* circle1Collider;
struct Circle* circle2Collider;


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.5; // This is the number of milliseconds we intend for the physics to update.


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
	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	VP = proj * view;

	//Create uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Checks for collision between two circles by seeing if the distance between them is less
//than the sum of the radii.
//
//Parameters:
//	c1: The first circle to test
//	c2: The second circle to test
//
//Returns:
//	true if colliding, else false.
bool CheckCollision(const Circle &c1, const Circle &c2)
{
	float dist = glm::length(c1.center - c2.center);
	return (c1.radius + c2.radius) >= dist;
}

///
//Performs a dynamic collision check between a moving circle and a static circle.
//
//Overview:
//	This algorithm detects potentially missed collisions by using the interval halving method. Over the
//	given movement interval, this algorithm will surround the extent of the movement of the moving circle
//	with a bounding circle. If this bounding circle still collides with the static circle, the interval
//	is split into two halves and the function calls itself recursively on the smaller intervals.
//	Once the interval being tested gets to a range which is smaller or equal to the radius of the circle
//	the function will exit, returning the end time (0.0f <= t <= 1.0f) of the smallest interval which first occurred.
//	If at any point the function does not detect a collision between the static circle and the bounding circle
//	before the exit condition is meant, the function registers no collision.
//
//Parameters:
//	c1: The moving circle
//	c2: The static circle
//	mvmt: The relative movement vector of circle 1 from an observer on circle 2
//	tStart: The start of the interval, 0.0f <= tStart < 1.0f
//	tEnd: The end of the interval, 0.0f < tEnd <= 1.0f
//
//Returns:
//	a t value between 0 and 1 indicating the "relative time" since the start of this frame that the collision occurred.
//	a t value of 0.0f would indicate the very start of this frame, a t value of 1.0f would indicate the very end of this frame.
//	This function will return a negative number if no collision was registered.
float CheckDynamicCollision(Circle c1, const Circle &c2, const glm::vec3 &mvmt, float tStart, float tEnd)
{
	//Get the midpoint time of the interval
	float tMid = 0.5f*(tEnd - tStart) + tStart;
	//Determine the position at time tMid
	glm::vec3 pos = tMid * mvmt + c1.center;

	float mvmtLength = glm::length((tEnd - tStart) * mvmt);

	//Determine the radius of the bounding circle needed to encapsulate this movement
	float rad = mvmtLength /*+ c1.radius*/;

	//Create the bounding circle
	Circle boundingCircle(pos, rad);

	//check the bounding circle colliding with c2
	if (CheckCollision(boundingCircle, c2))
	{
		//Set the result as the furthest point along the current interval.
		//This ensures that when the function returns t will represent the position along the movement vector
		//Which will cause you to be entering collision.
		//
		//If you had wanted to be just before the collision, you could set it to tStart.
		float result = tMid;

		//Note: This if statement determines the accuracy of the algorithm!
		//The smaller you make the right side, the more accurate the algorithm will become!
		//However this is at a big cost of speed.
		if (mvmtLength > c1.radius)
		{

			//Divide the interval in 2 and check the first half
			result = CheckDynamicCollision(c1, c2, mvmt, tStart, tMid);
			//If the result is false, check the second half
			if (result < 0.0f)
			{
				result = CheckDynamicCollision(c1, c2, mvmt, tMid, tEnd);
			}
		}
		//Return the result
		return result;
	}
	else
	{
		return -1.0f;
	}

}

//Updates the scene with continuous collision disabled
void noncontinuousCollisionUpdate(float dt)
{
	//Save previous position
	glm::vec3 prev1Pos = circle1Body->position;
	glm::vec3 prev2Pos = circle2Body->position;


	//Integrate and get the new position of circle1Body
	circle1Body->position += circle1Body->velocity * dt;
	//And circle2Body
	circle2Body->position += circle2Body->velocity * dt;


	//Move the circle1Collider
	circle1Collider->center = circle1Body->position;
	//And circle2Collider
	circle2Collider->center = circle2Body->position;

	//Move appearance to new position
	circle1->translation = glm::translate(glm::mat4(1.0f), circle1Body->position);
	circle2->translation = glm::translate(glm::mat4(1.0f), circle2Body->position);

	//Check for collision
	if (CheckCollision(*circle1Collider, *circle2Collider))
	{
		//If there is a collision, revert to last known positions
		circle1Body->position = prev1Pos;
		circle1Collider->center = circle1Body->position;

		//circle2
		circle2Body->position = prev2Pos;
		circle2Collider->center = circle2Body->position;
	}
}

//Updates the scene with continuous collision enabled
void continuousCollisionUpdate(float dt)
{
	//Check for collision a normal collision 
	//If they are already colliding, there is no need for continuous detection.
	if (!CheckCollision(*circle1Collider, *circle2Collider))
	{
		//Determine relative velocity of circle 1 from circle 2
		glm::vec3 relV = circle1Body->velocity - circle2Body->velocity;
		float t = CheckDynamicCollision(*circle1Collider, *circle2Collider, relV * dt, 0.0f, 1.0f);

		if (t >= 0.0f)
		{
			//Reposition at point of intersection
			circle1Body->position += circle1Body->velocity * dt * t;
			circle2Body->position += circle2Body->velocity * dt * t;

		}
		else
		{
			//Move the circlebody
			circle1Body->position += circle1Body->velocity * dt;
			circle2Body->position += circle2Body->velocity * dt;
		}
	}


	//Move collider
	circle1Collider->center = circle1Body->position;
	circle2Collider->center = circle2Body->position;

	//Once we have solved for the bodys position, we should translate the mesh to match it
	circle1->translation = glm::translate(glm::mat4(1.0f), circle1Body->position);
	circle2->translation = glm::translate(glm::mat4(1.0f), circle2Body->position);
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

	//If the position goes off of the right edge of the screen, loop it back to the left
	if (circle1Body->position.x > 1.0f)
	{

		circle1Body->position.x = -1.0f;
		circle1Collider->center = circle1Body->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		circle1->translation = glm::translate(glm::mat4(1.0f), circle1Body->position);
		

	}
	if (circle2Body->position.x < -1.0f)
	{
		circle2Body->position.x = 1.0f;
		circle2Collider->center = circle2Body->position;
		//Once we have solved for the bodys position, we should translate the mesh to match it
		circle2->translation = glm::translate(glm::mat4(1.0f), circle2Body->position);
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
	window = glfwCreateWindow(800, 800, "Circle - Circle (2D Dynamic Collision Detection)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate the circle mesh
	float circleScale = 0.1f;
	int numVertices = 72;
	struct Vertex circleVerts[72];
	float stepSize = 2.0f * 3.14159 / (numVertices/3.0f);
	int vertexNumber = 0;
	for (int i = 0; i < numVertices; i++)
	{
		circleVerts[i] = { cosf(vertexNumber * stepSize), sinf(vertexNumber * stepSize), 0.0f, 1.0f, 1.0f, 0.0f, 1.0 };
		++i;
		++vertexNumber;
		circleVerts[i] = { cosf(vertexNumber * stepSize), sinf(vertexNumber * stepSize), 0.0f, 1.0f, 1.0f, 0.0f, 1.0 };
		++i;
		circleVerts[i] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
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
	circle2->scale = glm::scale(circle2->scale, glm::vec3(circleScale));


	//Generate the circle's rigidbody
	circle1Body = new struct RigidBody(
		glm::vec3(-1.0f, 0.0f, 0.0f),		//Start on left side of screen
		glm::vec3(1.0f, 0.0f, 0.0f),		//constant right velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	circle2Body = new struct RigidBody(
		glm::vec3(0.75f, 0.0f, 0.0f),		//Start on right side
		glm::vec3(-0.5f, 0.0f, 0.0f),		//Constant left velocity
		glm::vec3(0.0f, 0.0f, 0.0f)			//Zero acceleration
		);

	//Position circles
	circle1->translation = glm::translate(circle1->translation, circle1Body->position);
	circle2->translation = glm::translate(circle2->translation, circle2Body->position);

	//Generate the circles colliders
	circle1Collider = new Circle(circle1Body->position, circleScale);
	circle2Collider = new Circle(circle2Body->position, circleScale);

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

	delete circle1;
	delete circle2;
	delete circle1Body;
	delete circle2Body;
	delete circle1Collider;
	delete circle2Collider;

	// Frees up GLFW memory
	glfwTerminate();
}