/*
Title: Minkowski Difference
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
This is a demonstration of the Minkowski Difference operation. The Minkowski Difference
is a binary operation performed on two sets of vectors which computes each difference of
each element in set1 with every element from set2. The Minkowski Difference is extremely
useful for collision detection.

This demonstration contains two polygons, one red and one green.
The convex hull formed by the minkowski difference of the two
polygons is drawn in white. The origin of the coordinate system
is also drawn in cyan. Note that when the two polygons are colliding,
the convex hull formed by the minkowski difference contains the origin 
of the coordinate system.

The user can move the selected polygon using WASD.
The user can also rotate the selected polygon using Q and E.
The user can also swap the selected polygon using spacebar.

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

struct Polygon
{
	glm::vec2 position;
	glm::mat2 rotation;
	std::vector<glm::vec2> vertices;
};

struct Polygon poly1;
struct Polygon poly2;

struct Polygon* selectedPoly;

std::vector<glm::vec2> differenceHull;

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();
	glEnable(GL_DEPTH_TEST);

	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPointSize(5.0f);
	glEnable(GL_POINT_SMOOTH);
}


#pragma endregion Helper_functions


///
//Determines if edge 1 is counter clockwise with respect to edge 2
//
//Parameters:
//	e1: The edge to check if it is counter clockwise
//	e2: The edge we are checking with respect to
//
//Returns:
//	true if edge1 is CCW with respect to edge 2, else false
bool IsCounterClockwise(const glm::vec2& e1, const glm::vec2& e2)
{
	if(e1.x * e2.y - e2.x * e1.y > 0.0f) return true;
	else return false;
}

///
//Uses the Jarvis March algorithm to determine the smallest convex hull around
//a 2D set of points in the XY plane.
//
//Parameters:
//	hullList: The list to store all points on the hull
//	pointSet: The set of points to calculate the convex hull from
void JarvisMarch(std::vector<glm::vec2> &hullList, const std::vector<glm::vec2> &pointSet)
{
	//Step 1: Find the left-most point
	glm::vec2 pointOnHull = pointSet[0];
	int size = pointSet.size();
	for(int i = 1; i < size; i++)
	{
		if(pointSet[i].x < pointOnHull.x) pointOnHull = pointSet[i];
	}

	//Step 2: Create second variable to hold prospective points on the hull which can still be outruled
	glm::vec2 endPoint = pointOnHull;

	//Until we loop back around to the left-most point
	while(hullList.size() <= 0 || endPoint != hullList[0])
	{
		//Add the current point to the hull list
		hullList.push_back(pointOnHull);

		//Set the current prospective point as the first point
		endPoint = pointSet[0];

		for(int i = 1; i < size; i++)
		{
			//Construct an edge from the last point on the hull to the ith point
			glm::vec2 edge1 = (pointSet[i] - pointOnHull);
			//Construct an edge from the last point on the hull to the current prospective point
			glm::vec2 edge2 = glm::vec2(endPoint - pointOnHull);

			//If the edge to the ith point is counter clockwise with respect to the edge to the prospective point
			//Set the prospective point to the point at i
			if(endPoint == pointOnHull || IsCounterClockwise(edge1, edge2))
			{
				endPoint = pointSet[i];
			}
		}

		//Set the point on hull
		pointOnHull = endPoint;
	}
}

///
//Calculates the Minkowski Difference of 2 sets of points
//The minkowski difference is the set of points containing the difference of every combination of a point from each set
//
//Parameters:
//	destination: A reference to a list to store the minkowski difference in
//	op1: The first set of points to perform the minkowski difference on
//	op2: The second set of points to perform the minkowski difference on
void MinkowskiDifference(std::vector<glm::vec2> &destination, const std::vector<glm::vec2> &op1, const std::vector<glm::vec2> &op2)
{
	int op1Size = op1.size();
	int op2Size = op2.size();
	for(int i = 0; i < op1Size; i++)
	{
		for(int j = 0; j < op2Size; j++)
		{
			destination.push_back(op1[i] - op2[j]);
		}
	}
}

// This runs once every physics timestep.
void update(float dt)
{	
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		selectedPoly->position.y += 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		selectedPoly->position.y -= 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		selectedPoly->position.x += 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		selectedPoly->position.x -= 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		selectedPoly->rotation = glm::mat2(glm::rotate(glm::mat4(selectedPoly->rotation), 0.01f, glm::vec3(0.0f, 0.0f, 1.0f)));
	}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		selectedPoly->rotation = glm::mat2(glm::rotate(glm::mat4(selectedPoly->rotation), -0.01f, glm::vec3(0.0f, 0.0f, 1.0f)));
	}

	//Clear the current minkowski sum's hull
	differenceHull.clear();

	//Get the points of the two polygons in worldspace
	std::vector<glm::vec2> set1;
	std::vector<glm::vec2> set2;

	for(int i = 0; i < poly1.vertices.size(); i++)
	{
		set1.push_back(poly1.position + poly1.rotation * poly1.vertices[i]);
	}

	for(int i = 0; i < poly2.vertices.size(); i++)
	{
		set2.push_back(poly2.position + poly2.rotation * poly2.vertices[i]);
	}


	//Calculate the new set of points for the minkowski sum
	std::vector<glm::vec2> differenceSet;

	MinkowskiDifference(differenceSet, set1, set2);

	//Calculate the new hull of the minkowski sum
	JarvisMarch(differenceHull, differenceSet);
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
	glUseProgram(0);

	// Draw the Gameobjects

	//Draw the origin
	glPointSize(4.0f);
	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_POINTS);
	glVertex2f(0.0f, 0.0f);
	glEnd();

	//Draw polygon 1
	glLineWidth(2.0f);
	int size = poly1.vertices.size();
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	glm::vec2 worldPoint;
	for(int i = 0; i < size; i++)
	{
		worldPoint = poly1.position + poly1.rotation * poly1.vertices[i]; 
		glVertex2f(worldPoint.x, worldPoint.y);
	}
	glEnd();

	//Draw polygon 2
	size = poly2.vertices.size();
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < size; i++)
	{
		worldPoint = poly2.position + poly2.rotation * poly2.vertices[i]; 
		glVertex2f(worldPoint.x, worldPoint.y);
	}
	glEnd();

	//Draw minkowski difference
	glLineWidth(4.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	size = differenceHull.size();
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < size; i++)
	{ 
		glVertex2f(differenceHull[i].x, differenceHull[i].y);
	}
	glEnd();


}

void OnKeyPress(GLFWwindow* win, int key, int scan, int action, int mods)
{
	if(action == GLFW_PRESS && key == GLFW_KEY_SPACE)
	{
		selectedPoly = selectedPoly == &poly1 ? &poly2 : &poly1;
	}
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Minkowski Difference", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();
	glfwSetKeyCallback(window, OnKeyPress);

	//Generate the first polygon
	poly1.vertices.push_back(glm::vec2(0.1f, 0.0f));
	poly1.vertices.push_back(glm::vec2(0.0f, 0.3f));
	poly1.vertices.push_back(glm::vec2(-0.1f, 0.0f));
	poly1.vertices.push_back(glm::vec2(0.0f, -0.1f));

	poly1.position = glm::vec2(0.3f, 0.0f);
	poly1.rotation = glm::mat2(1.0f);

	//Generate the second polygon
	poly2.vertices.push_back(glm::vec2(0.2f, -0.1f));
	poly2.vertices.push_back(glm::vec2(0.2f, 0.2f));
	poly2.vertices.push_back(glm::vec2(-0.2f, 0.05f));
	poly2.vertices.push_back(glm::vec2(-0.2f, -0.1f));

	poly2.position = glm::vec2(-0.3f, 0.0f);
	poly2.rotation = glm::mat2(1.0f);

	selectedPoly = &poly1;

	//Print controls
	std::cout << "Controls:\nUse WASD to move the selected polygon.\nUse Q and E to rotate the selected polygon.\nUse spacebar to swap the selected polygon.\n";

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


	// Frees up GLFW memory
	glfwTerminate();
}