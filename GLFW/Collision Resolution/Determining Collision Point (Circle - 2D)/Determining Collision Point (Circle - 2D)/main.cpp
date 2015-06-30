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
This is a demonstration of detecting the collision point between two colliding circles in 2D.
There are Pink and Yellow wireframes of circles. When the circles collide the collision point
is calculated and marked with a white box on the screen. 

After the two circles have been decoupled, the collision point of two circles can be found trivially.
The point is found by travelling a distance of a circles radius along the minimum translation vector.

The user can move a circle using WASD.
The user can also press spacebar to toggle the circle he/she is moving.

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

struct Mesh* circle1;
struct Mesh* circle2;

struct Circle* circle1Collider;
struct Circle* circle2Collider;

struct Circle* selectedCollider;

glm::vec2 minimumTranslationVector;
float overlap;

glm::vec2 pointOfCollision;

bool collision = false;

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Set key callback
	glfwSetKeyCallback(window, OnKeyPress);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Function which handles key-presses
//
//PArameters:
//	window: The window in focus during the key press
//	int: The key code
//	scanCode: The OS Scan code
//	action: GLFW_PRESS or GLFW_RELEASE
//	mods: Any modifier keys which were pressed.
void OnKeyPress(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
	static const float movementSpeed = 0.1f;

	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_SPACE)
		{
			selectedCollider = selectedCollider == circle1Collider ? circle2Collider : circle1Collider;
		}

		if(key == GLFW_KEY_A)
		{
			selectedCollider->center.x -= movementSpeed;
			collision = false;
		}
		if(key == GLFW_KEY_D)
		{
			selectedCollider->center.x += movementSpeed;
			collision = false;
		}
		if(key == GLFW_KEY_W)
		{
			selectedCollider->center.y += movementSpeed;
			collision = false;
		}
		if(key == GLFW_KEY_S)
		{
			selectedCollider->center.y -= movementSpeed;
			collision = false;
		}
	}
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
void DecoupleObjects(struct Circle &c1, struct Circle &c2, const glm::vec2 &MTV, const float &mag)
{
	//The first step in decoupling a pair of objects is to figure how much you must move each one.
	//Normally, you can do this by taking the sum of the magnitudes of their velocities along the MTV
	//And performing a ratio of each individual velocities magnitude along the MTV to that sum.
	//
	//For example, if we wanted to figure out how much to move circle c1:
	//	individual = fabs(glm::Dot(c1.velocity, MTV))
	//	sum = individual + fabs(glm::Dot(c2.velocity, MTV));
	//	ratio = individual / sum
	//
	//From here, we can figure out the magnitude of of how much to move poly 1 along the MTV by taking the product of the ratio with the magnitude of the overlap
	//	mag1 = ratio * mag
	//
	//However, in this simulation we do not have velocities-- and it is clear which object is doing all of the movement. However, Instead of translating the
	//moving polygon, I will translate the non-moving polygon to allow for a "Pushing" effect that might be sought after in simpler games!
	float mag1, mag2;
	//The next line of code is just setting the mag corresponding to polygon that is NOT the selected polygon to 1. The other polygon is set to 0.
	//So for example, if polygon 1 is the selected polygon, mag1 = 0, and mag2 = 1, and visa versa
	mag1 = selectedCollider == &c1 ? ((mag2 = 1.0f), 0.0f) : ((mag2 = 0.0f), 1.0f);	//Don't code like this if you aren't good at reading this kind of thing. Readability is important.
																					//Also, don't code like this if anybody else will need to maintain your code. They will hate you.
																					//I just broke that rule^^.. Sorry whoever needs to fix this one day...

	//Now, remember, the MTV always points toward object1, by the convention we established. So we want to move poly1 along the MTV, and poly2 opposite the MTV!
	c1.center += glm::vec3(mag1 * mag * MTV, 0.0f);
	c2.center -= glm::vec3(mag2 * mag * MTV, 0.0f);
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


// This runs once every physics timestep.
void update(float dt)
{	

	//Check for collision
	if(CheckCollision(*circle1Collider, *circle2Collider, minimumTranslationVector, overlap))
	{
		collision = true;
		DecoupleObjects(*circle1Collider, *circle2Collider, minimumTranslationVector, overlap);
		pointOfCollision = DetermineCollisionPoint(*circle1Collider, minimumTranslationVector, overlap);
	}


	//Move appearance to new position
	circle1->translation = glm::translate(glm::mat4(1.0f), circle1Collider->center);
	circle2->translation = glm::translate(glm::mat4(1.0f), circle2Collider->center);

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

	//DRaw the collision point
	if(collision)
	{
		glUseProgram(0);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPointSize(10.0f);
		glBegin(GL_POINTS);
		glVertex3f(pointOfCollision.x, pointOfCollision.y, 0.0f);
		glEnd();
	}
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Detecting the Point of Collision (Circle - 2D)", nullptr, nullptr);
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
	circle2->scale = glm::scale(circle2->scale, glm::vec3(circleScale));

	//Generate the circles colliders
	circle1Collider = new Circle(glm::vec3(-0.75f, 0.0f, 0.2f), circleScale);
	circle2Collider = new Circle(glm::vec3(0.75f, 0.0f, 0.2f), circleScale);

	selectedCollider = circle1Collider;


	//Position circles
	circle1->translation = glm::translate(circle1->translation, circle1Collider->center);
	circle2->translation = glm::translate(circle2->translation, circle2Collider->center);

	//Print controls
	std::cout << "Controls:\nUse WASD to move the selected circle around the XY plane.\n";
	std::cout << "Press spacebar to swap the selected circle.\n";

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

	// Frees up GLFW memory
	glfwTerminate();
}