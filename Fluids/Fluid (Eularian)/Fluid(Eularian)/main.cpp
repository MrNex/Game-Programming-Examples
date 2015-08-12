/*
Title: Fluid Simulation (Eularian)
File Name: main.cpp
Copyright © 2015
Original authors: Srinivasan Thiagarajan
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
In this example we demonstrate the implementation of fluid motion using Eularian 
approach. In this approach, we only deal with the velocity field in this example,
but the core concepts can be extended to other fields as well. 

The velocity field experiences the 3 separate kinds of forces, namely diffusion,
advection and external forces.

In the Eularian approach, the particles do not have mass. The entire area is classified into 
grids, the particles in a specific grid follow a the same path (have the same velocity).
The velocites of the grid consitute the velocity field.

Diffusion is the property of the of a fluid to spread a value accross the neighbors. 
Advection is the property of a fluid to carry objects from one point to another.Self advection 
is a part of the fluid motion.

use mouse to "Click and Drag" to add forces.

References:
Nicholas Gallagher
Real-Time Fluid Dynamics for Games by Jos Stam
*/

#include "GLIncludes.h"

#define POINTSIZE 5.0f
#define NUMBER_OF_PARTICLES 100
#define numberOfGrid 40
#define N (numberOfGrid-2)
#define XX(i,j) ((i)+ (numberOfGrid * (j)))
#define swap(a,b) {glm::vec3* temp = a; a =b; b= temp;}
#define Viscosity 0.001002f 

#pragma region program specific Data members
glm::vec3 G(0.0f, -9.8f, 0.0f);
glm::vec3 particles[NUMBER_OF_PARTICLES * NUMBER_OF_PARTICLES];
//float*	  pressure;
glm::vec3* velocity;
glm::vec3* prevVelocity;
bool mouseHeldDown = false;
double Xpos, Ypos, prevX,prevY;
double xdisplacement, ydisplacement;
#pragma endregion

#pragma region Global Data member
// Global data members
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;
GLuint color;

glm::mat4 view;
glm::mat4 proj;
glm::mat4 PV;
glm::mat4 MVP;


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.022; // This is the number of milliseconds we intend for the physics to update.


// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion

//This struct consists of the basic stuff needed for getting the shape on the screen.
struct stuff_for_drawing{
	
	//This stores the address the buffer/memory in the GPU. It acts as a handle to access the buffer memory in GPU.
	GLuint vbo;

	//This will be used to tell the GPU, how many vertices will be needed to draw during drawcall.
	int numberOfVertices;

	//This function gets the number of vertices and all the vertex values and stores them in the buffer.
	void initBuffer(int numVertices, VertexFormat* vertices)
	{
		numberOfVertices = numVertices;

		// This generates buffer object names
		// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
		glGenBuffers(1, &vbo);
		
		//// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
		//// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
		//// The second paramter is the buffer object reference. If no buffer object with the given name eXXsts, it will create one.
		//// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
		//// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		//// Creates and initializes a buffer object's data.
		//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
		//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
		//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
		//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
		//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
		//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);

		//// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
		//// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
		//// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
		glEnableVertexAttribArray(0);

		//// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
		//// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
		//// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
		//// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
		//// then the pointer parameter is treated as a byte offset into the buffer object's data.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
		//// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
		//// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
		//// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
		//// For more info on padding, Google it.

		//// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	}
};

void setup()
{
	//pressure = new float[(numberOfGrid * numberOfGrid)];
	velocity = new glm::vec3[(numberOfGrid * numberOfGrid)];			// Velocity field
	prevVelocity = new glm::vec3[(numberOfGrid * numberOfGrid)];		// A buffer used to compute the velocity

	for (int i = 0; i < numberOfGrid; i++)
	{
		for (int j = 0; j < numberOfGrid; j++)
		{
			velocity[XX(i, j)] = glm::vec3(0);
			//Set the velocity for each grid cell
		}
	}

	// this causes the points to be generated randomly while maintaining the uniformity
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.0f, 10.0f);

	for (int i = 0; i < NUMBER_OF_PARTICLES; i++)
	{
		for (int j = 0; j < NUMBER_OF_PARTICLES; j++)
		{
			particles[(i*NUMBER_OF_PARTICLES) + j].x = distribution(generator);
			particles[(i*NUMBER_OF_PARTICLES) + j].y = distribution(generator);
			particles[(i*NUMBER_OF_PARTICLES) + j].z = 0.0;
		}
	}

}

