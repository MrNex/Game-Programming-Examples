/*
Title: Marching Cubes
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
Marching cubes is a algorithm to construct isosurfaces for oddly shaped
objects, like fluids. It can be used to attain higher detail at lower memory cost.
In this demo we construct a Sphere using marching cubes algorithm.

We divide the space into symmetric cubes. these cubes can be subdivided into smaller cubes 
for better resolution. This structure resembles that of a oct-tree. 
For each vertex of every cube, we check if that vertex constitues a part of the surface.
since each each vertex can either be a part or not, we have a combination of 2^8 different 
scenarios. But these can be reduced to 15 unique cases which  can be transformed to reproduce 
the other formations.

These 8 values can be stored in 1 byte using each bit to represent a corner. In this example,
we are using bool variables instead, for ease of understanding.  

in this example all the logic is in the setup().

References:
https://www.jvrb.org/past-issues/5.2008/1309
https://www.youtube.com/watch?v=LfttaAepYJ8 - best visual demo for understanding
https://en.wikipedia.org/wiki/Marching_cubes
*/

#include "GLIncludes.h"

#define GridSize 30
#define radius 1.0f/(float)GridSize	

struct gridCell
{
	glm::vec3 position;
	bool X, Xy, Xyz, Xz;													// Edges
	bool Y, Yx, Yxz, Yz;
	bool Z, Zx, Zxy, Zy;

	bool A1, B1, C1, D1, A2, B2, C2, D2;									// Vertices

	std::vector<glm::vec3> adjacentPoints;
};

#pragma region program specific Data members
gridCell matrix[GridSize][GridSize][GridSize];

int vertexCount = 0;
std::vector<VertexFormat> CPUbuffer;
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
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.


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
		//// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
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

//Since we are only drawing a single object, we need only 1 VBO. Thus we create an object of Stuff_for_drawing on a global scope
stuff_for_drawing base;

//This function checks if the passed value is true, if it is then it sets to false, else it sets to true. 
//If we had used bits to represent them, then we can simply use the XOR function.
bool setTrue(bool var)
{
	if (var)
		var = false;
	else
		var = true;

	return var;
}

//This function pushes data onto the buffer we will later send to the GPU. It also updates the vertexcount to keep track of it.
void pushToCPUBuffer(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	VertexFormat v, u,w;

	u.position = a;
	v.position = b;
	w.position = c;

	CPUbuffer.push_back(u);
	CPUbuffer.push_back(v);
	CPUbuffer.push_back(w);

	vertexCount += 3;
}

