/*
Title: Calculating the Moment Of Intertia Tensor (MyRectangle - 2D)
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
This is a demonstration calculating & transforming moments of inertia of a rectangle in 2D.
The demo contains the wireframe of a rectangle with a given mass & dimensions. The 
demo calculates the moment of inertia tensor of the rectangle, and prints it in the console.
Whenever the rectangle rotates, the moment of inertia is transformed into the new coordinate system & re-printed.

The user can use Q and E to rotate the rectangle.

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

//A basic polygon consists of a set of points surrounding a center
//which they are rotated about by a given angle.
struct MyRectangle
{
	glm::vec2 center;					//Center of polygon
	glm::mat2 rotation;					//2x2 rotation matrix
	float width;
	float height;
};

struct MyRectangle rect;		//A MyRectangle
float mass;					//The mass of the MyRectangle
glm::mat2 inertiaTensor;	//The inertia tensor of the MyRectangle in model space


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

	//EAsier to not use a shader for this simple 2D example.


	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT, GL_FILL);
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
glm::mat2 CalculateInertiaTensorOfMyRectangle(MyRectangle r, float m)
{
	glm::mat2 inertia;
	inertia[0][0] = m * powf(r.width, 2.0f) / 12.0f;
	inertia[1][1] = m * powf(r.height, 2.0f) / 12.0f;
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
glm::mat2 RotateMomentOfInertiaTensor(glm::mat2 rotation, glm::mat2 inertia)
{
	glm::mat2 transpose = glm::transpose(rotation);
	return rotation * inertia * transpose;
}

void ReprintInertiaTensor(const glm::mat2 &tensor)
{
	//Move cursor to line 2 column 0
	COORD c;
	c.X = 0;
	c.Y = 2;
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE),
		c
		);
	std::cout << "Inertia Tensor:\n[\t" << tensor[0][0] << "\t" << tensor[0][1] << "\t] \n[\t" << tensor[1][0] << "\t" << tensor[1][1] << "\t] ";

}

// This runs once every physics timestep.
void update(float dt)
{
	static float movrate = 0.01f;
	static float rotrate = 0.01f;

	bool changed = false;
	//Set of controls to rotate the selected polygon
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		rect.rotation = glm::mat2(glm::rotate(glm::mat4(rect.rotation), rotrate, glm::vec3(0.0f, 0.0f, 1.0f)));
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		rect.rotation = glm::mat2(glm::rotate(glm::mat4(rect.rotation), -rotrate, glm::vec3(0.0f, 0.0f, 1.0f)));
		changed = true;
	}

	//Calculate transformed inertia tensor
	if(changed)
	{
		glm::mat2 transformed = RotateMomentOfInertiaTensor(rect.rotation, inertiaTensor);
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

	// Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 0.0);



	// code to draw the line between two points.This is using a deprecated method. This should be done in the shaders. But for this simple simulaton this is enough.
	glUseProgram(0);
	glLineWidth(2.5f);
	glPointSize(8.0f);
	glColor3f(1.0f , 1.0f, 1.0f);

	//We must get the points of the MyRectangle in world space
	std::vector<glm::vec2> rectPoints;
	rectPoints.push_back (rect.center + rect.rotation * glm::vec2(-rect.width * 0.5f, rect.height * 0.5f));
	rectPoints.push_back (rect.center + rect.rotation * glm::vec2(-rect.width * 0.5f, -rect.height * 0.5f));
	rectPoints.push_back (rect.center + rect.rotation * glm::vec2(rect.width * 0.5f, -rect.height * 0.5f));
	rectPoints.push_back (rect.center + rect.rotation * glm::vec2(rect.width * 0.5f, rect.height * 0.5f));



	//Draw polygon 1
	int size = rectPoints.size();
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < size; i++)
	{
		glVertex3f(rectPoints[i].x, rectPoints[i].y, 0.0f);
	}
	glEnd();
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

	//Initialize MyRectangle
	rect.width = 0.5f;
	rect.height = 0.1f;
	rect.center = glm::vec2(0.0f);
	rect.rotation = glm::mat2(1.0f);

	mass = 10.0f;
	inertiaTensor = CalculateInertiaTensorOfMyRectangle(rect, mass);

	//Print controls
	std::cout << "Controls:\nUse Q and E to rotate the rectangle and calculate the new moment of inertia.\n";

	//Print moment of inertia
	std::cout << std::fixed;
	std::cout.precision(4);

	std::cout << "Inertia Tensor:\n[\t" << inertiaTensor[0][0] << "\t" << inertiaTensor[0][1] << "\t]\n[\t" << inertiaTensor[1][0] << "\t" << inertiaTensor[1][1] << "\t]";

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