void clear(glm::vec3* a)
{
	//This function clears the buffer velocity field.
	for (int i = 0; i < numberOfGrid; i++)
	{
		for (int j = 0; j < numberOfGrid; j++)
		{
			velocity[XX(i, j)] = glm::vec3(0);
		}
	}
}

glm::vec3 EulerIntegrator(glm::vec3 pos, float h, glm::vec3 &velocity, glm::vec3 acc)
{
	glm::vec3 P;

	velocity += h * acc;

	//Calculate the displacement in that time step with the current velocity.
	P = pos + (h * velocity);

	//return the position P
	return P;
}

void set_bnd(int b, float * x)
{
	//This funciton set the boundary values 
	int i;
	for (i = 1; i <= N; i++) {
		x[XX(0, i)] = x[XX(1, i)];
		x[XX(N + 1, i)] = x[XX(N, i)];
		x[XX(i, 0)] = x[XX(i, 1)];
		x[XX(i, N + 1)] = x[XX(i, N)];
	}

	x[XX(0, 0)] = 0.5*(x[XX(1, 0)] + x[XX(0, 1)]);
	x[XX(0, N + 1)] = 0.5*(x[XX(1, N + 1)] + x[XX(0, N)]);
	x[XX(N + 1, 0)] = 0.5*(x[XX(N, 0)] + x[XX(N + 1, 1)]);
	x[XX(N + 1, N + 1)] = 0.5*(x[XX(N, N + 1)] + x[XX(N + 1, N)]);
}

void set_bnd1(int b0, int b1, glm::vec3 * x)
{
	//This funciton set the boundary valeus of the velocity field. The boudnary values need to be 
	//set separately for the as they should be able to contain the fluid inside the volume. 
	//This can be changed as needed for eg: the particles wrap around the screen. But diffusion code also needs to be altered to affect this.
	int i;
	for (i = 1; i <= N; i++) {
		x[XX(0, i)].x = b0 == 1 ? -x[XX(1, i)].x : x[XX(1, i)].x;
		x[XX(N + 1, i)].x = b0 == 1 ? -x[XX(N, i)].x : x[XX(N, i)].x;
		x[XX(i, 0)].x = b0 == 2 ? -x[XX(i, 1)].x : x[XX(i, 1)].x; x[XX(i, N + 1)].x = b0 == 2 ? -x[XX(i, N)].x : x[XX(i, N)].x;
	}

	x[XX(0, 0)].x = 0.5*(x[XX(1, 0)].x + x[XX(0, 1)].x);
	x[XX(0, N + 1)].x = 0.5*(x[XX(1, N + 1)].x + x[XX(0, N)].x);
	x[XX(N + 1, 0)].x = 0.5*(x[XX(N, 0)].x + x[XX(N + 1, 1)].x);
	x[XX(N + 1, N + 1)].x = 0.5*(x[XX(N, N + 1)].x + x[XX(N + 1, N)].x);

	for (i = 1; i <= N; i++) {
		x[XX(0, i)].y = b1 == 1 ? -x[XX(1, i)].y : x[XX(1, i)].y;
		x[XX(N + 1, i)].y = b1 == 1 ? -x[XX(N, i)].y : x[XX(N, i)].y;
		x[XX(i, 0)].y = b1 == 2 ? -x[XX(i, 1)].y : x[XX(i, 1)].y; x[XX(i, N + 1)].y = b1 == 2 ? -x[XX(i, N)].y : x[XX(i, N)].y;
	}

	x[XX(0, 0)].y = 0.5*(x[XX(1, 0)].y + x[XX(0, 1)].y);
	x[XX(0, N + 1)].y = 0.5*(x[XX(1, N + 1)].y + x[XX(0, N)].y);
	x[XX(N + 1, 0)].y = 0.5*(x[XX(N, 0)].y + x[XX(N + 1, 1)].y);
	x[XX(N + 1, N + 1)].y = 0.5*(x[XX(N, N + 1)].y + x[XX(N + 1, N)].y);
}

//This function diffuses the velocity of the a grid to the neighbouring grid cells.
//Diffusion refers to the process by which molecules intermingle as a result of their 
//kinetic energy of random motion.
void diffuse(glm::vec3* x, glm::vec3* x0, float diff, float dt)
{
	int i, j, k;

	float a = dt * diff * N * N;

	//clear(x);

	for (k = 0; k < 20; k++)
	{
		for (i = 1; i <= N; i++)
		{
			for (j = 1; j <= N; j++)
			{
				x[XX(i, j)] = (x0[XX(i, j)] + a * (x[XX((i - 1), j)] + x[XX((i + 1), j)] + x[XX(i, (j + 1))] + x[XX(i, (j - 1))])) / (1 + 4 * a);
			}
		}

		//Set the boundary Conditions here.
		set_bnd1(N, 2, x);
	}
}

