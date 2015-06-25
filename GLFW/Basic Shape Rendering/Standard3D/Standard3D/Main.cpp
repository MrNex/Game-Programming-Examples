/*
Title: Standard3D
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
Builds upon the "Standard2D" project and renders a 3D cube, spinning about the
X-Y axis. This uses a uniform transformation matrix. This project is more about
getting started and understanding what a basic OpenGL program looks like.
*/

#include "glew\glew.h"
#include "glfw\glfw3.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "glm\gtc\quaternion.hpp"
#include <iostream>
#include <fstream>
#include <vector>

// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform transformation matrix in your vertex shader
GLuint uniTrans;

// This is a 4x4 transformation matrix, which you will locally modify before passing into the vertex shader via uniTrans
glm::mat4 trans;

// This is your reference to your Vertex Buffer Object
unsigned int vbo;

// We create a VertexFormat struct, which defines how the data passed into the shader code wil be formatted
struct VertexFormat
{
	glm::vec4 color;	// A vector4 for color has 4 floats: red, green, blue, and alpha
	glm::vec3 position;	// A vector3 for position has 3 float: x, y, and z coordinates

	// Constructor
	VertexFormat(const glm::vec3 &pos, const glm::vec4 &iColor)
	{
		position = pos;
		color = iColor;
	}
};

// This runs once a frame, before renderScene
void update()
{
	// Translate the transformation matrix to the center of the cube, which is located at (0, 0, -0.5)
	trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, -0.5f));

	// Rotate the transformation matrix about the X-Y axis by 1 degree per function call.
	// glm::rotate returns a 4x4 matrix of the resulting transform, given an input matrix, an angle to rotate by (in radians), and a vector3 determining which axis to rotate about
	// glm::radians converts the value in paranthesis from degrees to radians
	// The axis parameter should be treated like a vector, so it doesn't just have to have a 1.0f in a single axis, but instead you're defining a vector that we are rotating around
	trans = glm::rotate(trans, glm::radians(1.0f), glm::vec3(1.0f, 1.0f, 0.0f));

	// Translate the transformation matrix back to the origin (0, 0, 0)
	// We do this because we don't want the transformation matrix to actually translate our coordinates, we only want the rotation.
	// However, when rotating, we need the matrix at the center of the object we're rotating, else we'll be rotating about the origin.
	trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, 0.5f));

	// This takes the value of our transformation matrix and sets our uniform transformation matrix within our shader to this value.
	// Parameters are: Location within the shader, size (in case we're passing in multiple matrices via a single pointer), whether or not to transpose the matrix, and a pointer 
	// to the matrix value we're passing in.
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	// Draw 3 vertices from the buffer as GL_TRIANGLES
	// There are several different drawing modes, GL_TRIANGLES takes every 3 vertices and makes them a triangle.
	// For reference, GL_TRIANGLE_STRIP would take each additional vertex after the first 3 and consider that a 
	// triangle with the previous 2 vertices (so you could make 2 triangles with 4 vertices)
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

// This method reads the text from a file.
// Realistically, we wouldn't want plain text shaders hardcoded in, we'd rather read them in from a separate file so that the shader code is separated.
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning, 
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Create an std::vector and put our vertices into it. These are just hardcoded values here defined once.
	std::vector<VertexFormat> vertices;//our vertex positions
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.25),
		glm::vec4(1.0, 0.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.25),
		glm::vec4(1.0, 0.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.25), // 3
		glm::vec4(1.0, 0.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.25),
		glm::vec4(1.0, 0.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.25),
		glm::vec4(1.0, 0.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.25), // 6 end front face
		glm::vec4(1.0, 0.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.25),
		glm::vec4(1.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.25),
		glm::vec4(1.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.75), // 9
		glm::vec4(1.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.25),
		glm::vec4(1.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.75),
		glm::vec4(1.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.75), // 12 end right face
		glm::vec4(1.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.75),
		glm::vec4(1.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.75),
		glm::vec4(1.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.75), // 15
		glm::vec4(1.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.75),
		glm::vec4(1.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.75),
		glm::vec4(1.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.75), // 18 end back face
		glm::vec4(1.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.75),
		glm::vec4(0.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.75),
		glm::vec4(0.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.25), // 21
		glm::vec4(0.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.75),
		glm::vec4(0.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.25),
		glm::vec4(0.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.25), // 24 end left face
		glm::vec4(0.0, 1.0, 0.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.25),
		glm::vec4(0.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.75),
		glm::vec4(0.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.75), // 27
		glm::vec4(0.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, 0.25, -0.25),
		glm::vec4(0.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.75),
		glm::vec4(0.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, 0.25, -0.25), // 30 end top face
		glm::vec4(0.0, 0.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.25),
		glm::vec4(0.0, 1.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.25),
		glm::vec4(0.0, 1.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.75), // 33
		glm::vec4(0.0, 1.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.25),
		glm::vec4(0.0, 1.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(0.25, -0.25, -0.75),
		glm::vec4(0.0, 1.0, 1.0, 1.0)));
	vertices.push_back(VertexFormat(glm::vec3(-0.25, -0.25, -0.75), // 36
		glm::vec4(0.0, 1.0, 1.0, 1.0)));

	// Now that's a lot of vertices! Maybe there's a way to reduce the number of vertices we're creating...
	// Notice how several of our vertices are the exact same? After all, the cube only really has 8 vertices.


	// This generates buffer object names
	// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
	// (In this example, there's only one buffer object.)
	glGenBuffers(1, &vbo);

	// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
	// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
	// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
	// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
	// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Creates and initializes a buffer object's data.
	// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
	// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
	// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
	// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
	// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
	// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * 36, &vertices[0], GL_STATIC_DRAW);

	// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
	// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
	// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
	glEnableVertexAttribArray(0);

	// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
	// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
	// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
	// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
	// then the pointer parameter is treated as a byte offset into the buffer object's data.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
	// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
	// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
	// For more info on padding, Google it.

	// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	// This gets us a reference to the uniform variable in the vertex shader, which is called "trans".
	// We're using this variable as a 4x4 transformation matrix
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniTrans = glGetUniformLocation(program, "trans");

	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CW);

	// This is also not necessary, but more efficient and is generally good practice. By default, OpenGL will render both sides of a triangle that you draw. By enabling GL_CULL_FACE, 
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-axis, you wouldn't see the other side of the triangle as it rotated.
	glEnable(GL_CULL_FACE);

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT, GL_FILL);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Cube!", nullptr, nullptr);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	glfwSwapInterval(1);

	// Initializes most things needed before the main loop
	init();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to the update function; should always be before rendering.
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

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}