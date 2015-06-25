/*
Title: Bounding Circles
File Name: Main.cpp
Copyright © 2015
Original authors: Brockton Roth
Revision authors: Nicholas Gallagher
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
This is a bounding circle collision test.  This is in 2D.
Contains two circles, both . They are bounded by circles, and when these circles collide they will change colors to red. 
The larger circle can be moved using the arrow keys, likewise the smaller circle can be moved with WASD.
The algorithm will detect any type of collision, including containment.
The circles should be the exact same as their bounding circles, since they circles any rotation applied to the objects will not make a difference.
*/

#include "GLIncludes.h"
#include "GameObject.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

//Shaders
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
//Uniforms
GLuint uniMVP;
GLuint uniHue;
glm::mat4 proj;
glm::mat4 view;
glm::mat4 PV;
glm::mat4 MVP;
glm::mat4 MVP2;
glm::mat4 hue;
// Reference to the window object being created by GLFW.
GLFWwindow* window;
// An array of vertices stored in an std::vector for our object.
std::vector<VertexFormat> vertices;
// References to our two GameObjects and the one Model we'll be using.
GameObject* obj1;
GameObject* obj2;
Model* circle;


///
//Tests for a collision between two bounding circles
//Parameters:
//	a: The first bounding circle to test with
//	aPosition: The position of the first bounding circle
//	b: The second bounding circle to test with
//	bPosition: The position of the second bounding circle
//Returns:
//	True if the circles are intersecting OR touching, else false
bool TestCircleCollision(BoundingCircle a, const glm::vec3 &aPosition, BoundingCircle b, const glm::vec3 &bPosition)
{
	//If the distance between the centers of the two circles is less than the sum of the circles radii they must be intersecting.
	glm::vec3 between = (b.centroid + bPosition) - (a.centroid + aPosition);	//Vector from circle a to circle b
	if (glm::length(between) <= a.radius + b.radius)
	{
		return true;
	}
	return false;
}

//Checks for collisions and updates the matrices which are being sent to the shaders
void update()
{
#pragma region Boundaries
	// This section just checks to make sure the object stays within a certain boundary. This is not really collision detection.
	glm::vec3 tempPos = obj2->GetPosition();
	
	if (fabsf(tempPos.x) > 1.35f)
	{
		glm::vec3 tempVel = obj2->GetVelocity();

		// "Bounce" the velocity along the axis that was over-extended.
		obj2->SetVelocity(glm::vec3(-1.0f * tempVel.x, tempVel.y, tempVel.z));
	}
	if (fabsf(tempPos.y) > 0.8f)
	{
		glm::vec3 tempVel = obj2->GetVelocity();
		obj2->SetVelocity(glm::vec3(tempVel.x, -1.0f * tempVel.y, tempVel.z));
	}
	if (fabsf(tempPos.z) > 1.0f)
	{
		glm::vec3 tempVel = obj2->GetVelocity();
		obj2->SetVelocity(glm::vec3(tempVel.x, tempVel.y, -1.0f * tempVel.z));
	}
#pragma endregion Boundaries section just bounces the object so it does not fly off the side of the screen infinitely.


	// Pass in our two objects to the SAT test, if it returns true then they are colliding.
	if (TestCircleCollision(obj2->GetBoundingCircle(), obj2->GetPosition(), obj1->GetBoundingCircle(), obj1->GetPosition()))
	{
		//Change the color of the objects to red if they are colliding
		hue[0][0] = 1.0f;
		hue[1][1] = 0.0f;
	}
	else
	{
		//Change the color of the objects to green if they are not colliding
		hue[0][0] = 0.0f;
		hue[1][1] = 1.0f;
	}

	// Update your MVP matrices based on the objects' transforms.
	MVP = PV * *obj1->GetTransform();
	MVP2 = PV * *obj2->GetTransform();
}


// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set the uniform hue matrix in fragment shader to the global hue matrix for both objects
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Set the uniform matrix in our shader to our MVP matrix for the first object.
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
	circle->Draw();

	// Set the uniform matrix in our shader to our MVP matrix for the second object.
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP2));
	circle->Draw();

}

//Reads in shader source
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

	//Set the global hue matrix to green
	hue[0][0] = 0;	//Red index
	hue[2][2] = 0;	//Blue index

	//The number of subdivisions of the circle
	const int subdivisions = 48;

	// An element array, which determines which of the vertices to display in what order. This is sometimes known as an index array.
	GLuint elements[subdivisions * 3];

	int vertex = 0;
	//Fill the element array
	for (int i = 0; i < subdivisions * 3; i+= 3)
	{
		elements[i] = vertex;									//Index on outer edge of circle for current iteration
		vertex++;
		elements[i + 1] = vertex < subdivisions ? vertex : 0;	//Index on outer edge of circle for next iteration
		elements[i + 2] = (subdivisions);						//Center index
	}


	//Generate the vertices for a circle
	float stepSize = (2.0f*3.14159f) / static_cast<float>(subdivisions);
	for (int i = 0; i < subdivisions; i++)
	{
		float currentAngle = stepSize * static_cast<float>(i);
		vertices.push_back(VertexFormat(glm::vec3(cosf(currentAngle), sinf(currentAngle), 0.0f), glm::vec4(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))));
		
	}
	vertices.push_back(VertexFormat(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));


	// Create our square model from the calculated data.
	circle = new Model(vertices.size(), vertices.data(), subdivisions * 3, elements);
	
	// Create two GameObjects based off of the square model (note that they are both holding pointers to the square, not actual copies of the square vertex data).
	obj1 = new GameObject(circle);
	obj2 = new GameObject(circle);

	// Set beginning properties of GameObjects.
	obj1->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	obj2->SetPosition(glm::vec3(-0.7f, 0.0f, 0.0f));
	obj1->SetScale(glm::vec3(0.25f, 0.25f, 0.25f));
	obj2->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));

	//Create shader program
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

	view = glm::lookAt(	glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	proj = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
	PV = proj * view;
	MVP = PV * *obj1->GetTransform();
	MVP2 = PV * *obj2->GetTransform();

	// Calculate the bounding circle for your object.
	obj1->CalculateBoundingCircle();
	obj2->CalculateBoundingCircle();

	//SEt options
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
}

// Fires every time a key on the keyboard is pressed.
void OnKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
		case GLFW_KEY_UP:
			obj1->AddPosition(glm::vec3(0.0f, 0.05f, 0.0f));
			break;
		case GLFW_KEY_RIGHT:
			obj1->AddPosition(glm::vec3(0.05f, 0.0f, 0.0f));
			break;
		case GLFW_KEY_LEFT:
			obj1->AddPosition(glm::vec3(-0.05f, 0.0f, 0.0f));
			break;
		case GLFW_KEY_DOWN:
			obj1->AddPosition(glm::vec3(0.0f, -0.05f, 0.0f));
			break;
		case GLFW_KEY_W:
			obj2->AddPosition(glm::vec3(0.0f, 0.05f, 0.0f));
			break;
		case GLFW_KEY_D:
			obj2->AddPosition(glm::vec3(0.05f, 0.0f, 0.0f));
			break;
		case GLFW_KEY_A:
			obj2->AddPosition(glm::vec3(-0.05f, 0.0f, 0.0f));
			break;
		case GLFW_KEY_S:
			obj2->AddPosition(glm::vec3(0.0f, -0.05f, 0.0f));
			break;
		default:
			break;
		}
	}
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 600, "Circle Collision", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Sets the callback functions that will trigger every time keyboard input is received.
	glfwSetKeyCallback(window, OnKeyPress);

	// Initializes most things needed before the main loop
	init();

	//Print controls
	std::cout << "Controls:\nMove circles with WASD and the Arrow Keys.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{

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

	delete(obj1);
	delete(obj2);
	delete(circle);

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}