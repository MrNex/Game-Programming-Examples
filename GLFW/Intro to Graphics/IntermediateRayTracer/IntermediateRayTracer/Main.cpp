/*
Title: Intermediate Ray Tracer
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

References:
https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-(Part-I)

Description:
This program serves to demonstrate the concept of ray tracing. This 
builds off of a previous Basic Ray Tracer, adding in the concept of 
light and shadow. There are four point lights, and this uses 
diffuse and specular lighting.
*/

#include "glew\glew.h"
#include "glfw\glfw3.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// These are your uniform variables.
GLuint eye;		// Specifies camera location
// The rays are the four corner rays of the camera view. See: https://camo.githubusercontent.com/21a84a8b21d6a4bc98b9992e8eaeb7d7acb1185d/687474703a2f2f63646e2e6c776a676c2e6f72672f7475746f7269616c732f3134313230385f676c736c5f636f6d707574652f726179696e746572706f6c6174696f6e2e706e67
GLuint ray00;
GLuint ray01;
GLuint ray10;
GLuint ray11;

// A variable used to describe the position of the camera.
glm::vec3 cameraPos;

// A reference to your Vertex Buffer Object, Vertex Array Object, Shader Storage Buffer Object, and the uniform float for radius in your compute shader.
GLuint vbo;
GLuint vao;

// A reference to our window.
GLFWwindow* window;

// Variables you will need to calculate FPS.
int frame;
double time;
double timebase;
int fps;

