/*
Title: TextShaders
File Name: Main.cpp
Copyright © 2015
Original authors: Brockton Roth
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
This project contains a basic approach to understanding shaders. It consists 
of shader code, written into std::string variables. The shaders are simple, 
the vertex shader having hard-coded vertices and the fragment shader 
having hard-coded color. This simply renders a green triangle on a red 
background.
*/

#include "glew\glew.h"
#include "glfw\glfw3.h"
#include <iostream>

// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// This function runs every frame
void renderScene(void)
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to red
	glClearColor(1.0, 0.0, 0.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	// Draw 3 vertices from the buffer as GL_TRIANGLES
	// There are several different drawing modes, GL_TRIANGLES takes every 3 vertices and makes them a triangle.
	// For reference, GL_TRIANGLE_STRIP would take each additional vertex after the first 3 and consider that a 
	// triangle with the previous 2 vertices (so you could make 2 triangles with 4 vertices)
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Triangle", nullptr, nullptr);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	glfwSwapInterval(1);

	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Plain text shader code
	std::string vertShader = "#version 400 core \n" // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code so we put the \n
		"void main(void){ "
		"const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0), " // Hardcoded vertices for simplicity.
		"vec4(-0.25, -0.25, 0.5, 1.0), "
		"vec4(0.0, 0.25, 0.5, 1.0)); "
		"gl_Position = vertices[gl_VertexID]; " // Pass out the vertices to the gl_Position
		"}";
	std::string fragShader = "#version 400 core \n"
		"out vec4 color; " // Establishes the variable we will pass out of this hsader.
		"void main(void){ "
		"color = vec4(0.0, 1.0, 0.0, 1.0); " // Hardcoded color for simplicity.
		"}";

	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const char *vert_shader_code_ptr = vertShader.c_str(); // We establish a pointer to our vertex shader code string
	const int vert_shader_code_size = vertShader.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(vertex_shader, 1, &vert_shader_code_ptr, &vert_shader_code_size);
	glCompileShader(vertex_shader); // This just compiles the shader, given the source code.

	// We repeat the above process, this time for the fragment shader.
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *frag_shader_code_ptr = fragShader.c_str();
	const int frag_shader_code_size = fragShader.size();

	glShaderSource(fragment_shader, 1, &frag_shader_code_ptr, &frag_shader_code_size);
	glCompileShader(fragment_shader);

	// Notice how this looks exactly the same, just with different variables? Maybe there's something we can do to make this more efficient?

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
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

	return 0;
}