//This function advects the velocity of a grid cell.
// Advection is the transfer of matter by the flow of a fluid.
void advect(glm::vec3* X, glm::vec3* X0, float dt)
{
	int i, j, i0, j0, i1, j1;
	float x, y, s0, t0, s1, t1, dt0;

	dt0 = dt * N;

	for (i = 1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			
			//We integrate the position byt dt using the negative of velcoty.
			x = i - dt0*X0[XX(i, j)].x;
			y = j - dt0*X0[XX(i, j)].y;

			//When we get the final position, we clamp it to ensure they don't end up outside the grid.
			if (x<0.5) x = 0.5;
			if (x>N + 0.5) x = N + 0.5;
			i0 = (int)x;
			i1 = i0 + 1;

			if (y<0.5) y = 0.5;
			if (y>N + 0.5) y = N + 0.5;
			j0 = (int)y;
			j1 = j0 + 1;

			//We find the closest points and depending on how close it is to a grid, that much of the velocity is added to that grid.
			//for eg: if the point is at 0.7, then the current grid will get 0.3 % of the velocity at (0) and the 0.7% of the velocity at (1)
			s1 = x - i0;
			s0 = 1 - s1;
			t1 = y - j0;
			t0 = 1 - t1;
			
			//std::cout << "\n " << s1 << " " << s0 << " " << t1 << " " << t0;

			X[XX(i, j)] = s0*(t0*X0[XX(i0, j0)] + t1*X0[XX(i0, j1)]) + s1*(t0*X0[XX(i1, j0)] + t1*X0[XX(i1, j1)]);
		}
	}
	//set_bnd1(N, 2, X);
}

void project(glm::vec3* u)
{
	//This function conserves mass. The eularian approach does not account for conservation of mass.
	//This function ensure the velocity field is distributed accounting for conservation of mass.
	//without the project(), the particels get piled up at the same point. This is because this 
	//approach does not account for pressure increase.
	int i, j, k;
	float h;

	float div[numberOfGrid*numberOfGrid];
	float p[numberOfGrid*numberOfGrid];

	h = 1.0 / N;

	for (i = 0; i < numberOfGrid*numberOfGrid; i++)
	{
		div[i] = u[i].y;
	}

	for (i = 1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			div[XX(i, j)] =
				-0.5f * h * (
				u[XX((i + 1), j)].x
				- u[XX((i - 1), j)].x
				+ u[XX(i, (j + 1))].y
				- u[XX(i, (j - 1))].y
				);

			p[XX(i, j)] = 0;
		}
	}

	set_bnd(0, div); set_bnd(0, p);

	for (k = 0; k<20; k++) {
		for (i = 1; i <= N; i++) {
			for (j = 1; j <= N; j++) {
				p[XX(i, j)] = (div[XX(i, j)] + p[XX((i - 1), j)] + p[XX((i + 1), j)] +
					p[XX(i, (j - 1))] + p[XX(i, (j + 1))]) / 4;
			}
		}
		set_bnd(0, p);
	}

	for (i = 1; i <= N; i++) {
		for (j = 1; j <= N; j++) {
			u[XX(i, j)].x -= 0.5f*(p[XX((i + 1), j)] - p[XX((i - 1), j)]) / h;
			u[XX(i, j)].y -= 0.5f*(p[XX(i, (j + 1))] - p[XX(i, (j - 1))]) / h;
		}
	}

	set_bnd1(1, 2, u);
}

void updateCursorPositions()
{
	//This function calculates the cursor position and the displacement.
	//The cursor position is in pixel values with respect to the window.
	glfwGetCursorPos(window, &Xpos, &Ypos);

	xdisplacement = prevX - Xpos;
	ydisplacement = prevY - Ypos;
	prevX = Xpos;
	prevY = Ypos;

	//calculating the position in terms of grid position
	Xpos /= 800.0f;
	Ypos /= 800.0f;

	Xpos *= numberOfGrid;
	Ypos *= numberOfGrid;
	Ypos = numberOfGrid - Ypos;


	if (mouseHeldDown)
	{
		std::cout << "\n Xpos :" << (int)Xpos << " Ypos" << (int)Ypos << " " << xdisplacement << " " << ydisplacement;
		velocity[XX((int)Xpos, (int)Ypos)] += glm::vec3(-xdisplacement, ydisplacement, 0.0f);
	}
}

