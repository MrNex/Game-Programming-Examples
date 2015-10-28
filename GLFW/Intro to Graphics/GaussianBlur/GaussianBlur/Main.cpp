/*
Title: Gaussian Blur
File Name: Main.cpp
Copyright © 2015
Original authors: Srinivasan T
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
This demo demonstrates the implementation of gaussian blur filter using fragment shader.
To implement a blur effect,we avarage the color values of a pixel with the values of the 
nearby pixels. Gaussian blur just implements this "averaging between pixels" using gaussian
weights, which are computed on the CPU side application and sent to the GPU as uniforms.

To implement gaussian blur, we need to implement 2 dimensional gaussian equation. This would require 
a NxN sampling calls of a texture. where N is the number of pixels we average for one pixel in the final image.

We reduce this problem to an order of N, by adding another render call. in The first render
call we average the values of the pixels horizontally. and in the second render call, we do this vertically.
This is equivalent to implementing a 2D gaussian function. 

In this program, we first render the scene normally onto a texture using a frame buffer, and apply horizontal
blur on it and render this onto another texture. On the third render call, we apply vertical blur and then we
render this texture on a plane which covers the entire scene/window. 

Use "SPACEBAR" to toggle blur on and off.

References:
OpenGL 4 Shading language cookbook, Second Edition
*/

#include "glew\glew.h"
#include "glfw\glfw3.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include "soil\SOIL.h"
#include <iostream>
#include <fstream>
#include <vector>

glm::mat4 trans;
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
GLuint uniTrans;
GLuint tex;
GLuint vbo;
GLuint fboHandle1;
GLuint fboHandle2;
GLuint depthBuf1;
GLuint depthBuf2;
GLuint renderTex1;
GLuint renderTex2;
GLuint SubroutinePass1;
GLuint SubroutinePass2;
GLuint SubroutinePass3;

bool blur = false;
float gaussianWeight[5];

// We create a VertexFormat struct, which defines how the data passed into the shader code wil be formatted
struct VertexFormat
{
	glm::vec2 texCoord; // A vector2 for texture coordinates, has 2 floats: u and v
	// These coordinates are 0,0 for the top left and 1,1 for the bottom right

	glm::vec3 position;	// A vector3 for position has 3 floats: x, y, and z coordinates

	// Constructor
	VertexFormat(const glm::vec2 &texCo, const glm::vec3 &pos)
	{
		texCoord = texCo;
		position = pos;
	}
};

void gaussianWeights(float(&x)[5])
{
	//the gaussian fucntion can be given as follows:
	//G(x) = e^[(-x*x)/(2*sigma)] / sqrt(2*sigma * sigma * pi)
	
	float sigma = 4.0f;
	float e = 2.71828f;		// mathematical constant (euler's number)
	float pi = 3.14f;
	float exp;
	float constant = 1.0f / sqrt((2.0f * pi * sigma * sigma));
	
	float sum = 0;
	x[0] = constant;
	for (int i = 1; i < 5; i++)
	{
		exp = (-i*i) / (2 * sigma *sigma);
		x[i] = constant * pow(e,exp);
		sum += x[i] * 2.0f;
	}

	for (int i = 0; i < 5; i++)
	{
		x[i] /= sum;
	}
	
}

// This runs once a frame, before renderScene
void update()
{
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to black
	glClearColor(0.3, 0.3, 0.3, 1.0);

	std::string uniformname;
	GLuint weightID;
	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);
	if (blur)
	{
		//set the weights in the sahder
		weightID = glGetUniformLocation(program, "weights[0]");
		glUniform1fv(weightID,5, gaussianWeight);
		
		//Bind the first frame buffer and render to texture1
		glBindFramebuffer(GL_FRAMEBUFFER, fboHandle1);
		glViewport(0, 0, 800, 800);
		//set the first pass : render the proper scene ( no blur)
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &SubroutinePass1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//bind the second frame buffer and render to texture2 
		glBindFramebuffer(GL_FRAMEBUFFER, fboHandle2);
		glViewport(0, 0, 800, 800);
		//set the second pass : render with the horizontal blur
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &SubroutinePass2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderTex1);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//Bind the defauld frame buffer and render to the main screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 800, 800);
		//set the third pass : render with the vertical blur
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &SubroutinePass3);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderTex2);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	else
	{
		//If no blur is selected, then bind the default frame buffer 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 800, 800);
		//set the first pass : render the proper scene ( no blur)
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &SubroutinePass1);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
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
	// Initializes the glew library
	glewInit();

	// Create an std::vector and put our vertices into it. These are just hardcoded values here defined once.
	std::vector<VertexFormat> vertices;

	vertices.push_back(VertexFormat(glm::vec2(1.0, 0.0), glm::vec3(1.0f, -1.0f, 0.0)));	// bottom right
	vertices.push_back(VertexFormat(glm::vec2(0.0, 0.0), glm::vec3(-1.0, -1.0f, 0.0))); // bottom left
	vertices.push_back(VertexFormat(glm::vec2(1.0, 1.0), glm::vec3(1.0, 1.0f, 0.0)));	// top right

	vertices.push_back(VertexFormat(glm::vec2(1.0, 1.0), glm::vec3(1.0, 1.0, 0.0)));	// top right
	vertices.push_back(VertexFormat(glm::vec2(0.0, 0.0), glm::vec3(-1.0, -1.0, 0.0)));	// bottom left
	vertices.push_back(VertexFormat(glm::vec2(0.0, 1.0), glm::vec3(-1.0, 1.0, 0.0)));	// top left
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * 6, &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)8);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation
	glUseProgram(program);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	int width, height; // Variables to store the width and height of the loaded texture. These will be empty until SOIL_load_image is called.

	unsigned char* image = SOIL_load_image("texture.jpg", &width, &height, 0, SOIL_LOAD_RGBA);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	// Now that we've created our texture on GL_TEXTURE_2D, we don't need the image anymore. So we free that memory using SOIL.
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	uniTrans = glGetUniformLocation(program, "trans");
	SubroutinePass1 = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "pass1");
	SubroutinePass2 = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "pass2");
	SubroutinePass3 = glGetSubroutineIndex(program, GL_FRAGMENT_SHADER, "pass3");

//================================================================================================================
//======================== CREATE AND BIND FIRST FBO: this will render to rendertex1 =============================
//================================================================================================================
	glGenTextures(1, &renderTex1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderTex1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glGenFramebuffers(1, &fboHandle1);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle1);

	glGenRenderbuffers(1, &depthBuf1);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf1);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,800, 800);

	// Bind the depth buffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf1);
	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, renderTex1, 0);
	
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(1,DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Frame buffer not created. \n";
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

//================================================================================================================
//======================== CREATE AND BIND SECOND FBO: this will render to rendertex2 ============================
//================================================================================================================
	glGenTextures(1, &renderTex2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderTex2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &fboHandle2);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle2);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, renderTex2, 0);

	glGenRenderbuffers(1, &depthBuf2);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf2);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 800);

	// Bind the depth buffer to the FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf2);

	//DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Frame buffer not created.\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//================================================================================================================

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT, GL_FILL);

	//Compute the Gaussian weights
	gaussianWeights(gaussianWeight);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		blur = !blur;
	}
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	GLFWwindow* window = glfwCreateWindow(800, 800, "Textures", nullptr, nullptr);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	glfwSwapInterval(1);

	// Initializes most things needed before the main loop
	init();

	glfwSetKeyCallback(window, key_callback);

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

	// Free up buffer memory.
	glDeleteBuffers(1, &vbo);

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}