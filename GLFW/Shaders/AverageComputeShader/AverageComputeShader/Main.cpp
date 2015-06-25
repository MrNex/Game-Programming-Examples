/*
Title: AverageComputeShader
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
Improves on the BasicComputeShader, taking advantage of the parallelization 
and drawing a spinning circle of points (dots, essentially). Each color is 
based off of the vertex ID (retrieved via gl_GlobalInvocationID) and each 
position is based off of a sin, cos, and radius to create the circle (the 
angle is also calculated using the gl_GlobalInvocationID). This allows for 
fast computation. The radius is set via a uniform variable, and the code 
has it growing to a certain point, then shrinking again, and repeating.

WARNING: Requires OpenGL version 4.3 or later for the compute shader to compile.
*/

#include "glew\glew.h"
#include "glfw\glfw3.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>

// These are your references to your shader programs.
// These will be assigned with glCreateProgram().
// These programs will run on your GPU.
GLuint program;
GLuint compute_program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;
GLuint compute_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;

// These are 4x4 transformation matrices, which you will locally modify before passing into the vertex shader via uniMVP
glm::mat4 trans;
glm::mat4 proj;
glm::mat4 view;
glm::mat4 MVP;

// A reference to your Vertex Buffer Object, Vertex Array Object, Shader Storage Buffer Object, and the uniform float for radius in your compute shader.
GLuint vbo;
GLuint vao;
GLuint ssbo;
GLuint iLocRadius;

// A frame number tracker, which isn't actually tracking the number of frames but instead the radius of the circle.
int frameNum = 0;

// A boolean to determine whether or not to reverse the direction of the circle (from growing to shrinking)
bool reverse = false;

// We create a VertexFormat struct, which defines how the data passed into the shader code wil be formatted
struct VertexFormat
{
	glm::vec4 color;	// A vector4 for color has 4 floats: red, green, blue, and alpha
	glm::vec3 position;	// A vector3 for position has 3 float: x, y, and z coordinates

	// A default constructor, which we will use with vector.resize()
	VertexFormat()
	{
		color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		position = glm::vec3(0.0f, 0.0f, 0.0f);
	}

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
	// If we're reversing the circle that means we are shrinking it.
	// This part of the code should be straightforward: frameNum will increase until 250, then decrease down to 1, then repeat
	if (reverse)
	{
		frameNum--; 

		if (frameNum < 1)
		{
			reverse = false;
		}
	}
	else
	{
		frameNum++;
		if (frameNum > 250)
		{
			reverse = true;
		}
	}

	// Rotate the transformation matrix about the Z axis by 1 degree per function call.
	// glm::rotate returns a 4x4 matrix of the resulting transform, given an input matrix, an angle to rotate by (in radians), and a vector3 determining which axis to rotate about
	// glm::radians converts the value in paranthesis from degrees to radians
	// The axis parameter should be treated like a vector, so it doesn't just have to have a 1.0f in a single axis, but instead you're defining a vector that we are rotating around
	trans = glm::rotate(trans, glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // All this is doing is rotating the circle.

	// This takes the value of our transformation, view, and projection matrices and multiplies them together to create the MVP matrix.
	// Then it sets our uniform MVP matrix within our shader to this value.
	// Parameters are: Location within the shader, size (in case we're passing in multiple matrices via a single pointer), whether or not to transpose the matrix, and a pointer 
	// to the matrix value we're passing in.
	MVP = proj * view * trans;

	// Tells the code to use the compute shader program.
	glUseProgram(compute_program);

	// Sets the uniform radius to frameNum / 500.
	glUniform1f(iLocRadius, ((float)frameNum / 500.0f));
	
	// Submit job for the compute shader execution.
	// GROUP_SIZE_HEIGHT = GROUP_SIZE_WIDTH = 8
	// NUM_VERTS_H = NUM_VERTS_V = 16
	// As the result the function is called with the following parameters:
	glDispatchCompute(2, 2, 1); // 2 x 2 = 4, and within the compute shader we've locally defined sizes x and y to be 8, so 8 x 8 = 64 and 64 x 4 = 256
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Ensures that we wait for the compute shader to finish before proceeding, since it runs in parallel.
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	// Tell OpenGL to use the regular shader program you've created.
	glUseProgram(program);

	// Sets the MVP matrix uniform in the shader.
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));

	// Bind the vao
	glBindVertexArray(vao);

	// Draw 256 vertices from the buffer as GL_POINTS which will just draw each vertex individually as a single point.
	// There are several different drawing modes, GL_TRIANGLES takes every 3 vertices and makes them a triangle.
	// For reference, GL_TRIANGLE_STRIP would take each additional vertex after the first 3 and consider that a 
	// triangle with the previous 2 vertices (so you could make 2 triangles with 4 vertices, 3 with 5, etc.)
	// The second parameter is the start index of the array, and the third parameter is the number of vertices to draw.
	glDrawArrays(GL_POINTS, 0, 256);
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

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may 
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	glewExperimental = GL_TRUE;
	// Initializes the glew library
	glewInit();

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

	// Do all the same for our compute shader and program
	std::string compShader = readShader("ComputeShader.glsl");
	compute_shader = createShader(compShader, GL_COMPUTE_SHADER);
	compute_program = glCreateProgram();
	glAttachShader(compute_program, compute_shader);
	glLinkProgram(compute_program);

	// Tell our code to use the compute program
	glUseProgram(compute_program);

	// Get the uniform location of the radius variable
	iLocRadius = glGetUniformLocation(compute_program, "radius");

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Create an std::vector and put our vertices into it. This is just going to be an empty array.
	std::vector<VertexFormat> vertices;//our vertex positions
	
	vertices.resize(256); // Resize the vector to 256, which creates 256 VertexFormat objects (using the default constructor) in an array.

	// Generate your vertex array object name.
	glGenVertexArrays(1, &vao);
	
	// Bind the vertex array object
	glBindVertexArray(vao);

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * 256, &vertices[0], GL_STATIC_DRAW);
	// GL_ARRAY_BUFFER is for vertices in an array, all drawing commands of glDrawArrays will use vertices from that buffer.

	// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
	// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
	// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
	glEnableVertexAttribArray(0);

	// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
	// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
	// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
	// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
	// then the pointer parameter is treated as a byte offset into the buffer object's data.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
	// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
	// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
	// For more info on padding, Google it.

	// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	
	// Now we set our Shader Storage Buffer Object reference equal to our Vertex Buffer Object reference. This is crucial for our Compute Shader to be able to modify the values in 
	// that buffer.
	ssbo = vbo;

	// Binds a buffer object (ssbo) to a binding point at the given index (0 in this case) of the array of targets (GL_SHADER_STORAGE_BUFFER)
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	// This gets us a reference to the uniform variable in the vertex shader, which is called "MVP".
	// We're using this variable as a 4x4 transformation matrix
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniMVP = glGetUniformLocation(program, "MVP");

	// Creates the view matrix using glm::lookAt.
	// First parameter is camera position, second parameter is point to be centered on-screen, and the third paramter is the up axis.
	view = glm::lookAt(	glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Creates a projection matrix using glm::perspective.
	// First parameter is the vertical FoV (Field of View), second paramter is the aspect ratio, 3rd parameter is the near clipping plane, 4th parameter is the far clipping plane.
	proj = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CW);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	GLFWwindow* window = glfwCreateWindow(800, 600, "Average Compute Shader", nullptr, nullptr);

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
	glDeleteShader(compute_shader);
	glDeleteProgram(compute_program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}