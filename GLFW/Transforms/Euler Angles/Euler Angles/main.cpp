/*
Title: Euler Angles
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
This is a demonstration of using euler angles (pronounced "oiler"-- by the way)
to describe an orientation in 3D space. The demo contains 3 lines representing 
the 3 cardinal axes:	X (red), Y (green), and Z (blue)
Which can be rotated to any orientation.

Euler angles are 3 scalar values which measure angles. The first angle,
the heading, measure the angle of rotation around the Y axis from the 
initial orientation (the frame of reference created by the cardinal axes X,Y,Z)
to the objects final orientation (X'Y'Z'). The second angle, the Elevation, measures 
the angle of rotation around the X axis from the initial X-Z plane to the 
X'-Z' plane created by the object's final orientation. The third and final
angle, the Roll, is the angle of rotation around the Z' axis to reach the final
orientation. It should be noted that the Z' axis is the Z axis of the object in worldspace, 
not the Z axis of the world.

An interesting side effect of euler angles is called Gimbal Lock, which is able to be experienced
in this simulation. Gimble lock is due to the fact that the axis of rotation of the heading never
changes, but the axis of rotation of the Roll does. As a result of this, it is possible to cause
the axis of rotation of the Roll to change in such a way that it has the same axis of rotation
as the heading. When this happens a degree of freedom is lost and it is only possible
to rotate about 2 linearly independent independent directions rather than 3.

In this simulation the Q and E buttons will alter the heading
W and S will alter the pitch
and A and D will alter the roll

In order to experience Gimble Lock, alter the pitch using W or S
such that the Z' axis (blue axis) is pointing straight up or straight down.
Once this is achieved you can see that the Q and E buttons will
accomplish the same action as the A and D buttons.

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
	//Parameterized constructor, constructs a line collider from a to b
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
	static float XAngle = 0.0f;	//Angle of rotation around the X axis, or Elevation
	static float YAngle = 0.0f;	//Angle of rotation around the Y axis, or Heading
	static float ZAngle = 0.0f;	//Angle of rotation around the Z axis, or Roll
	static float rotationSpeed = 3.0f;

	//Get change in time
	static double prevTime = glfwGetTime();
	double currTime = glfwGetTime();
	float dt = (float)(currTime - prevTime);
	prevTime = currTime;

	//Update euler angles based on input
	
	//Heading
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		YAngle += rotationSpeed * dt;
	}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		YAngle -= rotationSpeed * dt;
	}

	//Elevation
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		XAngle += rotationSpeed * dt;
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		XAngle -= rotationSpeed * dt;
	}

	//Roll
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		ZAngle += rotationSpeed * dt;
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		ZAngle -= rotationSpeed * dt;
	}

	//Reset
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		XAngle = YAngle = ZAngle = 0.0f;
	}

	//Create the Heading Elevation and Roll rotation matrices
	glm::mat3 heading = glm::mat3(glm::rotate(glm::mat4(1.0f), YAngle, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::mat3 elevation = glm::mat3(glm::rotate(glm::mat4(1.0f), XAngle, glm::vec3(1.0f, 0.0f, 0.0f)));
	glm::mat3 roll = glm::mat3(glm::rotate(glm::mat4(1.0f), ZAngle, glm::vec3(0.0f, 0.0f, 1.0f)));

	//create the final rotation matrix
	glm::mat3 rotation = heading * elevation * roll;

	//Set the rotation matrix of the axes
	right->rotation = up->rotation = forward->rotation = rotation;
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
	window = glfwCreateWindow(800, 800, "Euler Angles", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	right = new Line(glm::vec3(0.2f, 0.0f, 0.0f));
	up = new Line(glm::vec3(0.0f, 0.2f, 0.0f));
	forward = new Line(glm::vec3(0.0f, 0.0f, -0.2f));

	//Print controls
	std::cout << "Conrols:\nUse Q and E to change the Heading angle\nUse W and S to change the Elevation angle\nUse A and D to change the Roll angle\n";
	std::cout << "\nUse R to reset the orientation\n\nIn order to experience Gimble Lock, alter the pitch using W or S\n" << \
		"such that the Z' axis (blue axis) is pointing straight up or straight down." << \
		"\nOnce this is achieved you can see that the Q and E buttons will accomplish\n" << \
		"the same action as the A and D buttons.";

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