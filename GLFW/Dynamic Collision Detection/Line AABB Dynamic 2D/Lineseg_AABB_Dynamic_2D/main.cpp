/*
Title: Line Segment - AABB Dynamic 2D collision Detection
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
This example demonstrates the collision between a stationary line and a 
axis aligned bounding box. We use the same approach we used in determining 
the collision between a line and an AABB.
First we find out if the rectangle before and after the collision lies
completly on one side of the line.
second, we check if both the before and after timeStep the box lies on the
same Side of the triangle. If it does, then it is not colliding.

We determine the relative position of the rectangle with respect to line, 
using dot products of the 4 points to the normal of the line. If all the dot
products are of same sign then, then all 4 points lie on the same side.

Use "left Shift" to toggle the intergration mode from automatic to manual. 
Use "space" to move ahead by 1 timestep.

References:
Real time collision Detection by Ericson
AABB-2D by Brockton Roth
*/


#include "GLIncludes.h"

// We change this variable upon detecting collision
float blue = 0.0f;
float timestep = 0.5f;
bool isSpacePressed = false;
//Tochange the integration from automatic to manual
bool manual = false;

glm::mat4 MVP1, MVP2;

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

// The basic structure for a Circle. We need a center, a radius, and VBO and total number of vertices.
struct Line{
	glm::mat4 MVP;
	glm::vec2 point1;
	glm::vec2 point2;
	glm::vec2 velocity;
	stuff_for_drawing base;
}line;


/*

	A---------------------------B
	|							|
	|							|
	|							|
	C---------------------------D

*/
// The basic structure for a rectangle, you only need length and breadth, we are also maintaining the positions of th e4 vertices.
struct AABB{
	glm::vec2 origin;
	glm::vec2 velocity;
	float length;
	float breadth;
	glm::vec2 A, B, C, D;
	stuff_for_drawing base;
	glm::mat4 MVP;
}box;

// This function return the value between min and mx with the least distance value to x. This is called clamping.
float clamp_on_range(float x, float min, float max)
{
	if (x < min)
		return min;
	if (x > max)
		return max;

	return x;
}


float line_rectangle_collide(Line &l, AABB &r)
{
	glm::vec3 N = glm::rotateZ(glm::vec3(l.point2 - l.point1, 0.0f), (float)glm::radians(90.0f));
	//Get the normal to the line 
	glm::vec2 n(N.x, N.y);

	float dp1, dp2, dp3, dp4;

	//find the dot product of all 4 points on that normal.
	glm::vec2 c1, c2, c3, c4;
	c1 = r.A - l.point1;
	c2 = r.B - l.point1;
	c3 = r.C - l.point1;
	c4 = r.D - l.point1;

	dp1 = glm::dot(n, c1);
	dp2 = glm::dot(n, c2);
	dp3 = glm::dot(n, c3);
	dp4 = glm::dot(n, c4);
	
	//If all the 4 dot products have the same signs, then the object is not colliding with the line
	if ((dp1 * dp2 <= 0) || (dp2 * dp3) <= 0 || (dp3 * dp4) <= 0)
	{
		return 0;		// Collision detected
	}
	else
	{
		return dp1;
	}
}


//Convert the circle's position from world coordinate system to the box's model cordinate system.
bool is_colliding(Line &l, AABB &a)
{
	float d1, d2;
	//Check on which side of the line does the box lie
	d1 = line_rectangle_collide(l, a);
	//If the box is already intersecting the line, then return true
	if (d1 == 0)
		return true;								// early exit

	AABB b = a;
	//Take the relative velocity of the box with respect to the line
	glm::vec2 v = a.velocity - l.velocity;

	//Move the boxahead by one timestep
	b.A += timestep * v;
	b.B += timestep * v;
	b.C += timestep * v;
	b.D += timestep * v;

	// Check on which side of the line does the box lie on. 
	d2 = line_rectangle_collide(l, b);
	//If the resultant position is colliding with line, then return true 
	if (d2 == 0)									//Early exit
		return true;

	//Check if both the before poistion and the after poistion lie on the same side,
	//If they do, then no collision is detected, else collision has occoured, and the
	//box has tunneled through the line
	
	if (d1 * d2 > 0)
	{
		return false;
	}

	return true;
}