void updateVelocities(float t)
{
	//Diffuse 
	diffuse(prevVelocity, velocity, Viscosity, t);
	swap(prevVelocity, velocity);
	//conserve mass
	project(velocity);
	//Advect
	advect(prevVelocity, velocity, t);
	swap(prevVelocity, velocity);
	//Conserve mass
	project(velocity);
}

void integrate(float dt)
{
	//update the position of each particle
	int x, y;
	float denomX = 10.0f / (float)numberOfGrid, denomY = 10.0f / (float)numberOfGrid;
	glm::vec3 V;

	for (int i = 0; i < NUMBER_OF_PARTICLES*NUMBER_OF_PARTICLES; i++)
	{
		x = particles[i].x / denomX;
		y = particles[i].y / denomY;

		V = velocity[XX(x, y)];

		particles[i] = EulerIntegrator(particles[i], dt, V, glm::vec3(0));
	}
}

#pragma region Helper_functions
// Functions called only once every time the program is executed.
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
		// EXXt with failure.
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

	// Creates the view matrix using glm::lookAt.
	// First parameter is camera position, second parameter is point to be centered on-screen, and the third paramter is the up aXXs.
	view = glm::lookAt(glm::vec3(5.0f, 5.0f, 10.0f), glm::vec3(5.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	// Creates a projection matrix using glm::perspective.
	// First parameter is the vertical FoV (Field of View), second paramter is the aspect ratio, 3rd parameter is the near clipping plane, 4th parameter is the far clipping plane.
	proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	PV = proj * view;

	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	MVP = PV* translation;
	
	// This gets us a reference to the uniform variable in the vertex shader, which is called "color".
	// We're using this variable to change color during runtime, without changing the buffer values.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniMVP = glGetUniformLocation(program, "MVP");
	
	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CCW);

	// This is also not necessary, but more efficient and is generally good practice. By default, OpenGL will render both sides of a triangle that you draw. By enabling GL_CULL_FACE, 
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-aXXs, you wouldn't see the other side of the triangle as it rotated.
	glEnable(GL_CULL_FACE);
	//We are disabling hte cull face, because we wish to see both the front and back of the objects in wireframe mode for better understanding the depth.

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

#pragma endregion

#pragma region util_functions
// Functions called between every frame. game logic

// This runs once every physics timestep.
void update(float t)
{
	//take input
	updateCursorPositions();
	//update the velocities.
	updateVelocities(t);
	//calculate the new position of the particles.
	integrate(t);
}

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
			update(physicsStep);
			accumulator -= physicsStep;
		}
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.2f, 0.2f, 0.2f, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	glLineWidth(1.0f);
	
	glm::vec3 p;
	glm::mat4 m;
	glEnable(GL_POINT_SMOOTH);
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(PV));
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(POINTSIZE);
	
	glBegin(GL_POINTS);
	
	for (int i = 0; i < NUMBER_OF_PARTICLES* NUMBER_OF_PARTICLES ; i++)
	{
		glVertex3fv((float*)&particles[i]);
	}
	glEnd();
}

// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		velocity[XX(numberOfGrid/2, ((numberOfGrid/2) +1 ))] += glm::vec3(-1.0,0.0f,0.0f);
		velocity[XX(numberOfGrid / 2, ((numberOfGrid / 2) -1))] += glm::vec3(1.0, 0.0f, 0.0f);
		velocity[XX(((numberOfGrid / 2) + 1), numberOfGrid / 2)] += glm::vec3(0.0, 1.0f, 0.0f);
		velocity[XX(((numberOfGrid / 2) - 1), numberOfGrid / 2)] += glm::vec3(0.0, -1.0f, 0.0f);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS))
	{
		mouseHeldDown = true;
	}
	
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		mouseHeldDown = false;
	}
	
}
#pragma endregion

void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Fluid (Eularian)", nullptr, nullptr);

	std::cout << "\n This program demonstrates implementation of fluid motion with Eularian appraoch \n\n\n\n\n\n\n\n\n\n";
	std::cout << "\n use mouse to click and drag to add forces.";
	
	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	setup();

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