// This function takes in variables that define the perspective view of the camera, then outputs the four corner rays of the camera's view.
// It takes in a vec3 eye, which is the position of the camera.
// It also takes vec3 center, the position the camera's view is centered on.
// Then it will takes a vec3 up which is a vector that defines the upward direction. (So if you point it down, the camera view will be upside down.)
// Then it takes a float defining the verticle field of view angle. It also takes a float defining the ratio of the screen (in this case, 800/600 pixels).
// The last four parameters are actually just variables for this function to output data into. They should be pointers to pre-defined vec4 variables.
// For a visual reference, see this image: https://camo.githubusercontent.com/21a84a8b21d6a4bc98b9992e8eaeb7d7acb1185d/687474703a2f2f63646e2e6c776a676c2e6f72672f7475746f7269616c732f3134313230385f676c736c5f636f6d707574652f726179696e746572706f6c6174696f6e2e706e67
void calcCameraRays(glm::vec3 eye, glm::vec3 center, glm::vec3 up, float fov, float ratio, glm::vec4* r00, glm::vec4* r01, glm::vec4* r10, glm::vec4* r11)
{
	// Grab a ray from the camera position toward where the camera is to be centered on.
	glm::vec3 centerRay = center - eye;

	// w: Vector from center toward eye
	// u: Vector pointing directly right relative to the camera.
	// v: Vector pointing directly upward relative to the camera.

	// Create a w vector which is the opposite of that ray.
	glm::vec3 w = -centerRay;

	// Get the rightward (relative to camera) pointing vector by crossing up with w.
	glm::vec3 u = glm::cross(up, w);

	// Get the upward (relative to camera) pointing vector by crossing the rightward vector with your w vector.
	glm::vec3 v = glm::cross(w, u);
	
	// Each ray starts based off of the center ray. Then we will rotate them over the 
	*r00 = glm::vec4(centerRay, 1.0f);
	*r01 = *r00;
	*r10 = *r00;
	*r11 = *r00;

	// We create these two helper variables, as when we rotate the ray about it's relative Y axis (v), we will then need to rotate it about it's relative X axis (u).
	// This means that u has to be rotated by v too, otherwise the rotation will not be accurate. When the ray is rotated about v, so then are it's relative axes.
	glm::vec4 uRotateLeft = glm::vec4(u, 1.0f) * glm::rotate(glm::mat4(), glm::radians(-fov * ratio / 2.0f), v);
	glm::vec4 uRotateRight = glm::vec4(u, 1.0f) * glm::rotate(glm::mat4(), glm::radians(fov * ratio / 2.0f), v);

	// Now we simply take the ray and rotate it in each direction to create our four corner rays.
	*r00 = *r00 * glm::rotate(glm::mat4(), glm::radians(-fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(fov / 2.0f), glm::vec3(uRotateLeft.x, uRotateLeft.y, uRotateLeft.z));
	*r01 = *r01 * glm::rotate(glm::mat4(), glm::radians(-fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(-fov / 2.0f), glm::vec3(uRotateLeft.x, uRotateLeft.y, uRotateLeft.z));
	*r10 = *r10 * glm::rotate(glm::mat4(), glm::radians(fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(fov / 2.0f), glm::vec3(uRotateRight.x, uRotateRight.y, uRotateRight.z));
	*r11 = *r11 * glm::rotate(glm::mat4(), glm::radians(fov * ratio / 2.0f), v) * glm::rotate(glm::mat4(), glm::radians(-fov / 2.0f), glm::vec3(uRotateRight.x, uRotateRight.y, uRotateRight.z));
}


// This runs once a frame, before renderScene
void update()
{
	// Used for FPS
	time = glfwGetTime();

	// Every second, basically.
	if (time - timebase > 1)
	{
		// Calculate the FPS and set the window title to display it.
		fps = frame / (time - timebase);
		timebase = time;
		frame = 0;

		std::string s = "FPS: " + std::to_string(fps);

		glfwSetWindowTitle(window, s.c_str());
	}

	// For rotating the camera, we convert it to a vec4 and then do a rotation about the Y axis.
	glm::vec4 tempPos = glm::vec4(cameraPos, 1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Then we convert back to a vec3.
	cameraPos = glm::vec3(tempPos.x, tempPos.y, tempPos.z);

	// These are our four corner ray variables.
	glm::vec4 r00;
	glm::vec4 r01;
	glm::vec4 r10;
	glm::vec4 r11;

	// Call the function we created to calculate the corner rays.
	calcCameraRays(cameraPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 800.0f / 600.0f, &r00, &r01, &r10, &r11);

	// Now set the uniform variables in the shader to match our camera variables (cameraPos = eye, then four corner rays)
	glUniform3f(eye, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(ray00, r00.x, r00.y, r00.z);
	glUniform3f(ray01, r01.x, r01.y, r01.z);
	glUniform3f(ray10, r10.x, r10.y, r10.z);
	glUniform3f(ray11, r11.x, r11.y, r11.z);
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Bind the vao
	glBindVertexArray(vao);

	// Draw 4 vertices from the buffer as GL_TRIANGLE_FAN which will draw triangles fanning out from the first vertex.
	// There are several different drawing modes, GL_TRIANGLES takes every 3 vertices and makes them a triangle.
	// For reference, GL_TRIANGLE_STRIP would take each additional vertex after the first 3 and consider that a 
	// triangle with the previous 2 vertices (so you could make 2 triangles with 4 vertices, 3 with 5, etc.)
	// The second parameter is the start index of the array, and the third parameter is the number of vertices to draw.
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// In this case, we are drawing 4 vertices because that will draw a flat quad. With Ray Tracing, the Fragment Shader will do most of the work, drawing every pixel 
	// by tracing a ray.
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

	// Tell our code to use the program
	glUseProgram(program);

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// We are drawing a quad, what this means is that we're drawing two triangles to create a rectangle that covers the entire view area.
	// Thus, we're only passing in 4 vertices in a triangle fan to the vertex shader. It will then call the pixel shader and draw the pixel at it's appropriate coordinate.
	glm::vec2 quadVerts[4];
	quadVerts[0] = glm::vec2(1.0, -1.0);
	quadVerts[1] = glm::vec2(-1.0, -1.0);
	quadVerts[2] = glm::vec2(-1.0, 1.0);
	quadVerts[3] = glm::vec2(1.0, 1.0);
	// No reason to bother with a Z parameter, as the quad will fill up the entire screen, facing the camera directly, making depth irrelevant.

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, &quadVerts[0], GL_STATIC_DRAW);
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	// You'll note sizeof(glm::vec2) is our stride, because each vertex is that size.

	// This gets us a reference to the uniform variables in the vertex shader, which are called by the same name here as in the shader.
	// We're using these variables to define the camera. The eye is the camera position, and teh rays are the four corner rays of what the camera sees.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	eye = glGetUniformLocation(program, "eye");
	ray00 = glGetUniformLocation(program, "ray00");
	ray01 = glGetUniformLocation(program, "ray01");
	ray10 = glGetUniformLocation(program, "ray10");
	ray11 = glGetUniformLocation(program, "ray11");

	// This is where we'll set up our camera location at.
	cameraPos = glm::vec3(4.0f, 8.0f, 8.0f);

	// These are four corner ray variables to store the output from our calcCameraRays function.
	glm::vec4 r00;
	glm::vec4 r01;
	glm::vec4 r10;
	glm::vec4 r11;

	// Call our function to calculate the four corner rays. We're choosing to make the point the camera centeras on at 0, 0.5, 0.
	// Our FoV angle is 60 degrees and our ratio is 800/600 which is just the pixel ratio.
	calcCameraRays(cameraPos, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 800.0f/600.0f, &r00, &r01, &r10, &r11);

	// Now set the uniform variables in the shader to match our camera variables (cameraPos = eye, then four corner rays)
	glUniform3f(eye, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(ray00, r00.x, r00.y, r00.z);
	glUniform3f(ray01, r01.x, r01.y, r01.z);
	glUniform3f(ray10, r10.x, r10.y, r10.z);
	glUniform3f(ray11, r11.x, r11.y, r11.z);

	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CW);
}

int main(int argc, char **argv)
{
	// FPS variables.
	frame = 0;
	time = 0.0;
	timebase = 0.0;
	fps = 0;

	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 600, "Basic Ray Tracer", nullptr, nullptr);

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

		frame++; // For framerate checking.

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}