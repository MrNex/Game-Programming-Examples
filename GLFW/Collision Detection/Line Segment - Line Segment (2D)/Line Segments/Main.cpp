/*
Title: Line Segment - Line Segment (2D)
File Name: Main.cpp
Copyright © 2015
Author: Nicholas Gallagher
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
This is an example to detect the intersection of two line segments in 2D. 
You can control the two end-points of the line segment which a small part of the line, and move them using "w,a,s,d" and "i,j,k,l" respectively.
You can swap the line segment you are controlling with the space bar.
The line turns red when an intersection is detected, and is green when there is no intersection. 

References:
AABB2D by Brockton Roth
Line - Circle by Srinivasan Thiagarajan
*/

#include "GLIncludes.h"
#include "GameObject.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>


struct Line
{
	glm::vec2 point1;
	glm::vec2 point2;

};

//Shader Program
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
//Uniforms
GLuint uniMVP;
glm::mat4 proj;
glm::mat4 view;
glm::mat4 PV;
glm::vec3 lineColor(1.0f, 1.0f, 1.0f);
glm::mat4 MVP;
glm::mat4 MVP2;
// Variable to set the sensitivity of the input controls.
float movrate = 0.05f;
// Reference to the window object being created by GLFW.
GLFWwindow* window;
// An array of vertices stored in an std::vector for our object.
std::vector<VertexFormat> vertices;

Line line1, line2;
Line* selectedLine = &line1;

///
//Tests for the intersection of two line segments.
//If a point can be found which is on both lines between the ends of both lines,
//They must be colliding.
//
//Returns:
//	true if the lines are intersecting, else false
bool TestIntersection()
{
	//Get the direction vectors of the lines
	glm::vec2 dir1 = line1.point2 - line1.point1;	//Direction of line 1
	glm::vec2 dir2 = line2.point2 - line2.point1;	//Direction of line 2

	//We know that parallel lines will never cross, in order to avoid a possible divide by 0 case, we must make sure the lines are not parallel.
	//The dot product is equal to the magnitudes of the vectors * the cosine of the angle between them.
	//This means that the dot product will be equal to + or - the product of the magnitudes if the lines are parallel or anti-parallel (Cosine of 1 and -1 respectively)
	float magProd = glm::length(dir1) * glm::length(dir2);
	if (fabs(glm::dot(dir1, dir2)) == magProd) return false;	//Lines parallel

	//Beyond this point we can assume the lines are not parallel!

	//Next we must make sure no line is vertical or it will have an undefined slope and cause a divide by zero error.
	if (fabs(dir2.x) > FLT_EPSILON)
	{
		//Consider the parametric form of the lines line1 and line2 as such:
		//	P = line1.point1 + t * dir1
		//	P = line2.point1 + s * dir2

		//If a single point exists on both lines, P is the same for both equations, and we can equate them
		//	line1.point1 + t * dir1 = line2.point1 + s * dir2
		//	line1.point1 - line2.point1 + t * dir1 = s * dir2

		//We can break this up into a system of equations in the x and y components of the vectors to solve for s (and later, t)
		//In X:
		//	line1.point1.x - line2.point1.x + t * dir1.x = s * dir2.x
		//	(line1.point1.x - line2.point1.x + t * dir1.x) / dir2.x = s
		//And in Y:
		//	line1.point1.y - line2.point1.y + t * dir1.y = s * dir2.y

		//Now we can substitute our solution for s into our equation in Y
		//	line1.point1.y - line2.point1.y + t * dir1.y = dir2.y * (line1.point1.x - line2.point1.x + t * dir1.x) / dir2.x

		//On the RHS we have dir2.y / dir2.x, this can be interpreted as the slope of line2 (which we will denote as m2)
		//	line1.point1.y - line2.point1.y + t * dir1.y = m2 * (line1.point1.x - line2.point1.x + t * dir1.x)
		//	line1.point1.y - line2.point1.y + t * dir1.y = m2 * line1.point1.x - m2 * line2.point1.x + t * m2 * dir1.x

		float m2 = dir2.y / dir2.x;

		//And if we isolate all terms containing t:
		//	line1.point1.y - line2.point1.y - m2 * line1.point1.x + m2 * line2.point1.x = t * m2 * dir1.x - t * dir1.y
		//	line1.point1.y - line2.point1.y - m2 * line1.point1.x + m2 * line2.point1.x = t * (m2 * dir1.x - dir1.y) 
		//	t = (line1.point1.y - line2.point1.y - m2 * line1.point1.x + m2 * line2.point1.x) / (m2 * dir1.x - dir1.y)

		float t = (line1.point1.y - line2.point1.y - m2 * line1.point1.x + m2 * line2.point1.x) / (m2 * dir1.x - dir1.y);

		//t now represents the amount of the vectors dir1 we would need to traverse from line1.point1 to reach a point on line 2, as dictated by:
		//	P = line1.point1 + t * dir1

		//The magnitude of dir1 is the length of line 1, therefore if 0 <= t <= 1, line2 will intersect line1 within line1's segment.
		//The vise versa isn't necessarily true through, so we must use t to solve to s and make the same test.
		float s = (line1.point1.x - line2.point1.x + t * dir1.x) / dir2.x;

		//Else, they do not!
		return 0.0f <= t && t <= 1.0f && 0.0f <= s && s <= 1.0f;
	}
	else
	{
		//If line 2 does have a slope which is undefined, we must solve using a method in which we compute the answer by line1's slope.
		//Line1's slope could not be undefined as well because we have established the lines are not parallel or anti-parallel.
		//To arrive at the answer you use the same method outlined above, but you must solve for t first instead of s.
		//You arrive at the following similar looking equation:
		float m1 = dir1.y / dir1.x;
		float s = (line2.point1.y - line1.point1.y - m1 * line2.point1.x + m1 * line1.point1.x) / (m1 * dir2.x - dir2.y);
		float t = (line2.point1.x - line1.point1.x + s * dir2.x) / dir1.x;

		return 0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f;
	}
}

