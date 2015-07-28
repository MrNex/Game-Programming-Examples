/*
Title: Minkowski Sum
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
This is a demonstration of the Minkowski Sum operation. The Minkowski Sum
is a binary operation performed on two sets of vectors which computes each sum of
each element in set1 with every element from set2. The Minkowski sum has 
a variety of uses. One of the most prominent is it's use in continuous
collision detection of dynamic objects. 

This demonstration contains a polygon and a line segment. The line 
segment represents a path of motion of the polygon over a timestep.
The polygon is drawn in red and the line segment is drawn in green. 
The minkowski sum of the line and the polygon is drawn in white and 
represents the space which the polygon will enter throughout the timestep.

The user can move the polygon using WASD.
The user can also rotate the polygon using Q and E.

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



glm::vec2 polyPosition;
glm::mat2 polyRotation;
std::vector<glm::vec2> polygon;

glm::vec2 lineEndPoint;

std::vector<glm::vec2> sumHull;

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
//Calculates the Minkowski Sum of 2 sets of points
//The minkowski sum is the set of points containing the sum of every combination of a point from each set
//
//Parameters:
//	destination: A reference to a list to store the minkowski sum in
//	op1: The first set of points to perform the minkowski sum on
//	op2: The second set of points to perform the minkowski sum on
void MinkowskiSum(std::vector<glm::vec2> &destination, const std::vector<glm::vec2> &op1, const std::vector<glm::vec2> &op2)
{
	int op1Size = op1.size();
	int op2Size = op2.size();
	for(int i = 0; i < op1Size; i++)
	{
		for(int j = 0; j < op2Size; j++)
		{
			destination.push_back(op1[i] + op2[j]);
		}
	}
}

// This runs once every physics timestep.
void update(float dt)
{	
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		polyPosition.y += 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		polyPosition.y -= 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		polyPosition.x += 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		polyPosition.x -= 0.01f;
	}
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		polyRotation = glm::mat2(glm::rotate(glm::mat4(polyRotation), 0.01f, glm::vec3(0.0f, 0.0f, 1.0f)));
	}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		polyRotation = glm::mat2(glm::rotate(glm::mat4(polyRotation), -0.01f, glm::vec3(0.0f, 0.0f, 1.0f)));
	}

	//Clear the current minkowski sum's hull
	sumHull.clear();

	
	//Create a set of points for the line
	std::vector<glm::vec2> lineSet;
	lineSet.push_back(polyPosition);
	lineSet.push_back(lineEndPoint);

	//Create a set of points for the oriented polygon points
	std::vector<glm::vec2> polySet;
	int size = polygon.size();
	for(int i = 0; i < size; i++)
	{
		polySet.push_back(polyRotation * polygon[i]);
	}

	//Calculate the new set of points for the minkowski sum
	std::vector<glm::vec2> sumSet;

	MinkowskiSum(sumSet, polySet, lineSet);

	//Calculate the new hull of the minkowski sum
	JarvisMarch(sumHull, sumSet);
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

	glLineWidth(2.0f);
	// Draw the Gameobjects
	int size = polygon.size();
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	glm::vec2 worldPoint;
	for(int i = 0; i < size; i++)
	{
		worldPoint = polyPosition + polyRotation * polygon[i]; 
		glVertex2f(worldPoint.x, worldPoint.y);
	}
	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2f(polyPosition.x, polyPosition.y);
	glVertex2f(lineEndPoint.x, lineEndPoint.y);
	glEnd();

	glLineWidth(4.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	size = sumHull.size();
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < size; i++)
	{ 
		glVertex2f(sumHull[i].x, sumHull[i].y);
	}
	glEnd();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Minkowski Sum", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate the polygon
	polygon.push_back(glm::vec2(0.1f, 0.0f));
	polygon.push_back(glm::vec2(0.0f, 0.3f));
	polygon.push_back(glm::vec2(-0.1f, 0.0f));
	polygon.push_back(glm::vec2(0.0f, -0.1f));

	polyPosition = glm::vec2(0.0f);
	polyRotation = glm::mat2(1.0f);

	lineEndPoint = glm::vec2(0.5f, 0.0f);

	//Print controls
	std::cout << "Controls:\nUse WASD to move the polygon.\nUse Q and E to rotate the polygon.\n";

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