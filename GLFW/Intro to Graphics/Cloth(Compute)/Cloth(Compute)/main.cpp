/*
Title: Cloth Simulation using Compute Shaders
File Name: main.cpp
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
This program demonstrates the implementation of cloth like surface using
compute shaders. The cloth is considered to be made up of a mesh of springs.
Each point represents a point mass on the cloth which is connected to 4 
other point masses (top, bottom, left and right).

Each point mass computes the force exerted on it due to the 4 springs, which
connect the point mass to the neighbouring masses. This computation is done 
in GPU, namely the compute shader.

In this example, we allocate the required memory space in GPU buffers. We make
4 buffers: One to read position data from and one to write position data to.
similarly for velocity of each particle. We have two separate buffers to read 
and write to avoid data races and read-before-write errors.

We send the data to one of the buffers once, then compute the position and velocity 
at the of the frame in the sahder and store it in the output buffers. Then we 
bind the output buffer to the GPU ARRAY BUFFER, and tell the GPU "how to" read the data,
i.e. set attribute pointers and enable them.

Then we simply call the drawArrays function of openGL. Notice how we never read 
from the buffers on the CPU side of the applicaiton. This is the advantage of using 
shaders in this type of situations: We avoid unnecessary transfer of data from CPU to GPU.

References:
OpenGL 4 shading language cookbook by David Wolff
*/

#include "GLIncludes.h"

//number of Particles in each direction. these  particles will be uniformly spread from 0 to 1.
#define NUMBER_OF_PARTICLES_X 80
#define NUMBER_OF_PARTICLES_Y 40
#define NUMBER_OF_PARTICLES (NUMBER_OF_PARTICLES_X * NUMBER_OF_PARTICLES_Y)

// Global data members
#pragma region Base_data

GLuint program;
GLuint computeProgram;

GLuint vao;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;
GLuint compute_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniVP;
//Store the buffer ID of the position buffers and velocity buffers
GLuint posBuf[2];
GLuint velBuf[2];

//This vector is used to simulate wind in this example.
glm::vec3 externalForce = glm::vec3(0);

//These values will be used to set the uniforms in the compute shaders
float horizontalRest, verticalRest, DiagonalRest;

glm::mat4 PV;

// Reference to the window object being created by GLFW.
GLFWwindow* window;

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions
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

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");
	std::string computeShader = readShader("computeShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);
	// Compile the compute sahder like any other shader
	compute_shader = createShader(computeShader, GL_COMPUTE_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.
	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);

	//Link the compute shader to a separate program.
	// The compute shader can not be linked to a program containing vertex or fragment or any shader which takes part in graphics pipeline.
	// The compute shader is NOT a part of the  graphics pipeline. Thus we require a separate program for compute shaders.
	computeProgram = glCreateProgram();
	glAttachShader(computeProgram, compute_shader);
	glLinkProgram(computeProgram);

	//get the pointers to the uniform variables
	uniVP = glGetUniformLocation(program, "VP");

	//Since each point will have a different transform matrix, which will be in vertexShader, but the persective and view matrix remain the same.
	// Calculate the prespective and View matrix for the current scene.Since the camera is fixed, we need to do this only once.
	PV = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f) * 
		glm::lookAt(glm::vec3(0.5f, 0.20f, 1.5f), glm::vec3(0.5f, 0.20f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glFrontFace(GL_CW);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
}

#pragma endregion Helper_functions

void setup()
{
	std::vector<GLfloat> positions;
	std::vector<GLfloat> velocity;
	glm::vec3 p, v;

	horizontalRest = 1.0f/ ((float) NUMBER_OF_PARTICLES_X) ;
	verticalRest = 1.0f / ((float) NUMBER_OF_PARTICLES_Y);
	DiagonalRest = sqrt((horizontalRest * horizontalRest) + (verticalRest * verticalRest));

	for (int i = 0; i < NUMBER_OF_PARTICLES_Y; i++)
	{
		for (int j = 0; j < NUMBER_OF_PARTICLES_X; j++)
		{
			p.x = j * horizontalRest;
			p.y = i * verticalRest;
			p.z = 0.0f;

			positions.push_back(p.x);
			positions.push_back(p.y);
			positions.push_back(p.z);
			positions.push_back(1.0f);

			velocity.push_back(0.0f);
			velocity.push_back(0.0f);
			velocity.push_back(0.0f);
			velocity.push_back(0.0f);
		}
	}


	/*glGenBuffers(2, posBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBuf[1]);

	glGenBuffers(2, velBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBuf[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * sizeof(glm::vec3), &velocity[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBuf[1]);*/

	glGenBuffers(1, &posBuf[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * sizeof(GLfloat), &positions[0], GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &posBuf[1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBuf[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &velBuf[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBuf[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, velocity.size() * sizeof(GLfloat), &velocity[0], GL_DYNAMIC_COPY);
	
	glGenBuffers(1, &velBuf[1]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBuf[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, velocity.size() * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

}

// Functions called between every frame. game logic
#pragma region util_functions
// This runs once every physics timestep.
void update()
{
	GLuint readBuffer = 0;
	
	//Use the program, compute shader is linked to and set the uniform values.
	glUseProgram(computeProgram);
	glUniform1f(glGetUniformLocation(computeProgram, "RestLengthHoriz"), horizontalRest);
	glUniform1f(glGetUniformLocation(computeProgram, "RestLengthVert"), verticalRest);
	glUniform1f(glGetUniformLocation(computeProgram, "RestLengthDiag"), DiagonalRest);
	glUniform3fv(glGetUniformLocation(computeProgram, "externalForce"), 1, (float*) &externalForce);
	
	//We do this to avoid a major change in one time step.
	//The fixed time step in the compute shader is really small. 
	//dispatching the compute shader 100 times gives us a euler 
	//integration with samll time step. which helps us to have gradual interpolation using Euler scheme.
	for (int i = 0; i < 1000; i++)
	{
		//Dispatch the compute shader with each work group having 10x10x1 dimensions
		glDispatchCompute(NUMBER_OF_PARTICLES_X/10 , NUMBER_OF_PARTICLES_Y/10 , 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
		//swap buffers
		readBuffer = 1 - readBuffer;

		//Bind the appropriate buffers, to the appropriate locations
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf[readBuffer]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, posBuf[1 - readBuffer]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, velBuf[readBuffer]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, velBuf[1 - readBuffer]);
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0, 1.0, 1.0, 1.0);
	
	glPointSize(4.0f);
	
	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);
	//Bind the appropriate buffer to read data from in the vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)0);

	glUniformMatrix4fv(uniVP, 1, GL_FALSE, glm::value_ptr(PV));
	glDrawArrays(GL_POINTS, 0, NUMBER_OF_PARTICLES);
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//This set of controls are used to move one point (point1) of the line.
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		//We change the external force to simulate the fluctuation in the wind forces.
		externalForce.x = (rand() % 5) * 0.1f ;
	}
	if (key == GLFW_KEY_SPACE && (action == GLFW_RELEASE))
		externalForce = glm::vec3(0);
	
}

#pragma endregion Helper_functions


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
			update();
			accumulator -= physicsStep;
		}
	}
}

void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Some title", nullptr, nullptr);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	std::cout << glGetString(GL_VERSION);

	setup();
	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
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