//This function sets up the two shapes we need for this example.
void setup()
{
	//setting up the Line or atleast the two points the line passe through 
	line.point1 = glm::vec2(0.0f, 0.5f);
	line.point2 = glm::vec2(0.0f, -0.5f);
	line.velocity = glm::vec2(0.0f, 0.0f);

	//Setting up the AABB
	box.origin = glm::vec2(0.0f, 0.0f);
	box.breadth = 0.05f;
	box.length = 0.2f;
	box.velocity = glm::vec2(0.001f, 0.0f);

	box.A = box.origin + glm::vec2(-box.breadth / 2, box.length / 2);
	box.B = box.origin + glm::vec2(box.breadth / 2, box.length / 2);
	box.C = box.origin + glm::vec2(-box.breadth / 2, -box.length / 2);
	box.D = box.origin + glm::vec2(box.breadth / 2, -box.length / 2);
	
	std::vector<VertexFormat> vertices;
	vertices.push_back(VertexFormat(glm::vec3(box.A, 0.0f), glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(box.C, 0.0f), glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(box.B, 0.0f), glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(box.C, 0.0f), glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(box.D, 0.0f), glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(box.B, 0.0f), glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)));

	box.base.initBuffer(6, &vertices[0]);
}


// Global data members
#pragma region Global Data member
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

// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion			  

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
	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Creates a projection matrix using glm::perspective.
	// First parameter is the vertical FoV (Field of View), second paramter is the aspect ratio, 3rd parameter is the near clipping plane, 4th parameter is the far clipping plane.
	proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	PV = proj * view;

	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	MVP = PV* translation;
	MVP1 = MVP;

	view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//Create another position and perspective for camera, and compute the MVP matrix for that position.
	//Since we'll be just interchanging between two positions of the camera, there is no point in computing the MVP every frame. We will do that later while implementing camera controls.
	MVP2 = (proj * view) * translation;
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

#pragma endregion

// Functions called between every frame. game logic
#pragma region util_functions
// This runs once every physics timestep.
void update()
{
	//Check if the intergration is controlled manually or automatically.
	if (manual)
	{
		//Move only if there is no collision or penetration is detected in the next timestep
		if (isSpacePressed)
		{
			isSpacePressed = false;
			if (is_colliding(line, box))
			{
				//If colliding, Change the color
				blue = 1.0f;
			}
			else
			{
				//If not colliding, intergrateor move ahead by 1 timestep
				blue = 0.0f;
				box.origin += box.velocity * timestep;
				box.A += box.velocity * timestep;
				box.B += box.velocity * timestep;
				box.C += box.velocity * timestep;
				box.D += box.velocity * timestep;

				if (box.A.x > 1.0f)
				{
					box.origin.x -= 3.0f;
					box.A.x -= 3.0f;
					box.B.x -= 3.0f;
					box.C.x -= 3.0f;
					box.D.x -= 3.0f;
				}

				glm::mat4 translation;

				translation = glm::translate(glm::vec3(box.origin, 0.0f));

				box.MVP = PV * translation;
			}
		}
	}
	else
	{
		//If the integration is automatic, intergrate continously and change color only when collision is detected.
		if (is_colliding(line, box))
		{
			blue = 1.0f;
		}
		else
		{
			blue = 0.0f;
		}
		box.origin += box.velocity * timestep;
		box.A += box.velocity * timestep;
		box.B += box.velocity * timestep;
		box.C += box.velocity * timestep;
		box.D += box.velocity * timestep;

		if (box.A.x > 1.0f)
		{
			box.origin.x -= 3.0f;
			box.A.x -= 3.0f;
			box.B.x -= 3.0f;
			box.C.x -= 3.0f;
			box.D.x -= 3.0f;
		}

		glm::mat4 translation;

		translation = glm::translate(glm::vec3(box.origin, 0.0f));

		box.MVP = PV * translation;
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0 -blue, 1.0-blue, 1.0-blue, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	// Set the uniform matrix in our shader to our MVP matrix for the first object.
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
	glLineWidth(2.5f);
	glUniform3f(color, 0.0f,0.0f,blue);
	glBegin(GL_LINES);
	glVertex3fv((float*) &line.point1);
	glVertex3fv((float*) &line.point2);
	glEnd();

	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(box.MVP));
	glBindBuffer(GL_ARRAY_BUFFER, box.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, box.base.numberOfVertices);
	

}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	float moverate = 0.25f;

	////This set of controls are used to move one point (point1) of the line.
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
		isSpacePressed = true;

	if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if (manual)
		{
			//If the integration is occuring automatically, then the velocity needs to be reduced to get a smooth motion, as the CPU integrates at a much faster rate
			manual = false;
			box.velocity = glm::vec2(0.001f, 0.0f);
		}
		else
		{
			//Since we are moving timestep by timestep, 
			manual = true;
			box.velocity = glm::vec2(0.3f, 0.0f);
		}
	}
}

#pragma endregion


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Line segment AABB Dynamic 2D", nullptr, nullptr);

	std::cout << "\n This example demonstrates the collision between a stationary line and a \n axis aligned bounding box.We use the same approach we used in determining \n the collision between a line and an AABB.\n\n\n\n\n\n\n";
	std::cout << "Use \"left Shift\" to toggle the intergration mode from automatic to manual.\nUse \"space\" to move ahead by 1 timestep";

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
}