/*
Title: Rotation Matrix
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
This is a demonstration of using a rotation matrix to describe an 
orientation in 3D space. The demo contains 3 lines representing the 
3 cardinal axes:	X (red), Y (green), and Z (blue)
Which can be rotated to any orientation.

A rotation matrix is a collection of 9 scalars arranged in a 3x3 matrix.
The rotation matrix has special properties including:
- Each column represents the object's local X, Y, and Z axes in world space
- A rotation applied from a matrix can easily be reversed by applying the
inverse of that matrix
- The inverse of a rotation matrix is the transpose
- Multiplying two rotation matrices together gets a rotation matrix which,
when applied, has the same result as the two separate matrices

In this simulation the Q and E buttons will alter the Y component of the axis of rotation
W and S will alter the X component of the axis of rotation
and A and D will alter the Z component of the axis of rotation

References:
3D Math Primer for Graphics and Game Development by Fletcher Dunn & Ian Parberry
Base by Srinivasan Thiagarajan
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data

// Reference to the window object being created by GLFW.
GLFWwindow* window;

//Line struct
struct Line
{
	glm::vec3 direction;
	glm::mat3 rotation;

	///
	//Default constructor, constructs a line collider along the X axis
	Line::Line()
	{
		direction = glm::vec3(1.0f, 0.0f, 0.0f);
		rotation = glm::mat3(1.0f);
	}

	///
	//Parameterized constructor, constructs a line with a specified direction
	Line::Line(glm::vec3 dir)
	{
		direction = dir;
		rotation = glm::mat3(1.0f);
	}
};

struct Line* right;
struct Line* up;
struct Line* forward;

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



// This runs once every physics timestep.
void update()
{
	glm::vec3 axisOfRotation(0.0f);			//The final axis of rotation to construct a rotation matrix around
	static float rotationSpeed = 3.0f;		//Speed of rotation

	//Get change in time
	static double prevTime = glfwGetTime();
	double currTime = glfwGetTime();
	float dt = (float)(currTime - prevTime);
	prevTime = currTime;

	//Update axis of rotation based on input
	//Heading
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		axisOfRotation.y -= 1.0f;
	}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		axisOfRotation.y += 1.0f;
	}

	//Elevation
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		axisOfRotation.x -= 1.0f;
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		axisOfRotation.x += 1.0f;
	}

	//Roll
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		axisOfRotation.z -= 1.0f;
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		axisOfRotation.z += 1.0f;
	}

	//If the axis of rotation is not 0
	if(glm::length(axisOfRotation) > FLT_EPSILON)
	{
		//Create the rotation matrix around the axis
		//I will use GLM to do this here, but for more information on how you can do this
		//See Rodrigues' Rotation Formula in the Mathematics section of the tutorials
		glm::mat3 rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), rotationSpeed * dt, glm::normalize(axisOfRotation)));

		//If space is pressed, apply the rotation from modelSpace
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			//Now rotate the current orientation by the rotation matrix
			right->rotation = right->rotation * rotation;
			up->rotation = up->rotation * rotation;
			forward->rotation = forward->rotation * rotation;
		}
		//Else, apply the rotation from worldspace
		else
		{
			//Now rotate the current orientation by the rotation matrix
			right->rotation = rotation * right->rotation;
			up->rotation = rotation * up->rotation;
			forward->rotation = rotation * forward->rotation;
		}
	}

	//Reset
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		//One of the perks of using rotation matrices as a way of expressing orientation/rotation
		//is the ability to easily reverse a rotation
		//The reversal of any rotation is just the rotation matrices transpose
		//This is due to the fact that a rotation matrix is an orthogonal matrix
		//And all orthogonal matrices inverses are their transpose.
		//
		//glm::mat3 inverse = glm::transpose(right->rotation);
		//
		//right->rotation *= inverse;
		//up->rotation *= inverse;
		//forward->rotation *= inverse;
		//
		//However, because we are resetting the orientation of these objects to be
		//defined by the cardinal axes (X, Y, and Z), we can simply set their rotation to the identity matrix
		//This will have the same effect as the lines above, because any matrix, A, times it's inverse, B,
		//is equal to the identity matrix, I:
		//
		//	A * B = I
		right->rotation = up->rotation = forward->rotation = glm::mat3(1.0f);

	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 1.0);

	//Not using a shader program
	glUseProgram(0);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	//Get the lines in world space
	glm::vec3 transformedRight = right->rotation * right->direction;
	glm::vec3 transformedUp = up->rotation * up->direction;
	glm::vec3 transformedforward = forward->rotation * forward->direction;

	// Draw the Gameobjects
	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

	//Draw the right vector
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3fv(&transformedRight.x);


	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

	//Draw the up vector
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3fv(&transformedUp.x);


	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

	//Draw the forward vector
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3fv(&transformedforward.x);

	glEnd();
}

#pragma endregion util_Functions


void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Rotation Matrix", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	right = new Line(glm::vec3(0.2f, 0.0f, 0.0f));
	up = new Line(glm::vec3(0.0f, 0.2f, 0.0f));
	forward = new Line(glm::vec3(0.0f, 0.0f, -0.2f));

	//Print controls
	std::cout << \
		"Controls:\n" << \
		"Use Q and E to alter the Y component of the Axis of Rotation\n" << \
		"Use W and S to alter the X component of the axis of rotation\n" << \
		"Use A and D to alter the Z component of the axis of rotation\n" << \
		"Hold spacebar while applying a rotation to apply it in model space\n" << \
		"instead of world space\n";

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

	delete right;
	delete up;
	delete forward;

	// Frees up GLFW memory
	glfwTerminate();
}