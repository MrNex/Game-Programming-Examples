/*
Title: 3D angular Friction
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
This program demonstrates the implementation of angular friction in games. 
It builds upon the previous example of "3D Friction". We use the same concept 
as linear friction to caluculate the forces at the point of contact along 
the axis of spin. 

Then we use the same concept as in the Linear friction, and calculate the 
fritional force. In this model, the force is computed as an impulse acting
at the point of contact.

Use "Space" to reset the simulation and "W" to add angular velocity the box.

References:
Nicholas Gallagher
AABB-2D by Brockton Roth
*/


#include "GLIncludes.h"


#pragma region program specific Data members
float Speed = 0.05f;
glm::vec3 POC(0.0f, 0.0f, 0.0f);
float timeStep = 0.012f;
glm::vec3 G(0.0f, -0.98f, 0.0f);

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


// Just a square box to show where the fluid starts
struct ConvexHull {
	glm::vec3 origin;							// Since a convex hull can have any shape, there is no generalized way to store the shape.
	std::vector<glm::vec3> edgeSet;				// Here we are storing all the edges constituting the object
	std::vector<glm::vec3> vertexSet;			// Here we are storing all the vertices constituting the object
	std::vector<glm::vec3> faceNormalSet;		// Here we are storing all the face normals constituting the object; this is just for convinience as it reduces a lot of redundant calculations
	glm::mat4 MVP;
	glm::mat4 RotationMat;
	glm::mat3 inertiatensor;
	glm::vec3 linearVelocity;
	glm::vec3 linearAcc;
	glm::vec3 angularVelocity;
	glm::vec3 angularAcc;
	stuff_for_drawing base;
	float e;						// Coefficient of restitution
	float mass;
	float inverseMass;
	float Fstatic;					// Coefficient of static friction
	float Fdynamic;					// Coefficient of dynamic friction
}box1,box2;


ConvexHull* boxInfocus, *boxOutfocus;


struct Line{
	glm::vec3 point1;
	glm::vec3 point2;
};