void setup()
{
	base.vbo = 0;
	base.numberOfVertices = 0;

	CPUbuffer.clear();

	//Find all the points which lie inside the or outside the sphere
	for (int i = 0; i < GridSize; i++)
		for (int j = 0; j < GridSize; j++)
			for (int k = 0; k < GridSize; k++)
			{
				matrix[i][j][k].position.x = (i * 2.0f / (float)GridSize) -1.0f;
				matrix[i][j][k].position.y = (j * 2.0f / (float)GridSize) -1.0f;
				matrix[i][j][k].position.z = (k * 2.0f / (float)GridSize) -1.0f;

				//for each cube in the grid, check for all 8 corners.
				if (glm::length(matrix[i][j][k].position + glm::vec3(-radius, -radius, radius)) <= 1.0f)
					matrix[i][j][k].A1 = true;
				else
					matrix[i][j][k].A1 = false;

				if (glm::length(matrix[i][j][k].position + glm::vec3(radius, -radius, radius))<= 1.0f)
					matrix[i][j][k].B1 = true;
				else
					matrix[i][j][k].B1 = false;

				if (glm::length(matrix[i][j][k].position + glm::vec3(radius, radius, radius))<= 1.0f)
					matrix[i][j][k].C1 = true;
				else
					matrix[i][j][k].C1 = false;

				if (glm::length(matrix[i][j][k].position + glm::vec3(-radius, radius, radius))<= 1.0f)
					matrix[i][j][k].D1 = true;
				else
					matrix[i][j][k].D1 = false;

				if (glm::length(matrix[i][j][k].position + glm::vec3(-radius, -radius, -radius))<= 1.0f)
					matrix[i][j][k].A2 = true;
				else
					matrix[i][j][k].A2 = false;

				if (glm::length(matrix[i][j][k].position + glm::vec3(radius, -radius, -radius))<= 1.0f)
					matrix[i][j][k].B2 = true;										 
				else																 
					matrix[i][j][k].B2 = false;										 
																					 
				if (glm::length(matrix[i][j][k].position + glm::vec3(radius, radius, -radius))<= 1.0f)
					matrix[i][j][k].C2 = true;										 
				else																 
					matrix[i][j][k].C2 = false;										 
																					 
				if (glm::length(matrix[i][j][k].position + glm::vec3(-radius, radius, -radius))<= 1.0f)
					matrix[i][j][k].D2 = true;
				else
					matrix[i][j][k].D2 = false;
				
				//set all the edges to false to start with.
				matrix[i][j][k].X = false;
				matrix[i][j][k].Xy = false;
				matrix[i][j][k].Xyz = false;
				matrix[i][j][k].Xz = false;

				matrix[i][j][k].Y = false;
				matrix[i][j][k].Yx = false;
				matrix[i][j][k].Yz = false;
				matrix[i][j][k].Yxz = false;

				matrix[i][j][k].Z = false;
				matrix[i][j][k].Zx = false;
				matrix[i][j][k].Zy = false;
				matrix[i][j][k].Zxy = false;
			}

	
	for (int i = 0; i < GridSize; i++)
		for (int j = 0; j < GridSize; j++)
			for (int k = 0; k < GridSize; k++)
			{
				// if all the points are ture or false, then the cube wither lies completly outside or complettly inside the object. 
				// In all other cases, we need to set true the edges in cantact with the highlighted corners.
				if (matrix[i][j][k].A1 || matrix[i][j][k].B1 || matrix[i][j][k].C1 || matrix[i][j][k].D1 || matrix[i][j][k].A2 || matrix[i][j][k].B2 || matrix[i][j][k].C2 || matrix[i][j][k].D2 &&
					(matrix[i][j][k].A1 && matrix[i][j][k].B1 && matrix[i][j][k].C1 && matrix[i][j][k].D1 && matrix[i][j][k].A2 && matrix[i][j][k].B2 && matrix[i][j][k].C2 && matrix[i][j][k].D2 == false))
				{
					if (matrix[i][j][k].A1)
					{
						matrix[i][j][k].X = setTrue(matrix[i][j][k].X);
						matrix[i][j][k].Y = setTrue(matrix[i][j][k].Y);
						matrix[i][j][k].Z = setTrue(matrix[i][j][k].Z);
					}
		
					if (matrix[i][j][k].B1)
					{
						matrix[i][j][k].X = setTrue(matrix[i][j][k].X);
						matrix[i][j][k].Yx = setTrue(matrix[i][j][k].Yx);
						matrix[i][j][k].Zx = setTrue(matrix[i][j][k].Zx);
					}
		
					if (matrix[i][j][k].C1)
					{
						matrix[i][j][k].Xy = setTrue(matrix[i][j][k].Xy);
						matrix[i][j][k].Yx = setTrue(matrix[i][j][k].Yx);
						matrix[i][j][k].Zxy = setTrue(matrix[i][j][k].Zxy);
					}
		
					if (matrix[i][j][k].D1)
					{
						matrix[i][j][k].Xy = setTrue(matrix[i][j][k].Xy);
						matrix[i][j][k].Y = setTrue(matrix[i][j][k].Y);
						matrix[i][j][k].Zy = setTrue(matrix[i][j][k].Zy);
					}
		
					if (matrix[i][j][k].A2)
					{
						matrix[i][j][k].Xz = setTrue(matrix[i][j][k].Xz);
						matrix[i][j][k].Yz = setTrue(matrix[i][j][k].Yz);
						matrix[i][j][k].Z = setTrue(matrix[i][j][k].Z);
					}
		
					if (matrix[i][j][k].B2)
					{
						matrix[i][j][k].Xz = setTrue(matrix[i][j][k].Xz);
						matrix[i][j][k].Yxz = setTrue(matrix[i][j][k].Yxz);
						matrix[i][j][k].Zx = setTrue(matrix[i][j][k].Zx);
					}
		
					if (matrix[i][j][k].C2)
					{
						matrix[i][j][k].Xyz = setTrue(matrix[i][j][k].Xyz);
						matrix[i][j][k].Yxz = setTrue(matrix[i][j][k].Yxz);
						matrix[i][j][k].Zxy = setTrue(matrix[i][j][k].Zxy);
					}
		
					if (matrix[i][j][k].D2)
					{
						matrix[i][j][k].Xyz = setTrue(matrix[i][j][k].Xyz);
						matrix[i][j][k].Yz = setTrue(matrix[i][j][k].Yz);
						matrix[i][j][k].Zy = setTrue(matrix[i][j][k].Zy);
					}
			}
	}

	//At this point we know every vertex on the edge of the every single cube which needs to be drawn to depict the surface.
	int verticesToDraw = 0;
	glm::vec3 a, b, c;

	//This is where use another algorithm to conpute the mesh using the points given to us. (INCOMPLETE)
	for (int i = 0; i < GridSize; i++)
	{
		for (int j = 0; j < GridSize; j++)
		{
			for (int k = 0; k < GridSize; k++)
			{
				//First we push all the vertices to draw to a vector buffer.
				matrix[i][j][k].adjacentPoints.clear();
				verticesToDraw = 0;
				glm::vec3 Position = matrix[i][j][k].position;
				//X-axis
				if (matrix[i][j][k].X)
				{
					verticesToDraw++;
					Position += glm::vec3(0, -radius, radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Xy)
				{
					verticesToDraw++;
					Position += glm::vec3(0, radius, radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Xyz)
				{
					verticesToDraw++;
					Position += glm::vec3(0, radius, -radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Xz)
				{
					verticesToDraw++;
					Position += glm::vec3(0, -radius, -radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				//Yaxis
				if (matrix[i][j][k].Y)
				{
					verticesToDraw++;
					Position += glm::vec3(-radius, 0, radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Yx)
				{
					verticesToDraw++;
					Position += glm::vec3(radius, 0, radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Yxz)
				{
					verticesToDraw++;
					Position += glm::vec3(radius, 0, -radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Yz)
				{
					verticesToDraw++;
					Position += glm::vec3(-radius, 0, -radius);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				//Z-axis
				if (matrix[i][j][k].Z)
				{
					verticesToDraw++;
					Position += glm::vec3(-radius, -radius, 0);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Zx)
				{
					verticesToDraw++;
					Position += glm::vec3(radius, -radius, 0);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Zxy)
				{
					verticesToDraw++;
					Position += glm::vec3(radius, radius, 0);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}
				if (matrix[i][j][k].Zy)
				{
					verticesToDraw++;
					Position += glm::vec3(-radius, radius, 0);
					matrix[i][j][k].adjacentPoints.push_back(Position);
					//glVertex3fv((float*)&Position);
					Position = matrix[i][j][k].position;
				}

				//Depending on the number of points we can roughly deduce the kind of shape we need to draw.
				//Here we are doing only the basic three cases of the 15 possible cases to render a sphere.
				switch (verticesToDraw)
				{
				case 3: 
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[0], matrix[i][j][k].adjacentPoints[1], matrix[i][j][k].adjacentPoints[2]);
					break;
				case 4:
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[0], matrix[i][j][k].adjacentPoints[1], matrix[i][j][k].adjacentPoints[2]);
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[0], matrix[i][j][k].adjacentPoints[1], matrix[i][j][k].adjacentPoints[3]);
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[1], matrix[i][j][k].adjacentPoints[2], matrix[i][j][k].adjacentPoints[3]);
					break;
				case 5:
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[0], matrix[i][j][k].adjacentPoints[1], matrix[i][j][k].adjacentPoints[2]);
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[0], matrix[i][j][k].adjacentPoints[1], matrix[i][j][k].adjacentPoints[3]);
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[0], matrix[i][j][k].adjacentPoints[3], matrix[i][j][k].adjacentPoints[4]);
					pushToCPUBuffer(matrix[i][j][k].adjacentPoints[2], matrix[i][j][k].adjacentPoints[3], matrix[i][j][k].adjacentPoints[4]);
					break;
				default:
					//std::cout << "\n " << matrix[i][j][k].adjacentPoints.size();
					break;
				}

			}
		}
	}

	base.initBuffer(vertexCount, &CPUbuffer[0]);
}

#pragma region Helper_functions
// Functions called only once every time the program is executed.
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// if we were writing to the file, we would use ofstream and std::ios::out.
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
	// First parameter is camera position, second parameter is point to be centered on-screen, and the third paramter is the up axis.
	view = glm::lookAt(glm::vec3(0.0f , 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
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
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-axis, you wouldn't see the other side of the triangle as it rotated.
	//glEnable(GL_CULL_FACE);
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
	std::vector<glm::vec3> adjacentPoints;

	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.2f, 0.2f, 0.2f, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	glLineWidth(1.0f);
	
	//glm::vec3 p;
	//glm::mat4 m;
	//glEnable(GL_POINT_SMOOTH);
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glPointSize(1);

	glBindBuffer(GL_ARRAY_BUFFER, base.vbo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, position));
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexFormat), (void*)offsetof(VertexFormat, color));

	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glm::vec3 Position;
	
	//glBegin(GL_LINES);
	//for (int i = 0; i < GridSize; i++)
	//{
	//	for (int j = 0; j < GridSize; j++)
	//	{
	//		for (int k = 0; k < GridSize; k++)
	//		{
	//			for (int l = 0; l < matrix[i][j][k].adjacentPoints.size(); l++)
	//			{
	//				for (int m = l + 1; m < matrix[i][j][k].adjacentPoints.size(); m++)
	//				{
	//					//if (glm::distance(matrix[i][j][k].adjacentPoints[m], matrix[i][j][k].adjacentPoints[l]) <= 2 * radius)
	//					{
	//						glVertex3fv((float*)&matrix[i][j][k].adjacentPoints[l]);
	//						glVertex3fv((float*)&matrix[i][j][k].adjacentPoints[m]);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
	//glEnd();
}

// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS))
	//{
	//	if (G.x >= 0)
	//	{
	//		G.x = -14.8f;
	//	}
	//	else
	//		G.x = 0;
	//}
}
#pragma endregion

void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "FMarching Cubes", nullptr, nullptr);

	std::cout << "\n This program demonstrates implementation of marching cubes to render a sphere\n\n\n\n\n\n\n\n\n\n";
	
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