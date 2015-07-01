/*
Title: Calculating the Moment Of Intertia Tensor (Rectangular Plane)
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
This is a demonstration calculating & transforming moments of inertia of a rectangular plane in 3D.
The demo contains a rectangular plane with a given mass & dimensions. The 
demo calculates the moment of inertia tensor of the rectangle, and prints it in the console.
Whenever the rectangle rotates, the moment of inertia is transformed into the new coordinate system & re-printed.

When doing a simulation in 2D, you only need a single value as your moment of inertia. That value would correspond to the initial
(2, 2) index of the inertia tensor, and it would not change.

The user can click and drag to rotate the rectangle.

References:
http://ocw.mit.edu/courses/aeronautics-and-astronautics/16-07-dynamics-fall-2009/lecture-notes/MIT16_07F09_Lec26.pdf
*/

#include "GLIncludes.h"
#include <Windows.h>

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

struct Mesh* rect;			//A rectangle mesh
float mass;					//The mass of the MyRectangle
glm::mat3 inertiaTensor;	//The inertia tensor of the MyRectangle in model space


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.

bool isMousePressed = false;
double prevMouseX = 0.0f;
double prevMouseY = 0.0f;

//Mouse input function declaration
void mouse_callback(GLFWwindow* window, int button, int action, int mods);

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
}

#pragma endregion Helper_functions

///
//Calculates the inertia tensor in modelspace (at origin) of a MyRectangle
//
//Parameters:
//	r: The MyRectangle to calculate the moment of inertia of
//	m: the mass of the MyRectangle
//
//Returns:
//	The moment of inertia tensor in modelspace for rotation about the origin of the MyRectangle
glm::mat3 CalculateInertiaTensorOfMyRectangle(float width, float height, float m)
{
	glm::mat3 inertia;
	inertia[0][0] = m * powf(width, 2.0f) / 12.0f;
	inertia[1][1] = m * powf(height, 2.0f) / 12.0f;

	//If you're doing a simulation in 2D, this is the only value you need and your moment of inertia can be a scalar!
	inertia[2][2] = m * (powf(width, 2.0f)  + powf(height, 2.0f)) / 12.0f; 
	return inertia;
}

///
//Computes a moment of inertia tensor after a rotation transformation is applied.
//
//Parameters:
//	rotation: The rotation transformation to apply to the tensor
//	inertia: The moment of inertia tensor to get the transformation of
//
//Returns:
//	A moment of inertia tensor about the origin of an object which has been transformed to a given rotation.
glm::mat3 RotateMomentOfInertiaTensor(glm::mat3 rotation, glm::mat3 inertia)
{
	glm::mat3 transpose = glm::transpose(rotation);
	return rotation * inertia * transpose;
}

void ReprintInertiaTensor(const glm::mat3 &tensor)
{
	//Move cursor to line 2 column 0
	COORD c;
	c.X = 0;
	c.Y = 2;
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE),
		c
		);
	std::cout << "Inertia Tensor:\n[\t" << tensor[0][0] << "\t" << tensor[0][1] << "\t" << tensor[0][2] << "\t]\n[\t" <<
		tensor[1][0] << "\t" << tensor[1][1] << "\t" << tensor[1][2] << "\t]\n[\t" <<
		tensor[2][0] << "\t" << tensor[2][1] << "\t" << tensor[2][2] << "\t]";

}

// This runs once every physics timestep.
void update(float dt)
{
	static float movrate = 0.01f;
	static float rotrate = 0.01f;

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
			yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotrate, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		if (deltaMouseY != 0.0f)
		{
			pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * rotrate, glm::vec3(1.0f, 0.0f, 0.0f));

		}

		rect->rotation = yaw * pitch * rect->rotation;

		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

		//Calculate transformed inertia tensor
		glm::mat3 transformed = RotateMomentOfInertiaTensor(glm::mat3(rect->rotation), inertiaTensor);
		ReprintInertiaTensor(transformed);

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
	rect->Draw();


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

	// Create a window
	window = glfwCreateWindow(800, 800, "Calculating the Moment of Inertia Tensor (Rectangular Plane)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate the Plane1 mesh
	struct Vertex planeVerts[6];
	float arr1[] = { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[0] = *((Vertex*)arr1);
	float arr2[] =  { 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[1] =  *((Vertex*)arr2);
	float arr3[] = { -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[2] = *((Vertex*)arr3);
	float arr4[] = { -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[3] = *((Vertex*)arr4);
	float arr5[] = { -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[4] = *((Vertex*)arr5);
	float arr6[] ={ 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[5] = *((Vertex*)arr6);

	rect = new struct Mesh(6, planeVerts, GL_TRIANGLES);

	//Scale the rectangle
	glm::vec3 scale(0.3f, 0.1f, 1.0f);
	rect->scale = glm::scale(glm::mat4(1.0f), scale);

	mass = 10.0f;
	inertiaTensor = CalculateInertiaTensorOfMyRectangle(scale.x, scale.y, mass);

	//Print controls
	std::cout << "Controls:\nClick and drag to rotate the rectangle and calculate the new moment of inertia.\n";

	//Print moment of inertia
	std::cout << std::fixed;
	std::cout.precision(4);

	std::cout << "Inertia Tensor:\n[\t" << inertiaTensor[0][0] << "\t" << inertiaTensor[0][1] << "\t" << inertiaTensor[0][2] << "\t]\n[\t" <<
		inertiaTensor[1][0] << "\t" << inertiaTensor[1][1] << "\t" << inertiaTensor[1][2] << "\t]\n[\t" <<
		inertiaTensor[2][0] << "\t" << inertiaTensor[2][1] << "\t" << inertiaTensor[2][2] << "\t]";
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

	// Frees up GLFW memory
	glfwTerminate();
}