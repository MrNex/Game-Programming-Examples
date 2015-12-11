/*
Title: Bezier curve tessellation
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
This program demonstrates the implementation of bezier curve with the tessellation
being computed in the shader. In this program, we send only the control points to
the shader and adjust the tessellation level accordingly.

The tessellation control shader begins by defining the number of vertices in the
output patch: layout (vertices = 4) out;

Note that this is not the same number of vertices that will be produced by the process.
In this case the number of control points are 4, so we pass 4 in one patch.

The vertex sahder simply passes the input data as read from the buffer to the next shader
(tessellation control shader). The TCS sets the tessellation levels by assigning a value to
gl_TessLevlOuter array. The first element defines the number of isolines that will be generated.
In this example that value will be 1. The second value defines the number of line segments per
isoline.

In tessellation evaluation shader, we start by defining the input primitive type using a layout
declaration: layout (isolines) in;
In TCS, we access the uv coordinates using glTessCoord. Then we access the position of the
four control points (all the points in our patch). Using these values we compute the bernstein
polynomials at given uv cordinates and calculate the position.

References:
OpenGL 4 Shading language cookbook (second edition)
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;
GLuint tessEval_shader;
GLuint tessCtrl_shader;

GLuint vbo;

// This is a reference to your uniform variables in your program
GLuint uniMVP;
GLuint uniNumStrips;
GLuint uniNumSegments;

glm::mat4 MVP;
int NumStrips = 1;
int NumSegments = 15;

// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region 
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	file.seekg(0, std::ios::end);					
	shaderCode.resize((unsigned int)file.tellg());	
	file.seekg(0, std::ios::beg);					

	file.read(&shaderCode[0], shaderCode.size());	
	
	file.close(); 
	return shaderCode;
}

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
void initOpenGL()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");
	std::string tesseval = readShader("TessEvalShader.glsl");
	std::string tesscontrol = readShader("TessControlShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);  
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);
	tessCtrl_shader = createShader(tesscontrol, GL_TESS_CONTROL_SHADER);
	tessEval_shader = createShader(tesseval, GL_TESS_EVALUATION_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, tessCtrl_shader);	// This attaches our tessellation control shader to our program.
	glAttachShader(program, tessEval_shader);	// This attaches our tessellation evaluation shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	uniMVP = glGetUniformLocation(program, "MVP");
	uniNumStrips = glGetUniformLocation(program, "NumStrips");
	uniNumSegments = glGetUniformLocation(program, "NumSegments");

	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
}

#pragma endregion Helper_functions


void setupData()
{
	std::vector<glm::vec2> controlPoints;

	//Set up the control points
	controlPoints.push_back(glm::vec2(-0.75f, -0.75f));
	controlPoints.push_back(glm::vec2(-0.5f, 0.75f));
	controlPoints.push_back(glm::vec2(0.5f, -0.75f));
	controlPoints.push_back(glm::vec2(0.75f, 0.75f));

	//Uncomment this section ot add another curve
	//controlPoints.push_back(glm::vec2(-0.75f, 0.75f));
	//controlPoints.push_back(glm::vec2(-0.5f, -0.75f));
	//controlPoints.push_back(glm::vec2(0.5f, 0.75f));
	//controlPoints.push_back(glm::vec2(0.75f, -0.75f));

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * controlPoints.size(), &controlPoints[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	MVP = glm::mat4(1);
}

// Functions called between every frame. game logic
#pragma region 
// This runs once every physics timestep.
void update()
{
	
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.3f, 0.3f, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);
	//glLineWidth(3.0f);

	// Set the uniform matrix in our shader to our MVP matrix for the first object.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glUniform1i(uniNumSegments, NumSegments);
	glUniform1i(uniNumStrips, NumStrips);
	glUniformMatrix4fv(uniMVP,1,GL_FALSE, glm::value_ptr(MVP));

	glDrawArrays(GL_PATCHES, 0, 4);
	// Draw the Gameobjects
	
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//This set of controls are used to increase or decrease the tessellation of the curve
	if (key == GLFW_KEY_KP_ADD && (action == GLFW_PRESS || action == GLFW_REPEAT))
		NumSegments++;
	if (key == GLFW_KEY_KP_SUBTRACT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		NumSegments--;
}

#pragma endregion util_functions


void main()
{
	glfwInit();
	window = glfwCreateWindow(800, 800, "Tessellating a bezier curve", nullptr, nullptr);

	std::cout << "This program demonstrates the rendering of bezier curve, with the tessellating being executed in the tessellation shaders.";
	std::cout << "\n Use numpad + and numpad - to increase or decrease tessellation.";

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	initOpenGL();
	setupData();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{

		update();
		renderScene();
		glfwSwapBuffers(window);
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