//Checks for line intersections and adjusts color accordingly
void update()
{
	//test if they are intersection. If so, change the color of the line segment.
	if (TestIntersection())
	{
		//Red color
		lineColor.x = 1.0f;
		lineColor.y = 0.0f;
		lineColor.z = 0.0f;
	}
	else
	{	
		//Green color
		lineColor.x = 0.0f;
		lineColor.y = 1.0f;
		lineColor.z = 0.0f;
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0, 1.0, 1.0, 1.0);



	// code to draw the line between two points.This is using a deprecated method. This should be done in the shaders. But for simple physics implementations and debugging,
	// this is enough. 
	glUseProgram(0);
	glLineWidth(2.5f);
	glColor3f(lineColor.x, lineColor.y, lineColor.z);
	glBegin(GL_LINES);
	glVertex3f(line1.point1.x, line1.point1.y, 0.0f);
	glVertex3f(line1.point2.x, line1.point2.y, 0.0f);
	glVertex3f(line2.point1.x, line2.point1.y, 0.0f);
	glVertex3f(line2.point2.x, line2.point2.y, 0.0f);
	
	glEnd();

}

// This function is used to handle key inputs.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//This control will alter the line selected
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		selectedLine = selectedLine == &line1 ? &line2 : &line1;

	//This set of controls are used to move one point (point1) of the line (line1).
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		selectedLine->point1.y += movrate;
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		selectedLine->point1.x -= movrate;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		selectedLine->point1.y -= movrate;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		selectedLine->point1.x += movrate;

	//This set of controls are used to move one point (point2) of the line (line1).
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		selectedLine->point2.y += movrate;
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		selectedLine->point2.x -= movrate;
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		selectedLine->point2.y -= movrate;
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		selectedLine->point2.x += movrate;
}

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

// Create shader from source
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

	//Set the starting points of the first line segment
	line1.point1 = glm::vec2(-0.5f, 0.0f);
	line1.point2 = glm::vec2(0.5f, 0.0f);
	
	//Set the starting points of the second line segment
	line2.point1 = glm::vec2(0.0f, -0.5f);
	line2.point2 = glm::vec2(0.0f, 0.5f);


	//Generate shader program
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	//Generate uniforms
	uniMVP = glGetUniformLocation(program, "translation_2D");

	//SEt options
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Line Segments Intersection Test", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	//Print instructions
	std::cout << "Controls:\nUse WASD and IJKL to control and move the endpoints of the line.\nSwap the line you control with spacebar.";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
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

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}