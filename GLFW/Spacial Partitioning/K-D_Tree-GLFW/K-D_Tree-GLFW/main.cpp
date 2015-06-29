/*
K-D Tree
(c) 2015
original authors: Benjamin Robbins
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

*	This example uses a K-D Tree to sort an array of interactive shapes by their positions. A K-D tree uses a revolving set of dividing'
*	values. In the case of this example, the dividing value alternates between x-position and y-position. For the first division, the
*	tree sorts the entire array of shapes by their x-positions and uses that to find the median value, dividing the field in half. For
*	that node's children, the field is sorted and divided by their y-Positions.
*
*	While a K-D tree seems like it's less useful than an Oct-Tree because it's so much slower while not accomplishing much more, it still has
*	significance, but in a different area. K-D trees are not useful for dynamically updating, but creating effective binary search trees based
*	on multiple variables. If you want to find vertices in 3-D space near a particular location, an Oct-Tree requares multiple collision detection
*	tests, while a K-D tree simply allows the program to compare a single value for each level, dividing the field in half each time. The
*	most common application that I know of is when ray tracing against an array of triangles (eg you're shooting something with a gun and
*	you want to know precisely where it hit), you have a position from your ray. If the verts of the triangles are sorted into a K-D Tree,
*	then it's an easy matter of finding the triangles near your position for a finer collision detection method to be used.
*
*	1) RenderManager
*	- This class maintains data for everything that needs to be drawn in two display lists, one for non-interactive shapes and
*	one for interactive shapes. It handels the updating and drawing of these shapes.
*
*	2) InputManager
*	- This class handles all user input from the mouse and keyboard.
*
*	3) KDTreeManager
*	- This class maintains an array of references to InteractiveShapes and sorts them into the K-DTree. Furthermore it maintains references to
*	and updates the transforms of the green division lines to show the borders of the nodes.
*
*	RenderShape
*	- Holds the instance data for a shape that can be rendered to the screen. This includes a transform, a vao, a shader, the drawing
*	mode (eg triangles, lines), it's active state, and its color
*
*	InteractiveShape
*	- Inherits from RenderShape, possessing all the same properties. Additionally, it has a collider and can use it to check collisions against
*	world boundries, other colliders, and the cursor.
*
*	Init_Shader
*	- Contains static functions for loading, compiling and linking shaders.
*
*/

#include <glew\glew.h>
#include <glfw\glfw3.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\random.hpp>
#include <iostream>
#include <ctime>

#include "RenderShape.h"
#include "Init_Shader.h"
#include "RenderManager.h"
#include "InputManager.h"
#include "KDTreeManager.h"

GLFWwindow* window;

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;
Shader shader;

GLuint vbo;
GLuint vao0;
GLuint vao1;
GLuint ebo0;
GLuint ebo1;
GLint posAttrib;
GLint uTransform;
GLint uColor;

GLfloat vertices[] = {
	-1.0f, +1.0f,
	+1.0f, +1.0f,
	-1.0f, -1.0f,
	+1.0f, -1.0f
};

GLint elements[] = {
	0, 1, 2,
	1, 3, 2
};

GLint outlineElements[] = {
	1, 2 
};

void initShaders()
{
	char* shaders[] = { "fshader.glsl", "vshader.glsl" };
	GLenum types[] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
	int numShaders = 2;
	
	shaderProgram = initShaders(shaders, types, numShaders);

	uTransform = glGetUniformLocation(shaderProgram, "transform");
	uColor = glGetUniformLocation(shaderProgram, "color");

	shader = Shader();
	shader.shaderPointer = shaderProgram;
	shader.uTransform = uTransform;
	shader.uColor = uColor;
}

void initGeometry()
{
	// Store the data for the triangles in a buffer that the gpu can use to draw
	// Create the VAO for the squares
	glGenVertexArrays(1, &vao0);
	glBindVertexArray(vao0);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo0);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	// Bind buffer data to shader values
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Create the VAO for the green lines
	glGenVertexArrays(1, &vao1);
	glBindVertexArray(vao1);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(outlineElements), outlineElements, GL_STATIC_DRAW);

	// Bind buffer data to shader values
	posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void init()
{
	if (!glfwInit()) exit(EXIT_FAILURE);

	//Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "K-D_Tree-GLFW", NULL, NULL); // Windowed

	//Activate window
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	initShaders();

	initGeometry();

	glfwSetTime(0.0);

	time_t timer;
	time(&timer);
	srand((unsigned int)timer);

	RenderManager::GenerateShapes(shader, vao0, 500, GL_TRIANGLES, 6);

	InputManager::Init(window);

	KDTreeManager::InitKDTree(5, RenderShape(vao1, 2, GL_LINE_STRIP, shader, glm::vec4(0.0f, 1.0f, 0.3f, 1.0f)));
	
	unsigned int shapesSize = RenderManager::interactiveShapes().size();
	for (unsigned int i = 0; i < shapesSize; ++i)
	{
		KDTreeManager::AddShape(RenderManager::interactiveShapes()[i]);
	}
	KDTreeManager::UpdateKDtree();
}

void step()
{
	// Clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	InputManager::Update();

	// Get delta time since the last frame
	float dt = glfwGetTime();
	glfwSetTime(0.0);

	// Change the max depth of the tree if the up or down arrow keys are pressed
	int dDepth = 0;
	dDepth += (InputManager::upKey(true) && !InputManager::upKey());
	dDepth -= (InputManager::downKey(true) && !InputManager::downKey());
	KDTreeManager::SetMaxDepth(KDTreeManager::maxDepth() + dDepth);

	RenderManager::Update(dt);
	if (RenderManager::shapeMoved())
		KDTreeManager::UpdateKDtree();

	RenderManager::Draw();

	// Swap buffers
	glfwSwapBuffers(window);
}

void cleanUp()
{
	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo0);
	glDeleteBuffers(1, &ebo1);
	glDeleteBuffers(1, &vao0);
	glDeleteBuffers(1, &vao1);

	RenderManager::DumpData();

	KDTreeManager::DumpData();

	glfwTerminate();
}

int main()
{
	init();

	while (!glfwWindowShouldClose(window))
	{
		step();
		glfwPollEvents();
	}

	cleanUp();

	return 0;
}