void setup()
{//This function sets up the two shapes we need for this example.
	std::vector<VertexFormat> vertices;

	//Setting up box1
	vertices.clear();
	box1.origin = glm::vec3(-0.5f, 0.0f, 0.0f);
	
	glm::vec3 p1, p2, p3, p4, p5, p6, p7, p8;

	// The 8 vertices of object 1
	p1 = glm::vec3(0.2f, 0.2f, 0.2f);
	p2 = glm::vec3(-0.2f, 0.2f, 0.2f);
	p3 = glm::vec3(-0.2f, -0.2f, 0.2f);
	p4 = glm::vec3(0.2f, -0.2f, 0.2f);
	p5 = glm::vec3(0.2f, 0.2f, -0.2f);
	p6 = glm::vec3(-0.2f, 0.2f, -0.2f);
	p7 = glm::vec3(-0.2f, -0.2f, -0.2f);
	p8 = glm::vec3(0.2f, -0.2f, -0.2f);

	box1.vertexSet.push_back(p1);
	box1.vertexSet.push_back(p2);
	box1.vertexSet.push_back(p3);
	box1.vertexSet.push_back(p4);
	box1.vertexSet.push_back(p5);
	box1.vertexSet.push_back(p6);
	box1.vertexSet.push_back(p7);
	box1.vertexSet.push_back(p8);

	// Edges making up the polygon
	box1.edgeSet.push_back(p1 - p2);
	box1.edgeSet.push_back(p2 - p3);
	box1.edgeSet.push_back(p3 - p4);
	box1.edgeSet.push_back(p4 - p1);
	box1.edgeSet.push_back(p2 - p6);
	box1.edgeSet.push_back(p6 - p5);
	box1.edgeSet.push_back(p5 - p1);
	box1.edgeSet.push_back(p8 - p4);
	box1.edgeSet.push_back(p5 - p8);
	box1.edgeSet.push_back(p6 - p7);
	box1.edgeSet.push_back(p7 - p3);
	box1.edgeSet.push_back(p8 - p7);

	// Facenormals of each face
	// Since we are generating a cube, we don't need to give 6 facenormals. as 3 base axis will do.
	box1.faceNormalSet.push_back(glm::vec3(1.0f,0.0f,0.0f));
	box1.faceNormalSet.push_back(glm::vec3(0.0f,1.0f,0.0f));
	box1.faceNormalSet.push_back(glm::vec3(0.0f,0.0f,1.0f));

	vertices.clear();

	// Push the vertices for each face in counte clockwise manner
	//Front Face
	vertices.push_back(VertexFormat(p1, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p2, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p3, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	vertices.push_back(VertexFormat(p1, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p3, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p4, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	
	//Top face
	vertices.push_back(VertexFormat(p1, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p5, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p6, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	vertices.push_back(VertexFormat(p1, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p6, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p2, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	//Left face
	vertices.push_back(VertexFormat(p2, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p6, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p7, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	vertices.push_back(VertexFormat(p2, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p7, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p3, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	
	//Right face
	vertices.push_back(VertexFormat(p1, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p4, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p8, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	vertices.push_back(VertexFormat(p1, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p8, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p5, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	//bottom face
	vertices.push_back(VertexFormat(p3, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p7, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p8, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	vertices.push_back(VertexFormat(p3, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p8, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p4, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	//Back face
	vertices.push_back(VertexFormat(p5, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p7, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p6, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	vertices.push_back(VertexFormat(p5, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p8, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p7, glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)));

	box1.base.initBuffer(30, &vertices[0]);
	
	//Setting up plane
	box2.origin = glm::vec3(0.0f, -0.5f, 0.0f);

	// The 5 vertices of object 2
	p1 = glm::vec3(1.5f, 0.0f, 1.5f);
	p2 = glm::vec3(1.5f, 0.0f,-1.5f);
	p3 = glm::vec3(-1.5f, 0.0f,-1.5f);
	p4 = glm::vec3(-1.5f, 0.0f, 1.5f);
	
	// Push these vertices into the vector set in the object
	box2.vertexSet.push_back(p1);
	box2.vertexSet.push_back(p2);
	box2.vertexSet.push_back(p3);
	box2.vertexSet.push_back(p4);
	
	// Edges making up the polygon
	box2.edgeSet.push_back(p1 - p2);
	box2.edgeSet.push_back(p2 - p3);
	box2.edgeSet.push_back(p3 - p4);
	box2.edgeSet.push_back(p4 - p1);

	//Facenormals of each face
	box2.faceNormalSet.push_back(glm::vec3(0.0f,1.0f,0.0f));
	
	vertices.clear();

	// Push the vertices for each face in counte clockwise manner
	vertices.push_back(VertexFormat(p1, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p2, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p3, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
													  
	vertices.push_back(VertexFormat(p1, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p3, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
	vertices.push_back(VertexFormat(p4, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
												  
	// Push the data to the buffer on the GPU
	box2.base.initBuffer(6, &vertices[0]);

	//Initially, neither of the objects have any rotation, thus their rotation matrix would be an identity matrix
	box1.RotationMat = glm::mat4(1.0f);
	box2.RotationMat = glm::mat4(1.0f);

	box1.linearVelocity = glm::vec3(1.0f, 0.0f, 0.0f);
	box2.linearVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

	box1.linearAcc = glm::vec3(0.0f, 0.0f, 0.0f);
	box2.linearAcc = glm::vec3(0.0f, 0.0f, 0.0f);

	box1.angularAcc = glm::vec3(0.0f, 0.0f, 0.0f);
	box2.angularAcc = glm::vec3(0.0f, 0.0f, 0.0f);

	box1.angularVelocity = glm::vec3(0.0f, 20.0f, 0.0f);
	box2.angularVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

	box1.e = 0.7f;
	box2.e = 1.0f;
	box1.mass = 10.0f;
	box2.mass = 10000.0f;					//Does'nt matter
	box1.inverseMass = 1.0f / box1.mass;	
	box2.inverseMass = 0.0f;				// Since the inverse mass is 0, then the object's mass is infinity (immovable)

	box1.inertiatensor = glm::mat3(1.0f);
	box1.inertiatensor[0][0] = box1.mass * ((0.4f)*(0.4f))/ 6.0f;
	box1.inertiatensor[1][1] = box1.inertiatensor[0][0];
	box1.inertiatensor[2][2] = box1.inertiatensor[0][0];

	box2.inertiatensor = glm::mat3(0.0f);

	box1.Fstatic = 0.20f;
	box1.Fdynamic = 0.18f;

	box2.Fstatic = 0.20f;
	box2.Fdynamic = 0.18;
}


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
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	// Creates a projection matrix using glm::perspective.
	// First parameter is the vertical FoV (Field of View), second paramter is the aspect ratio, 3rd parameter is the near clipping plane, 4th parameter is the far clipping plane.
	proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	PV = proj * view;

	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	MVP = PV* translation;
	box1.MVP = PV * (glm::translate(box1.origin)* box1.RotationMat);
	box2.MVP = PV * (glm::translate(box2.origin)* box2.RotationMat);

	boxInfocus = &box1;
	boxOutfocus = &box2;
	// This gets us a reference to the uniform variable in the vertex shader, which is called "color".
	// We're using this variable to change color during runtime, without changing the buffer values.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniMVP = glGetUniformLocation(program, "MVP");
	color = glGetUniformLocation(program, "blue");

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


glm::vec3 EulerIntegrator(glm::vec3 pos, float h, glm::vec3 &velocity, glm::vec3 acc)
{
	glm::vec3 P;

	velocity += h * acc;

	//Calculate the displacement in that time step with the current velocity.
	P = pos + (h * velocity);

	//return the position P
	return P;
}


glm::mat4 AngularEulerIntegrator(glm::vec3 angularVel, float t)
{ 
	//This function will return theroation matrix that will be multuplied to the current rotation matrix of the object 
	if (glm::length(angularVel) > FLT_EPSILON)
		return glm::rotate((glm::length(angularVel) * t), glm::normalize(angularVel));		// The magnitude of the angular velocity is considered as the angle and the angular vel itself is considered as the axis
	else
	return glm::mat4(1.0f);								// If the angular velocity is zero, the function would return an invalid matrix, to avoid that we will return an identity matrix.
}


void checkbounds(ConvexHull &A)
{
	// This funciton is to check if the objects are within the screen bounds. 
	// There are no collision detection algorithms used here.
	// We just check if the center of the object goes out of bounds, then reverse that component of the velocity
	float bounds = 1.0f;

	if (abs(A.origin.x) > bounds)
		A.linearVelocity.x *= -1.0f;
	if (abs(A.origin.y) > bounds)
		A.linearVelocity.y *= -1.0f;
	if (abs(A.origin.z) > bounds)
		A.linearVelocity.z *= -1.0f;
}


float overlap(float min1, float max1, float min2, float max2)
{//Function to check if the two ranges from min1 to max1, and min2 to max2 overlap.
	if (min1 == max1 || min2 == max2)
	{
		//One of the obejcts is plane.
		if (abs(min1 - min2) > abs(max1 - max2))
			return max1 - max2;
		else
			return min1 - min2;
	}
	else
	{
		// Case 2 : min1 lies on the left of min2
		if (min1 < min2)
		{
			if (max1 >= max2)
				return max2 - min2;			// overlap, return positive

			return max1 - min2;				// If overlap, then return positive else negative
		}

		// Case 3 : min1 lie between min2 and max2
		if (min1 >= min2 && min1 < max2)
		{
			if (max1 <= max2)				// segment1 lies completly inside 2
				return max1 - min1;

			return max2 - min1;
		}

		//Case 1 : min1 lies outside max2
		if (min1 >= max2)
			return max2 - min1;				// no overlap, return negative
	}
}


glm::vec3 getMinMax(glm::vec3 n, ConvexHull &A, float &min, float &max)
{// This function gets the minimum and maximul values of an object projected on the vector n. It also returns the vertex which is projecting the lowest value.
 // This is a redundant operation in this example. Thus, it makes sense in encapsulating it in a function.
	n = glm::normalize(n);
	float temp;
	min = 99.0f;				// Set min to the maximum possible value.
	max = -99.0f;				// Set max to minimum possible value.
	std::vector<glm::vec3>::iterator xy;
	glm::mat4 transform = glm::translate(A.origin) * A.RotationMat;
	glm::vec3 minvector(0.0f,0.0f,0.0f);

	for (xy = A.vertexSet.begin(); xy != A.vertexSet.end(); xy++)
	{
		temp = glm::dot(n, glm::vec3(transform * glm::vec4(*xy, 1.0f)));
		if (min > temp)					// If the current min is greater than the temp value, then replace min with the temp value.
		{
			min = temp;
			minvector = glm::vec3(transform * glm::vec4(*xy, 1.0f));
		}
		if (max < temp)					//If the current max is less than the temp value, then replace max with the temp value.
			max = temp;
	}

	return minvector;					// return the point projecting the minimum value.
}


glm::vec3 getPOCin1D(ConvexHull &A, ConvexHull &B, glm::vec3 n)
{// This function returns the component of a point on a single vector.
	n = glm::normalize(n);				// Normalize the vector. (good practice)
	float min1, min2, max1, max2, o;		
	glm::vec3 POC;

	getMinMax(n, A, min1, max1);		// get min and max of the projection of object A on vector n
	getMinMax(n, B, min2, max2);		// get min and max of the projection of object B on vector n

	o = overlap(min1, max1, min2, max2);	// find the overlap of the two regions. We are sure to get  an over lap at this point.

	if (min1 < min2)						// We need the second to last value. as that is where the overlap starts. 
	{										// Adding half the overlap value gives us the midpoint of the area overlapped.
		POC = n * ( min2 + (o / 2.0f));		// Multiplying that value by the vector gives us conponent of that point.
	}
	else
		POC = n * ( min1 + (o / 2.0f));

	return POC;						
}


glm::vec3 LineCollision(Line &l1, Line &l2)
{//Convert the circle's position from world coordinate system to the box's model cordinate system.
	// We Change the line equation to parametric form.
	/*

	Assume that P1 and P2 are the closest points on line 1 and 2 respectively.
	So,
	P1 = L1(s) = point1 + (s* d1)
	P2 = L2(t) = point1 + (t* d2)

	for some constant s and t.

	the vector joining P1 and P2 is v.
	thus, v(s,t) = L1(s) + L2(t)

	So,
	since v is perpendicular to both L1 and L2.
	d1.v(s,t) = 0
	d2.v(s,t) = 0

	substituting the parametric equation for v(s,t) and expanding them gives us:

	(d1.d1)s - (d1.d2)t = -(d1.r)
	and
	(d2.d1)s - (d2.d2)t = -(d2.r)

	where r = l1.point1 - l2.point1

	this can also be written as :

	|a -b| |s| = |-c|
	|b -c| |t|   |-f|

	where;
	a =d1.d1
	b= d1.d2
	c = d1.r
	e = d2.d2
	f = d2.r
	d = ae - b^2;

	solving this system of equation we get:
	s = (bf -ce)/d
	t = (af -bc)/d

	substituting these values in the quation gives us the closest point between two line segments.
	We can then check if these points lie on the line segment or outside. We then compute the distance
	between these points, and if the distance is less than a specified threshold then a collision is said to have occoured.

	If you want more detailed explanations, check the book Real-time Collision detection by Ericson, page 146.
	*/

	glm::vec3 d1, d2, r;
	d1 = l1.point2 - l1.point1;
	d2 = l2.point2 - l2.point1;
	r = l1.point1 - l2.point1;

	float a = glm::dot(d1, d1);
	float b = glm::dot(d1, d2);
	float c = glm::dot(d1, r);
	float e = glm::dot(d2, d2);
	float f = glm::dot(d2, r);

	float d = (a*e) - (b*b);

	float s = ((b*f) - (c*e)) / d;
	float t = ((a*f) - (b*c)) / d;

	/*
	since its a parametric representation, if the constants s and t are greater than 1, then the point lies along the vectorPQ
	but farther down the line i.e not on the line segment. And if the constants are less than 0, then the point lies along the
	vector -PQ, but after P, so not on the line segment PQ. Thus we can bail at this point knowing no collision occoured.

	collision detection between two Lines whcih extend infinitly will not have this check.
	*/

	//This check is not required as we already know that the lines are colliding.
	//if (((s <= 1.0f) && (s >= 0.0f)) && ((t <= 1.0f) && (t >= 0.0f)))
	{
		glm::vec3 p1, p2;
		p1 = l1.point1 + (s * d1);
		p2 = l2.point1 + (t * d2);

	//This check is not required as we already know that the lines are colliding.
	//	if (glm::distance(p1, p2) <= 0.01f)
		{
			return (p1 + p2) / 2.0f;
		}

	//	return false;
	}
	//else
	//	return false;
}


bool returnMTV(ConvexHull &A, ConvexHull &B, glm::vec3 &mtv, float &o)
{

	mtv = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 n(0.0f,0.0f,0.0f);

	//We only need to maintain the the smallest axis, witch can be MTV, and need to check the axis's overlap
	float minOverlap = 99.0f,current;

	//set the min and max values to highest possible values.
	float min1 = 99.0f, max1= -99.0f,min2 = 99.0f, max2 = -99.0f, temp;

	// the transform matrices for each object
	glm::mat4 transform1, transform2;
	
	//transform matrices consist of translation and rotation matrix of each respective object
	transform1 = glm::translate(A.origin) * A.RotationMat;
	transform2 = glm::translate(B.origin) * B.RotationMat;

	std::vector<glm::vec3>::iterator it, xy,ab;

	// Go through face normals of box1
	for (it = A.faceNormalSet.begin(); it != A.faceNormalSet.end(); it++)
	{
		n = glm::normalize(*it);
		n = glm::vec3(transform1 * glm::vec4(n, 0.0f));
		n = glm::normalize(n);
		
		min1 = 99.0f;
		max1 = -99.0f;
		min2 = 99.0f;
		max2 = -99.0f;

		//find the dot produnct of each vertex in world space on the axis
		getMinMax(n, A, min1, max1);
		
		//find the dot produnct of each vertex in world space on the axis
		getMinMax(n, B, min2, max2);

		if (min1 > max2 || max1 < min2)
			return false;							// If overlap occours in any of the axis, then the objects are not intersecting : early exit

		current = overlap(min1, max1, min2, max2);
		
		if (abs(minOverlap) > abs(current))
		{
			mtv = n;//glm::normalize(n) * current;
			minOverlap = current;
		}

	}

	//For face normals of box2
	for (it = B.faceNormalSet.begin(); it != B.faceNormalSet.end(); it++)
	{
		n = glm::normalize(*it);
		n = glm::vec3(transform2 * glm::vec4(n, 0.0f));
		n = glm::normalize(n);

		min1 = 99.0f;
		max1 = -99.0f;
		min2 = 99.0f;
		max2 = -99.0f;

		//find the dot produnct of each vertex in world space on the axis
		getMinMax(n, A, min1, max1);

		//find the dot produnct of each vertex in world space on the axis
		getMinMax(n, B, min2, max2);

		if (min1 > max2 || max1 < min2)
			return false;													// If overlap occours in any of the axis, then the objects are not intersecting : early exit
		
		current = overlap(min1, max1, min2, max2);

		if (abs(minOverlap) > abs(current))
		{
			mtv = n;//glm::normalize(n) * current;
			minOverlap = current;
		}
	}


	//For edges of (box1 X box2)
	for (it = A.edgeSet.begin(); it != A.edgeSet.end(); it++)
	{
		for (ab = B.edgeSet.begin(); ab != B.edgeSet.end(); ab++)
		{
			n = glm::normalize(*it);
			n = glm::vec3(transform1 * glm::vec4(n, 0.0f));
			n = glm::cross(n, glm::vec3(transform2 * glm::vec4(*ab, 0.0f)));

			if (glm::length(n) > FLT_EPSILON)
			{
				n = glm::normalize(n);
				
				min1 = 99.0f;
				max1 = -99.0f;
				min2 = 99.0f;
				max2 = -99.0f;
				//find the dot produnct of each vertex in world space on the axis
				getMinMax(n, A, min1, max1);

				//find the dot produnct of each vertex in world space on the axis
				getMinMax(n, B, min2, max2);

				if (min1 >= max2 || max1 <= min2)
					return false;														// If overlap occours in any of the axis, then the objects are not intersecting : early exit

				current = overlap(min1, max1, min2, max2);
				
				if (abs(minOverlap) > abs(current))
				{
					mtv = n;// glm::normalize(n) * current;
					minOverlap = current;
				}
			}
		}
	}

	float min = 9999999;
	n = glm::vec3(0.0f, 0.0f, 0.0f);

	// At this point we are sure that the objects are intersecting. 
	// Now we have to find the smallest overlap, and the corresponding axis.
	
	// making sure that the axis points from A to B
	glm::vec3 d = B.origin - A.origin;
	
	if (glm::length(mtv) < FLT_EPSILON)
		return false;

	if (glm::dot(d, mtv) < 0.0f)
		mtv *=-1.0f;

	//We need to account for floating point errors, thus we ensure that any any value under EPSILON is set to 0, 
	//This is done so that the errors do not build up leading to a miscalculation
	mtv.x = (fabs(mtv.x) < 2 * FLT_EPSILON) ? 0.0f : mtv.x;
	mtv.y = (fabs(mtv.y) < 2 * FLT_EPSILON) ? 0.0f : mtv.y;
	mtv.z = (fabs(mtv.z) < 2 * FLT_EPSILON) ? 0.0f : mtv.z; 

	o = abs(minOverlap);
	if (fabs(minOverlap) <= FLT_EPSILON)
		o = 0.0f;

	//return true for collision detection
	return true;
	
}


glm::vec3 getpointOfCollision(ConvexHull &A, ConvexHull &B, glm::vec3 mtv)
{
	/*
	The process we adopted to find the point of collision is to first detect the type of collision.
	We find the number of points lying close to each other and to the other obejct.
	Depending on the number,we can classify it as either,point-face, Edge-Edge or Face-face.
	*/
	glm::vec3 n = glm::normalize(mtv);

	float tolerance = 0.001f;

	int totalSuspects1= 1, totalSuspects2=1 ;

	// the transform matrices for each object
	glm::mat4 transform1, transform2;
	glm::vec3 pointofCollision;

	glm::vec3 closestPointsA[2], ClosestPointsB[2];

	//transform matrices consist of translation and rotation matrix of each respective object
	transform1 = glm::translate(A.origin) * A.RotationMat;
	transform2 = glm::translate(B.origin) * B.RotationMat;
	
	float pointsA, pointsB;
	std::vector<glm::vec3>::iterator it;

	//Set the min to maximum possible value, and max to minimum possible value
	float max = -99.0f, min = 99.0f;
	int i;

	for (it = A.vertexSet.begin(); it != A.vertexSet.end(); it++)
	{
		pointsA = (glm::dot(glm::vec3(transform1*(glm::vec4(*it, 1.0f))), n));

		if (abs(max - pointsA) <= tolerance)
		{
			closestPointsA[1] = *it;			// If any other point lies close to the given point within the tolerance level, increment the total number of suspects. 
			totalSuspects1++;					// update the closest poitns set
		}
		else
		if (max < pointsA)						// If another point is found which is closer then reset the count and store the 2 closest points 
		{
			max = pointsA;
			totalSuspects1 = 1;
			pointofCollision = *it;
			closestPointsA[0] = pointofCollision;
			closestPointsA[1] = pointofCollision;
		}
	}

	//Point to face or edge collision, with point of object A piercing object B
	if (totalSuspects1 == 1)
	{
		return glm::vec3(transform1*(glm::vec4(pointofCollision, 1.0f)));
	}

	for (it = B.vertexSet.begin(); it != B.vertexSet.end(); it++)
	{
		pointsB = (glm::dot(glm::vec3(transform2*(glm::vec4(*it, 1.0f))), n));

		if (abs(min - pointsB) <= tolerance)
		{
			ClosestPointsB[1] = *it;							// If any other point lies close to the given point within the tolerance level, increment the total number of suspects. 
			totalSuspects2++;									// update the closest poitns set
		}
		else
		if (min > pointsB)
		{
			min = pointsB;
			totalSuspects2 = 1;									// If another point is found which is closer then reset the count and store the 2 closest points 
			pointofCollision = *it;
			ClosestPointsB[0] = pointofCollision;
			ClosestPointsB[1] = pointofCollision;
		}
	}

	//Point to face or edge collision, with point of object B piercing object A
	if (totalSuspects2 == 1)
	{
		return glm::vec3(transform2*(glm::vec4(pointofCollision, 1.0f)));
	}

	//If the number of suspects is exactly 2 on both sides, then the collision is edge to edge
	//Edge to edge collision
	if (totalSuspects1 == 2 && totalSuspects2 == 2)
	{	
		Line l1, l2;																		// Create two lines with the values we stored earlier in the previous steps. 
		l1.point1 = glm::vec3(transform1 * glm::vec4(closestPointsA[0], 1.0f));
		l1.point2 = glm::vec3(transform1 * glm::vec4(closestPointsA[1], 1.0f));

		l2.point1 = glm::vec3(transform2 * glm::vec4(ClosestPointsB[0], 1.0f));
		l2.point2 = glm::vec3(transform2 * glm::vec4(ClosestPointsB[1], 1.0f));

		return LineCollision(l1,l2);														// Pass them to deduce the point of collision
	}

	//If we have reached this point, then the face of one of the objects is colliding with the edge or the face of
	//the other object, as the suspects count has to be greater than 2 for both the objects.
	// edge-face collision or face face collision;
	
	//here v1,v2 and v3 are the basis vector we will derive from n, using grahm-schmidt's process
	//v1 = x-axis
	//v2 = y-axis
	//v3 = z-axis
	glm::vec3 v1, v2, v3,startingVec1(1.0f,0.0f,0.0f),startingVec2(0.0f,1.0f,0.0f);

	//In cases where edge collides with the face, we need to ensure that the axis we get from Grahm-Schmidt process
	//should have one the axis beign the edge.
	if (totalSuspects1 == 2)
	{
		startingVec1 = glm::normalize(closestPointsA[0] - closestPointsA[1]);
		startingVec2 = glm::cross(startingVec1, n);
	}
	if (totalSuspects2 == 2)
	{
		startingVec1 = glm::normalize(ClosestPointsB[0] - ClosestPointsB[1]);
		startingVec2 = glm::cross(startingVec1, n);
	}

	// There are several possible to ways to get three axis, and one of them would be a zero vector, we need to ensure that we don't consider that one.
	// There is no possible way for the program to fail both the test for v1 and v2 at the same time.
	v1 = startingVec1 - n * glm::dot(n,startingVec1);																// Subtract the component of the vector n on X-axis to get the first basis vector. 									
	if (glm::length(v1) < 2.0f*FLT_EPSILON)
	{
		v1 = startingVec2 - n * glm::dot(n, startingVec1);
		startingVec2 = glm::vec3(0.0f, 0.0f, 1.0f);
	}
	
	v2 = startingVec2 - n * glm::dot(n, startingVec2) - v1 * n * glm::dot(v1, startingVec2);						// Subtract the component of the vector n on Y axis and v1 on Yaxis to get the second vector
	if (glm::length(v2) < 2.0f * FLT_EPSILON)
	{
		startingVec2 = glm::vec3(0.0f, 0.0f, 1.0f);
		v2 = startingVec2 - n * glm::dot(n, startingVec2) - v1 * n * glm::dot(v1, startingVec2);
	}
	
	//v3 = glm::vec3(0.0f, 0.0f, 1.0f) - n*n.z - (v1* v1.z) - (v2* v2.z);												// subtract the component of the vector n on Z axis and v1 on Z axis and v2 on Zaxis to get the third basis vector
	
	//We only need 2 basis vectors, So we choose 2 of those which are not 0.
	// Since we are dealing with a small system here, FLT_EPSILON is not enough of a error margin, so we multiply it by 2 to increase it.

	//This section of code basically, swaps the three vectors v1,v2,and v3 such that the non-zero vectors are always stored in v1 and v2

	v1 = glm::normalize(v1);
	v2 = glm::normalize(v2);

	glm::vec3 component1, component2,component3, POC;

	//get the component along the two vectors and add it.
	component1 = getPOCin1D(A, B, v1);
	component2 = getPOCin1D(A, B, v2);

	POC = component1 + component2;

	//adding the resultant with the distance of the plane from the origin multiplied by the normal(plane) will give us the position of the point.
	float d = glm::dot(n, (glm::vec3(transform2*(glm::vec4(ClosestPointsB[0], 1.0f)))));

	return POC + d * n;

}


void resolveCollision(ConvexHull &A, ConvexHull &B, glm::vec3 poc,glm::vec3 mtv)
{	
	//To start resolving the collision, we need to first find the velocities at the point of contact on each obejct.
	glm::vec3 n = glm::normalize(mtv);
	glm::vec3 Va, Vb, Vab;
	glm::vec3 Ra, Rb;
	//The radius of the POC from the origin ofthe respective object.
	Ra = poc - A.origin;
	Rb = poc - B.origin;

	Ra.x = (fabs(Ra.x) < FLT_EPSILON) ? 0.0f : Ra.x;
	Ra.y = (fabs(Ra.y) < FLT_EPSILON) ? 0.0f : Ra.y;
	Ra.z = (fabs(Ra.z) < FLT_EPSILON) ? 0.0f : Ra.z;


	Rb.x = (fabs(Rb.x) < FLT_EPSILON) ? 0.0f : Rb.x;
	Rb.y = (fabs(Rb.y) < FLT_EPSILON) ? 0.0f : Rb.y;
	Rb.z = (fabs(Rb.z) < FLT_EPSILON) ? 0.0f : Rb.z;

	//The velocity at teh point of contact can be computed by adding the linear velocity and the component of the angular velocity at that point.
	Va = A.linearVelocity + glm::cross(A.angularVelocity, Ra);
	Vb = B.linearVelocity + glm::cross(B.angularVelocity, Rb);

	//Now we find the linear velocity at the point on object A with respect to B
	Vab = Va - Vb;

	// This is to ensure that the points are moving towards each other, rather than away from each other. This is done to avoid resolving the same collision over and over again.
	float directionIndicator = glm::dot(glm::normalize(Vab), n);
	if (directionIndicator > 0.0f)
	{	
		// Calculate the cumulative elasticity(Coefficient of restitution
		float e =  A.e * B.e;
		
		//Calculate the inertia for the current orientation. We can derive this by multiplying the inertia tensor wit hteh rotation matrix and its transpose as shown below.
		glm::mat3 inertiaA = A.inertiatensor;
		glm::mat3 IA = glm::mat3(0.0f);
		if (A.inverseMass != 0.0f)
		{
			inertiaA = glm::mat3(A.RotationMat) * inertiaA * glm::transpose(glm::mat3(A.RotationMat));
			IA = glm::inverse(inertiaA);
		}
		glm::mat3 inertiaB = B.inertiatensor;
		glm::mat3 IB = glm::mat3(0.0f);
		if (B.inverseMass != 0.0f)
		{
			inertiaB = glm::mat3(B.RotationMat) * inertiaB * glm::transpose(glm::mat3(B.RotationMat));
			IB = glm::inverse(inertiaB);
		}
		
		// Calculate the component of the denominator. We are doing this here because, containing the entire equation in one line would be messy and prone to errors.
		float denomComponent = glm::dot((glm::cross( IA * glm::cross(Ra, n), Ra) + glm::cross(IB *glm::cross(Rb, n), Rb)), n);

		// Caculate the impulse
		float j = (-(1 + e) * glm::dot(Vab, n)) / (A.inverseMass + B.inverseMass + denomComponent);
		
		glm::vec3 Va2, Vb2;

		//The linear velocity resultant would be equal to the initial velocity plus the the impulse in the direction of collision divided by the mass. 
		// Using the inverse mass is a better idea than acually dividing by mass because, this way you can create objects with infinite mass by simply 
		// Creating objects with inverse mass as 0. and this wouldn't break the program. 
		Va2 = A.linearVelocity + j * A.inverseMass * n;
		Vb2 = B.linearVelocity - j * B.inverseMass * n;

		// Calculate the torque generated by the impulse
		glm::vec3 L1 =glm::cross(Ra, n) * j;
		glm::vec3 L2 =glm::cross(Rb, n) * -j;
		
		// Divide the torque by the inertia to get the angular velocity
		glm::vec3 Wa2 = IA * L1;
		glm::vec3 Wb2 = IB * L2;

		//Compensate for floating point error
		Wa2.x = (fabs(Wa2.x) < FLT_EPSILON) ? 0.0f : Wa2.x;
		Wa2.y = (fabs(Wa2.y) < FLT_EPSILON) ? 0.0f : Wa2.y;
		Wa2.z = (fabs(Wa2.z) < FLT_EPSILON) ? 0.0f : Wa2.z;
				 		   
		Wb2.x = (fabs(Wb2.x) < FLT_EPSILON) ? 0.0f : Wb2.x;
		Wb2.y = (fabs(Wb2.y) < FLT_EPSILON) ? 0.0f : Wb2.y;
		Wb2.z = (fabs(Wb2.z) < FLT_EPSILON) ? 0.0f : Wb2.z;


		//Add the angular velocity to the existing velocity to get the final velocities
		A.angularVelocity += Wa2;
		B.angularVelocity += Wb2;

		//Calculate the Friction
		glm::vec3 t;
		
		if (glm::dot(Vab, n) != 0.0f)
			t = glm::normalize(Vab);
		else
			if (glm::dot(G, n) != 0.0f)
				t = glm::normalize(G);
			else
				t = glm::vec3(0.0f);

		t = t - glm::dot(t, n)*n;														// This is the vector against which the frictional force will act.
		
		glm::vec3 jf;
		float js, jd;

		js = (A.Fstatic + B.Fstatic)/2.0f;												// Calculate the total static friction co-efficient
		jd = (A.Fdynamic + B.Fdynamic)/2.0f;											// Calculate the total dynamic friction co-efficient

		if (A.mass*glm::dot(Vab, t) <= abs(j) * js || abs(glm::dot(Vab, t)) <= 0.01f)	// This happens when the object is moving with a velocity which is less than required to overcome 
			jf = A.mass*glm::dot(Vab, t) * -t;
		else																			// Once the object overcomes static friction, the frition becomes a constant. (dynamic friction)
			jf = -jd * t;

		//Calculate the angular Friction
		glm::vec3 relativeAngularVel = B.angularVelocity - A.angularVelocity;			// We use the same logic that we used to calculate the linear friction
		glm::vec3 Rv = n * glm::dot(relativeAngularVel, n);								// Find the relative angular velocity, along the vecor perpendicular to the surface.That is the axis along which the surfaces are spinning

		float Fs = abs(j * js), Fd = abs(j * jd);										// The impulse is the reactive force acting on the objects. This is used to compute the static and dynamic friction.

		glm::vec3 l = inertiaA * Rv;													// calculate the angular momentum which we'll consider as angular impulse

		glm::vec3 Wf(0.0f);																

		if (glm::length(l) <= Fs)														// if the angular momentum is less than that of the static frictional force, then the impule would be same as the momentum, which would negate the current spin
		{
			Wf = l;
		}
		else
		{
			Wf = (Fd < glm::length(Rv)) ? Fd * n : l;									// but if it greater than static value, then the dynamic friction comes into effect.
		}

		Wa2 = IA * Wf;																	// Multiply the impulse with inverse of the inertia tensor with the impulse to get the resultant change in angular velocity
		Wb2 = IB * Wf;

		//Compensate for floating point error
		Wa2.x = (fabs(Wa2.x) < FLT_EPSILON) ? 0.0f : Wa2.x;
		Wa2.y = (fabs(Wa2.y) < FLT_EPSILON) ? 0.0f : Wa2.y;
		Wa2.z = (fabs(Wa2.z) < FLT_EPSILON) ? 0.0f : Wa2.z;

		Wb2.x = (fabs(Wb2.x) < FLT_EPSILON) ? 0.0f : Wb2.x;
		Wb2.y = (fabs(Wb2.y) < FLT_EPSILON) ? 0.0f : Wb2.y;
		Wb2.z = (fabs(Wb2.z) < FLT_EPSILON) ? 0.0f : Wb2.z;

		//Add the angular velocity to the existing velocity to get the final velocities
		A.angularVelocity += Wa2;
		B.angularVelocity += Wb2;

		
		// Since we are using a impulse generating frictional force.
		// The frictional force creates an impulse which acts on the object
		// Then we convert the impulse and add it to the velocity.
		Va2 += jf * A.inverseMass;														
		Vb2 -= jf * B.inverseMass;
		
		A.linearVelocity = Va2;
		B.linearVelocity = Vb2;


	}
}


#pragma region util_functions
// Functions called between every frame. game logic

// This runs once every physics timestep.
void update(float t)
{
	glm::vec3 n;
	float Overlap = 0;

	// Integrate The objects
	box1.origin = EulerIntegrator(box1.origin, t, box1.linearVelocity,G+box1.linearAcc);			// Since gravity also applies on the object, we add the object's acceleration to the gravity
	box2.origin = EulerIntegrator(box2.origin, t, box2.linearVelocity,box2.linearAcc);				// Since the plane does not move, it does not experience gravity
	
	box1.RotationMat = AngularEulerIntegrator(box1.angularVelocity, t) * box1.RotationMat;
	box2.RotationMat = AngularEulerIntegrator(box2.angularVelocity, t) * box2.RotationMat;
	
	// Ensure that the objects are bound inside the screen space. 
	checkbounds(box1);
	checkbounds(box2);

	if (returnMTV(*boxInfocus, *boxOutfocus, n, Overlap))					// Check if the collision occours
	{
		if (boxOutfocus->inverseMass != 0.0f)
			boxOutfocus->origin += n*Overlap;								// if collision occours then move the other object by MTV		(Decoupling)
		else
			boxInfocus->origin -= n * Overlap;								// If one of the object is immovable, then move the other object

		POC = getpointOfCollision(*boxInfocus, *boxOutfocus, n);	// Get the point of contact
		resolveCollision(*boxInfocus, *boxOutfocus, POC, n);		// Resolve the velocitites of the objects						(Resolution)
	}

	//The position of the obejct needs to be changed and sent to the GPU in form of a matrix
	box1.MVP = PV * (glm::translate(box1.origin)* box1.RotationMat);
	box2.MVP = PV * (glm::translate(box2.origin)* box2.RotationMat);
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

	glLineWidth(4.0f);
	//Draw the object1
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(box1.MVP));
	glBindBuffer(GL_ARRAY_BUFFER, box1.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box1.base.numberOfVertices);

	//Draw the object2
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(box2.MVP));
	glBindBuffer(GL_ARRAY_BUFFER, box2.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box2.base.numberOfVertices);

	glm::mat4 specialMVP = PV * glm::translate(POC);
	glEnable(GL_POINT_SMOOTH);
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(specialMVP));
	glPointSize(9.0f);
	glBegin(GL_POINTS);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS))
	{
		box1.origin = glm::vec3(0.0f, 0.0f, 0.0f);
		box1.linearVelocity = glm::vec3(1.5f, 0.0f, 0.0f);
		box1.angularVelocity = glm::vec3(0.0f, 20.0f, 0.0f);
	}

	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		box1.angularVelocity += glm::vec3(0.0f, 1.0f, 0.0f);
	}

}
#pragma endregion


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "3D angular friction", nullptr, nullptr);

	std::cout << "\n This program demonstrates implementation of angular friction \n\n\n\n\n\n\n\n\n\n";
	std::cout << " Press \"SPACE\" to reset the simulation.";
	std::cout << "\n Press \"W\" increase the angular velcity of the box.";

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