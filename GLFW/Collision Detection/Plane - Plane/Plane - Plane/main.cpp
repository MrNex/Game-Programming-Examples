/*
Title: Plane - Plane
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
This is a demonstration of collision detection between two planes.
When the objects are not colliding one plane will appear blue and the
other will appear green. When the two objects collide one will 
become pink and the other will become yellow. Keep in mind that planes extend
infinitely in two directions, but to represent them in a cleaner manner
the meshes do not extend infinitely.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. Lastly, you
can rotate the objects by clicking the left mouse button and dragging the mouse. It is
worth noting that Planes almost always collide in 3D. This means as soon as you
rotate a plane it will be difficult- if not impossible to get them to not collide again.

This demo detects collisions between planes by first testing if their normals are the same. 
If two planes do not have the same (or anti-parallel) normals they must collide 
(remember, planes extend infinitely in two directions). If two planes do have the same
or antiparallel normals they may still be colliding if and only if they are on top of each other.
This is just a matter of testing if the center of one plane lies on another plane. We can do this
using the dot product and the mathematical definition of a plane.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shadar variables
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
//uniforms
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

//A plane collider struct
struct Plane
{
	glm::vec3 normal;

	///
	//Generates a plane with a normal pointing down the X axis
	Plane::Plane()
	{
		normal = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	///
	//Generates a plane with a given normal
	Plane::Plane(glm::vec3 norm)
	{
		normal = norm;
	}
};

struct Mesh* plane1;
struct Mesh* plane2;

struct Mesh* selectedShape;

struct Plane* plane1Collider;
struct Plane* plane2Collider;


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

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between two planes.
//
//Overview:
//	Planes almost always collide in 3D. This algorithm detects collisions between planes by first
//	testing if their normals are the same. If two planes do not have the same (or anti-parallel) normals
//	they must collide (remember, planes extend infinitely in two directions). If two planes do have the same
//	or antiparallel normals they may still be colliding if and only if they are on top of each other.
//	This is just a matter of testing if the center of one plane lies on another plane. We can do this
//	using the dot product and the mathematical definition of a plane.
//
//Parameters:
//	p1Collider: The first plane's collider
//	p1ModelMatrix: The first plane's model to world transformation matrix
//	p2Collider: The second plane's collider
//	p2ModelMatrix: The second plane's model to world transformation matrix
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const Plane &p1Collider, const glm::mat4 &p1ModelMatrix, const Plane &p2Collider, const glm::mat4 &p2ModelMatrix)
{
	//Step1: convert the normals into worldspace
	glm::vec4 worldNorm1 = p1ModelMatrix * glm::vec4(p1Collider.normal, 0.0f);
	glm::vec4 worldNorm2 = p2ModelMatrix * glm::vec4(p1Collider.normal, 0.0f);

	//Step2: Are the normals the same or opposite?
	//If not the planes must collide at some point!
	if (glm::length(worldNorm1 - worldNorm2) < FLT_EPSILON || glm::length(worldNorm1 + worldNorm2) < FLT_EPSILON)
	{
		//Step3: If the normals are the same, the only way the planes can be colliding is if they are on top of each other.
		//We must determine a point in worldspace on each plane
		glm::vec4 worldPoint1 = p1ModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec4 worldPoint2 = p2ModelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		//If the points on the planes return the same dot product with one of the (equal) normals, they are on top of each other.
		if (fabs(glm::dot(worldPoint1, worldNorm1) - glm::dot(worldPoint2, worldNorm2)) < FLT_EPSILON) return true;
		else return false;

	}

	return true;
}

// This runs once every physics timestep.
void update()
{
	//Check if the mouse button is being pressed
	if (isMousePressed)
	{
		//Get the current mouse position
		double currentMouseX, currentMouseY;
		glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

		//Get the difference in mouse position from last frame
		float deltaMouseX = (float)(currentMouseX - prevMouseX);
		float deltaMouseY = (float)(currentMouseY - prevMouseY);

		glm::mat4 yaw;
		glm::mat4 pitch;

		//Rotate the selected shape by an angle equal to the mouse movement
		if (deltaMouseX != 0.0f)
		{
			yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
			
		}
		if (deltaMouseY != 0.0f)
		{
			pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
			
		}

		selectedShape->rotation = yaw * pitch * selectedShape->rotation;

		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

	}

	if (TestCollision(*plane1Collider, plane1->GetModelMatrix(), *plane2Collider, plane2->GetModelMatrix()))
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
	plane1->Draw();
	plane2->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == plane1 ? plane2 : plane1;

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
	window = glfwCreateWindow(800, 800, "Plane - Plane Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();



	//Generate the Plane1 mesh
	struct Vertex planeVerts[6];
	planeVerts[0] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[1] = { 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[2] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[3] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[4] = { 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[5] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };

	plane1 = new struct Mesh(6, planeVerts, GL_TRIANGLES);

	//Translate the plane
	plane1->translation = glm::translate(plane1->translation, glm::vec3(0.25f, 0.0f, 0.0f));

	//Generate plane2 mesh
	planeVerts[0].b = \
		planeVerts[1].b = \
		planeVerts[2].b = \
		planeVerts[3].b = \
		planeVerts[4].b = \
		planeVerts[5].b = 0.0f;

	planeVerts[0].g = \
		planeVerts[1].g = \
		planeVerts[2].g = \
		planeVerts[3].g = \
		planeVerts[4].g = \
		planeVerts[5].g = 1.0f;



	plane2 = new struct Mesh(6, planeVerts, GL_TRIANGLES);

	//Translate the plane
	plane2->translation = glm::translate(plane2->translation, glm::vec3(-0.15f, 0.0f, 0.0f));

	//Set the selected shape
	selectedShape = plane1;

	//Generate plane collider

	//Get two edges of the plane and take the cross product for the normal (Or just hardcode it, for example we know the normal to this plane
	//Will be the Z axis, because the plane mesh lies in the XY Plane to start.
	glm::vec3 edge1(planeVerts[0].x - planeVerts[1].x, planeVerts[0].y - planeVerts[1].y, planeVerts[0].z - planeVerts[1].z);
	glm::vec3 edge2(planeVerts[1].x - planeVerts[2].x, planeVerts[1].y - planeVerts[2].y, planeVerts[1].z - planeVerts[2].z);

	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

	plane1Collider = new struct Plane(normal);
	plane2Collider = new struct Plane(normal);

	//Print controls
	std::cout << "Use WASD to move the selected shape in the XY plane.\nUse left CTRL & left shift to move the selected shape along Z axis.\n";
	std::cout << "Left click and drag the mouse to rotate the selected shape.\nUse spacebar to swap the selected shape.\n";

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

	delete plane1;
	delete plane2;

	//Delete Colliders
	delete plane1Collider;
	delete plane2Collider;

	// Frees up GLFW memory
	glfwTerminate();
}