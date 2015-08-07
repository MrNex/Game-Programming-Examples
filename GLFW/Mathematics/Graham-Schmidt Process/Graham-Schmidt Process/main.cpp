/*
Title: Graham-Schmidt Process
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
This is a demonstration of an implementation of the Graham-Schmidt process.
The demo contains 3 axes which kept mutually perpendicular through the Graham-Schmidt
process. The red axis represents the axis which is controlled by user input.

The Graham Schmidt process is an algorithm for taking a single vector and computing
the rest of the vectors needed to span that space. It is useful for generating vectors
which represent a coordinate system of a plane.

The red axis can be rotated about the Y axis by holding the left mouse button
and moving the mouse left and right. The red axis can be rotated about the X axis
by holding the left mouse button and moving the mouse up and down.

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection - Christer Ericson
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

//Line struct
struct Line
{
	glm::vec3 direction;

	///
	//Default constructor, constructs a line collider along the X axis
	Line::Line()
	{
		direction = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	///
	//Parameterized constructor, constructs a line collider from a to b
	Line::Line(glm::vec3 dir)
	{
		direction = dir;
	}
};

struct Line* right;
struct Line* up;
struct Line* forward;

bool isMousePressed = false;
double prevMouseX = 0.0f;
double prevMouseY = 0.0f;

//Out of order Function declarations
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
//void mouse_callback(GLFWwindow* window, int button, int action, int mods);

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();
	glEnable(GL_DEPTH_TEST);


	//Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPointSize(3.0f);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Projects a vector u onto a vector v
//
//Parameters:
//	u: The vector to project
//	v: The vector to project onto
//
//Returns:
//	The vector representing the projection of u onto v.
glm::vec3 Project(const glm::vec3& u, const glm::vec3& v)
{
	float scale = glm::dot(u, v) / glm::dot(v, v);
	return scale * v;
}

///
//Performs the Graham Schmidt process on a vector.
//
//Overview:
//	The purpose of the Graham Schmidt process is to generate a basis for a space
//	which contains a given vector. For example, if you have a vector X, which vectors, Y and Z,
//	would you neet to be able to reach any point in 3-Dimensional space using a linear combination of X, Y, and Z
//	with the constraint that all vectors must be mutually perpendicular
//
//Parameters:
//	startingVector: A unit vector to use as the start of the basis
//	v1: A reference to a vector in which to store the first generated basis vector in
//	v2: A reference to a vector in which to store the second generated basis vector in
void GrahamSchmidt(const glm::vec3& startingVector, glm::vec3& v1, glm::vec3& v2)
{
	//Step 1: Come up with a basis for the space
	//We know that the X, Y, and Z vectors form a basis for 3D space
	glm::vec3 x(1.0f, 0.0f, 0.0f);
	glm::vec3 y(0.0f, 1.0f, 0.0f);
	glm::vec3 z(0.0f, 0.0f, 1.0f);

	//Step 2: Get the projection of the X axis onto the startingVector for the basis and subtract it from the X axis
	//You will be left with only the part of the X axis which is perpendicular to the starting vector
	v1 = x - Project(x, startingVector);

	//Step 3: Make sure v1 is not the zero vector (This will happen if the starting vector is the X axis)
	//If it isn't we must normalize it. If it is we must perform the computation again using another vector in our
	//basis from Step 1
	if(glm::length(v1) < FLT_EPSILON)
	{
		//If our vector is the zero vector we will repeat the process using the Z vector
		v1 = glm::normalize(z - Project(z, startingVector));
	}
	else
	{
		v1 = glm::normalize(v1);
	}

	//Step 4: Get the projection of the next vector (y axis) in our basis onto the subspace formed by the starting vector and v1,
	//and subtract it from the vector (y axis). We will be left with only the portion of the Y axis which is perpendicular to
	//the subspace formed by startingVector and v1
	v2 = y - (Project(y, startingVector) + Project(y, v1));

	//Step 5: Make sure v2 is not the zero vector (This will happen if the starting vector is the Y axis)
	//If it isn't the zero vector we must normalize it. If it is the zero vector we must perform the computation again using
	//another vector in our basis from step 1.
	if(glm::length(v2) < FLT_EPSILON)
	{
		//If our vector is the zero vector we will repeat the process using the Z vector
		v2 = glm::normalize(z - (Project(z, startingVector) + Project(z, v1)));
	}
	else
	{
		v2 = glm::normalize(v2);
	}
}

// This runs once every physics timestep.
void update()
{
	//Get the current mouse position
	double currentMouseX, currentMouseY;
	glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

	//Check if the mouse button is being pressed
	if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
	{
		//Get the difference in mouse position from last frame
		float deltaMouseX = (float)(currentMouseX - prevMouseX);
		float deltaMouseY = (float)(currentMouseY - prevMouseY);

		static float rotationSpeed = 0.01f;
		glm::mat4 yaw;
		glm::mat4 pitch;

		//Rotate the selected shape by an angle equal to the mouse movement
		if (deltaMouseX != 0.0f)
			yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));		

		if (deltaMouseY != 0.0f)
			pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * -rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));

		right->direction = glm::vec3(pitch * yaw * glm::vec4(glm::normalize(right->direction), 0.0f));

		//Perform the graham schmidt process to generate two more mutually orthogonal vectors which,
		//with the right vector, will form a basis for 3D space
		GrahamSchmidt(right->direction, up->direction, forward->direction);

		right->direction *= 0.2f;
		up->direction *= 0.2f;
		forward->direction *= 0.2f;

	}



	//Update previous positions
	prevMouseX = currentMouseX;
	prevMouseY = currentMouseY;
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	//glUseProgram(program);
	glUseProgram(0);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	// Draw the Gameobjects
	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	//Draw the right vector
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3fv(&right->direction.x);


	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	//Draw the up vector
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3fv(&up->direction.x);


	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

	//Draw the forward vector
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3fv(&forward->direction.x);

	glEnd();
}

#pragma endregion util_Functions


void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Graham-Schmidt Process", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	right = new Line(glm::vec3(0.2f, 0.0f, 0.0f));
	up = new Line(glm::vec3(0.0f, 0.2f, 0.0f));
	forward = new Line(glm::vec3(0.0f, 0.0f, -0.2f));

	//Print controls
	std::cout << "Conrols:\nHold the left mouse button and:\n\tMove the mouse left and right to rotate the red axis around the Y axis.\n";
	std::cout << "\tMove the mouse up and down to rotate the red axis around the X axis.\n";

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

	delete right;
	delete up;
	delete forward;

	// Frees up GLFW memory
	glfwTerminate();
}