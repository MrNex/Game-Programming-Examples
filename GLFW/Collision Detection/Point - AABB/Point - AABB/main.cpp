/*
Title: Point - AABB
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
This is a demonstration of collision detection between a point and an AABB.
The demo contains a point and a wireframe of a box. When the objects are not 
colliding the box will appear blue and the point will appear green. When the 
two objects collide the box will become pink and the point will become yellow. 

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. You can
swap which shape is selected with spacebar. 

This algorithm tests for collisions between a point and an AABB by determining if
the point lies between bounds of the AABB on the global X, Y, and Z axis. If
this is true for all 3 axis, we have a collision.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader variables
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

//Uniform variables
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

//An AABB Collider struct
struct AABB
{
	float width, height, depth;

	///
	//Default constructor creating an AABB of unit
	//Width, height, and depth (-1.0f to 1.0f on each axis)
	AABB::AABB()
	{
		width = height = depth = 2.0f;
	}

	///
	//Parameterized constructor creating an AABB of specified
	//width, height, and depth
	AABB::AABB(float w, float h, float d)
	{
		width = w;
		height = h;
		depth = d;
	}
};

struct Mesh* box;
struct Mesh* point;

struct Mesh* selectedShape;

struct AABB* aabbCollider;

float movementSpeed = 0.02f;
float rotationSpeed = 0.01f;

bool isMousePressed = false;
double prevMouseX = 0.0f;
double prevMouseY = 0.0f;

//Out of order Function declarations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Reads Shader Source
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

//Creates shader from source
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

	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	glPointSize(3.0f);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a point and an axis aligned bounding box
//
//Overview:
//	This algorithm tests for collisions between a point and an AABB by determining if
//	the point lies between bounds of the AABB on the global X, Y, and Z axis. If
//	this is true for all 3 axis, we have a collision.
//
//Parameters:
//	boxCollider: The AABB to test
//	boxModelMatrix: The box's model to world transformation matrix (assumes no rotation)
//	point: The point in worldspace
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const AABB &boxCollider, const glm::mat4 &boxModelMatrix, const glm::vec3 &point)
{
	//Get the minimum and maximum points on the AABB
	glm::vec3 min(-boxCollider.width / 2.0f, -boxCollider.height / 2.0f, -boxCollider.depth / 2.0f);
	glm::vec3 max(boxCollider.width / 2.0f, boxCollider.height / 2.0f, boxCollider.depth / 2.0f);

	//Translate the min and max into worldspace
	min = glm::vec3(boxModelMatrix * glm::vec4(min, 1.0f));
	max = glm::vec3(boxModelMatrix * glm::vec4(max, 1.0f));

	if (min.x <= point.x && point.x <= max.x)
		if (min.y <= point.y && point.y <= max.y)
			if (min.z <= point.z && point.z <= max.z)
				return true;

	return false;
}

// This runs once every physics timestep.
void update()
{

	if (TestCollision(*aabbCollider, box->GetModelMatrix(), glm::vec3(point->translation[3][0], point->translation[3][1], point->translation[3][2])))
	{
		//Turn red on
		hue[0][0] = 1.0f;
	}
	else
	{
		//Turn red off
		hue[0][0] = 0.0f;
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
	box->Draw();
	point->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == box ? point : box;

		//This set of controls are used to move the selectedShape.
		if (key == GLFW_KEY_W)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, movementSpeed, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_A)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-movementSpeed, 0.0f, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_S)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -movementSpeed, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_D)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(movementSpeed, 0.0f, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_LEFT_CONTROL)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, movementSpeed)) * selectedShape->translation;
		if (key == GLFW_KEY_LEFT_SHIFT)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -movementSpeed)) * selectedShape->translation;
	}

}

///
//Inturrupt triggered by mouse buttons
//
//Parameters:
//	window: The window which recieved the mouse click event
//	button: The mouse button which was pressed
//	action: GLFW_PRESS or GLFW_RELEASE
//	mods: The modifier keys which were pressed during the mouse click event
void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	//Set the boolean indicating whether or not the mouse is pressed
	isMousePressed = button == GLFW_MOUSE_BUTTON_LEFT ?
		(action == GLFW_PRESS ? true : false)
		: false;

	//Update the previous mouse position
	glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
}

#pragma endregion util_Functions


void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Point - AABB (3D) Collision Detection", nullptr, nullptr);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();



	//Generate the box mesh
	struct Vertex boxVerts[24];

	//Bottom face
	boxVerts[0] = { -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[1] = { 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[2] = { 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[3] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[4] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[5] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[6] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[7] = { -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };

	//Walls
	boxVerts[8] = { -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[9] = { -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[10] = { 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[11] = { 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[12] = { 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[13] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[14] = { -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[15] = { -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };

	//Top
	boxVerts[16] = { -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[17] = { 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[18] = { 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[19] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[20] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[21] = { -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[22] = { -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	boxVerts[23] = { -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };

	box = new struct Mesh(24, boxVerts, GL_LINES);

	//Translate the box
	box->translation = glm::translate(box->translation, glm::vec3(0.15f, 0.0f, 0.0f));

	//Scale the box
	box->scale = glm::scale(box->scale, glm::vec3(0.1f));

	//Generate point mesh
	struct Vertex pointVert = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };

	point = new struct Mesh(1, &pointVert, GL_POINTS);

	//Translate the point
	point->translation = glm::translate(point->translation, glm::vec3(-0.15f, 0.0f, 0.0f));

	//Set the selected shape
	selectedShape = box;

	//Generate AABB collider
	aabbCollider = new struct AABB(boxVerts[1].x - boxVerts[0].x, boxVerts[9].y - boxVerts[8].y, boxVerts[3].z - boxVerts[2].z);

	//Print controls
	std::cout << "Use WASD to move the selected shape in the XY plane.\nUse left CTRL & left shift to move the selected shape along Z axis.\n";
	std::cout << "Use spacebar to swap the selected shape.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);

	delete box;
	delete point;

	delete aabbCollider;

	// Frees up GLFW memory
	glfwTerminate();
}