/*
Title: Line - AABB Collision Detection
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
This is a collision detection algorithm between a line and an axis-aligned bounding box in 2D.
By determining which side of the line the corners of the AABB fall on, we can tell if they are colliding.
If two corners fall on two different sides of the line, then the line and the box must be colliding.
We can determine which side of the line corners fall on by observing the sign of the dot product of each corner 
with the normal of the line. The objects will appear green when not colliding, and red once they are colliding.

Use WASD to move the line, and Q and E to rotate the line.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
2D Game Collision Detection by Thomas Schwarzl
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
	glm::mat4 modelMatrix;
	int numVertices;
	struct Vertex* vertices;
	GLenum primitive;

	Mesh::Mesh(int numVert, struct Vertex* vert, GLenum primType)
	{
		this->modelMatrix = glm::mat4(1.0f);

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

	void Mesh::Draw(void)
	{
		//Bind the VAO being drawn
		glBindVertexArray(this->VAO);

		// Set the uniform matrix in our shader to our MVP matrix for this mesh.
		glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(this->modelMatrix));
		//Draw the mesh
		glDrawArrays(this->primitive, 0, this->numVertices);

	}

};

struct AABB
{
	float width;
	float height;

	AABB::AABB()
	{
		this->width = 1.0f;
		this->height = 1.0f;
	}

	///
	//Generates an axis aligned bounding box to a specified width and height
	//
	//Parameters:
	//	w:	The width of the AABB (Before scaling)
	//	h: The height of the AABB (before scaling)
	AABB::AABB(const float w, const float h)
	{
		this->width = w;
		this->height = h;
	}
};

//A collider which consists of two points representing the line which travels through both of them.
struct LineCollider
{
	glm::vec2 point1;
	glm::vec2 point2;

	LineCollider::LineCollider()
	{
		point1 = glm::vec2(-1.0f, 0.0f);
		point2 = glm::vec2(1.0f, 0.0f);
	}

	///
	//Generates a line collider given two points which exist on the line
	//
	//Parameters:
	//	p1: A point on the line
	//	p2: Another point on the line
	LineCollider::LineCollider(glm::vec2 p1, glm::vec2 p2)
	{
		point1 = p1;
		point2 = p2;
	}
};

struct Mesh* square;
struct Mesh* line;

struct AABB squareCollider;
struct LineCollider lineCollider;

float movementSpeed = 0.02f;
float rotationSpeed = 0.01f;

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Reads shader source
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

	//Generate shader program
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests if an axis aligned bounding box and a line collider are intersecting
//
//Overview:
//	This test uses the dot product to check the direction of the corners of the AABB in relative to the line.
//	If all four corners are in the same direction, the line must not be colliding with the box.
//
//Parameters:
//	aabb: The axis aligned bounding box to test with
//	AABBTransform: The model to world transformation matrix of the model the AABB is surrounding
//	lineCol: The line collider to test with
//	lineTransform: The model to world transformation matrix of the line
//
//Returns:
//	true if a collision is detected, else false.
bool TestCollision(const AABB &aabb, const glm::mat4 &AABBTransform, const LineCollider &lineCol, const glm::mat4 &lineTransform)
{
	//Step 1: Get the corners of the AABB in world space.
	glm::vec4 corner1 = AABBTransform * glm::vec4(aabb.width / 2.0f, aabb.height / 2.0f, 0.0f, 1.0f);	//Top right
	glm::vec4 corner2 = AABBTransform * glm::vec4(-aabb.width / 2.0f, aabb.height / 2.0f, 0.0f, 1.0f);	//Top left
	glm::vec4 corner3 = AABBTransform * glm::vec4(-aabb.width / 2.0f, -aabb.height / 2.0f, 0.0f, 1.0f);	//Bottom left
	glm::vec4 corner4 = AABBTransform * glm::vec4(aabb.width / 2.0f, -aabb.height / 2.0f, 0.0f, 1.0f);	//Bottom right

	//Step 2: Find the normal of the line rotated to the lines current rotation
	glm::vec2 direction = glm::normalize(lineCol.point2 - lineCol.point1);
	glm::vec3 orientedDirection = glm::mat3(lineTransform) * glm::vec3(direction, 0.0f);
	glm::vec4 normal(-orientedDirection.y, orientedDirection.x, 0.0f, 0.0f);


	//Step 3: It is much easier to determine the side of the line which the corners are on if the line passes through the origin
	//So we can shift the coordinate system to be centered on the line
	glm::vec3 linePosition(lineTransform[3][0], lineTransform[3][1], lineTransform[3][2]);
	glm::mat4 shift = glm::translate(glm::mat4(1.0f), -linePosition);

	//(ProTip: You can do this in one step when you are finding the corners to begin with)
	corner1 = shift * corner1;
	corner2 = shift * corner2;
	corner3 = shift * corner3;
	corner4 = shift * corner4;

	//Now we can use the dot product of the normal with the corners, if the dot product has the same sign for all corners then there is no intersection!
	if (glm::dot(corner1, normal) > 0.0f)
	{
		//If any signs do not match the first
		if (glm::dot(corner2, normal) < 0.0f ||
			glm::dot(corner3, normal) < 0.0f ||
			glm::dot(corner4, normal) < 0.0f)
		{
			return true;
		}
	}
	else
	{
		//If any signs do not match the first
		if (glm::dot(corner2, normal) > 0.0f ||
			glm::dot(corner3, normal) > 0.0f ||
			glm::dot(corner4, normal) > 0.0f)
		{
			return true;
		}
	}

	return false;
}

// This runs once every physics timestep.
void update()
{
	if (TestCollision(squareCollider, square->modelMatrix, lineCollider, line->modelMatrix))
	{
		//Change hue to red
		hue[0][0] = 1.0f;
		hue[1][1] = 0.0f;
	}
	else
	{
		//Change hue to green
		hue[0][0] = 0.0f;
		hue[1][1] = 1.0f;
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Draw the Gameobjects
	square->Draw();
	line->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This set of controls are used to move the line.
		if (key == GLFW_KEY_W)
			line->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, movementSpeed, 0.0f)) * line->modelMatrix;
		if (key == GLFW_KEY_A)
			line->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-movementSpeed, 0.0f, 0.0f)) * line->modelMatrix;
		if (key == GLFW_KEY_S)
			line->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -movementSpeed, 0.0f)) * line->modelMatrix;
		if (key == GLFW_KEY_D)
			line->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(movementSpeed, 0.0f, 0.0f)) * line->modelMatrix;

		//This set of controls is used to rotate the line
		if (key == GLFW_KEY_Q)
			line->modelMatrix = glm::rotate(line->modelMatrix, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
		if (key == GLFW_KEY_E)
			line->modelMatrix = glm::rotate(line->modelMatrix, -rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
	}

}

#pragma endregion Helper_functions


void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Line - AABB (2D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	//Set global hue to color objects green
	hue[0][0] = 0.0f;
	hue[2][2] = 0.0f;

	//Generate the square mesh
	struct Vertex squareVerts[6];
	squareVerts[0] = { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };		//Top right
	squareVerts[1] = { -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };		//Top Left
	squareVerts[2] = { -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };	//Bottom Left
	squareVerts[3] = { -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };	//Bottom Left
	squareVerts[4] = { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };		//Bottom Right
	squareVerts[5] = { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };		//Top Right

	square = new struct Mesh(6, squareVerts, GL_TRIANGLES);

	//Scale the square
	square->modelMatrix = square->modelMatrix * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

	//Generate the AABB for the square
	float width = glm::length(glm::vec2(squareVerts[0].x, squareVerts[0].y) - glm::vec2(squareVerts[1].x, squareVerts[1].y));
	float height = glm::length(glm::vec2(squareVerts[2].x, squareVerts[2].y) - glm::vec2(squareVerts[1].x, squareVerts[1].y));

	squareCollider = AABB(width, height);


	//Generate the line mesh
	struct Vertex lineVerts[2];
	lineVerts[0] = {-15.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	lineVerts[1] = { 15.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	line = new struct Mesh(2, lineVerts, GL_LINES);

	//Generate the line collider using the two vertices of the line.
	lineCollider = LineCollider(glm::vec2(lineVerts[0].x, lineVerts[0].y), glm::vec2(lineVerts[1].x, lineVerts[1].y));

	//Print controls
	std::cout << "Controls:\nUse WASD to move the line.\nUse Q and E to rotate the line.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

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

	delete square;
	delete line;

	// Frees up GLFW memory
	glfwTerminate();
}