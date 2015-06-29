/*
Title: Buoyancy Example
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
This is a program showing the implementation of the bouyant force in fluids.
This example has a sphere which represents the mass, and the fluid is represented
by the blue box. The parameters for the fluid like density, and the parameters for
the ball, like mass and size can be changed from the first code block.

Buoyancy is the property of fluid to extert force on an object when submerged in the fuid.
This force is equal to the weight of water displaced by the object. The mass of the water 
displaced can be computed by the volume of the fluid displaced multiplied by the density 
of the fluid. The "weight" can be computed by multiplying the mass and gravity. This gives
us the magnitude of the buoyant force acting on the object(ball).

Buoyant force always acts in the direction opposite to gravity. So, multiplying the magnitude
with the unit vector against the gravitional force gives us the buoyant force. 
Adding this force to the all the other forces acting on the body (in this case gravitational pull)
gives us the behaviour we expect. 

References:
Nicholas Gallagher
AABB-2D by Brockton Roth
*/


#include "GLIncludes.h"

#pragma region program specific Data members
// We change this variable upon detecting collision
float timestep = 0.012f;
//the number of disvision used to make the structure of the circle
int NumberOfDivisions = 20;
//Acceleration and force
glm::vec3 acc,force;

//mass of the object
float mass = 1.5f;
//Density of the water
float density = 1.0f;
#pragma endregion


glm::vec3 AcceleratedVel(glm::vec3 Acc, glm::vec3 velocity, float h)
{
	// Calculate the change in velocity ina given time h, for an acceleration of Acc
	return velocity + (Acc*h);
}

// Euler integrator is necessary for RK4 integrator
glm::vec3 EulerIntegrator(glm::vec3 pos, float h, glm::vec3 &velocity, glm::vec3 Acceleration)
{
	glm::vec3 P;

	//Calculate the displacement in that time step with the current velocity.
	P = pos + (h * velocity);

	//calculate the velocity at the end of the timestep.
	velocity = AcceleratedVel(Acceleration, velocity, h);
	
	//return the position P
	return P;
}

// RK4 Integrator.
glm::vec3 RK4Intergrator(glm::vec3 pos, float h, glm::vec3 &velocity, glm::vec3 &Acceleration)
{
	/*
	K1 is the increment based on the slope at the beginning of the interval, uing y (euler's method)
	k2 is the increment based on the slope at the midpoint of the interval, using y + (h/2)k1
	k3 is the increment based on the slope at the midpoint of the interval, using y + (h/2)k2
	k4 is the increment based on the slope at the end of the interval, using y + h*k3

	k1-------------k2-----------------k3----------------k4
	|<--------------------- T -------------------------->|
	*/

	// Se t current position as pos
	glm::vec3 P = pos;

	glm::vec3 k1, k2, k3, k4, vel;
	glm::vec3 k;
	vel = velocity;

	k = EulerIntegrator(pos, 0.0f, vel, Acceleration);

	k1 = vel;

	//vel = velocity;

	k = EulerIntegrator(pos + (h * k1 / 2.0f), h / 2.0f, vel, Acceleration);
	k2 = vel;

	k = EulerIntegrator(pos + (h * k2 / 2.0f), h / 2.0f, vel, Acceleration);
	k3 = vel;

	k = EulerIntegrator(pos + (h * k3), h, vel, Acceleration);
	k4 = vel;

	//Use the velocity at the mid point to compute the displacement during the timestep h
	P += h * (k1 + (2.0f * k2) + (2.0f * k3) + k4) / 6.0f;

	//Change the velocity to the value at the end of the timestep.
	//EulerIntegrator(pos, h, velocity, Acceleration);
	velocity =  AcceleratedVel(Acceleration, velocity, h);
	 Acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

	//Return the poisiotn P
	return P;
}

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

// The basic structure for a Circle. We need a center, a radius, velocity, acceleration, mass, and VBO and total number of vertices.
struct Circle{
	glm::vec3 origin;
	float radius;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	stuff_for_drawing base;
	float mass;
	glm::mat4 MVP;
}circle;

// Just a square box to show where the fluid starts
struct Box {
	glm::vec3 origin;
	float length;
	float breadth;
	stuff_for_drawing base;
}water;

//This function sets up the two shapes we need for this example.
void setup()
{
	//Set up the global variables.
	acc = glm::vec3(0.0f, 0.0f, 0.0f);
	force = glm::vec3(0.0f, 0.0f, 0.0f);
	
	//Setting up the Circle
	/*
	A large sphere with a smaller mass is more likely to float than a small sphere with the same mass as it displaces a larger amount of fluid
	and has a lesser weight.
	*/
	circle.origin = glm::vec3(0.0f, 0.1f, 0.2f);
	circle.velocity = glm::vec3(0.0f, 0.0f,0.0f);
	circle.radius = 0.25f;
	circle.mass = mass * 0.1f;
	circle.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	float radius = circle.radius;

	VertexFormat center(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	std::vector<VertexFormat> vertices;

	float theta = 360.0f / NumberOfDivisions;

	//Circle vertex generation
	//In this example we are not implementing the proper the code for indices. We are just going to produce redundant information in the buffer.
	//since we are only having a small number of objects on the screen currently, this redundancy should not matter.
	for (int i = 0; i < NumberOfDivisions; i++)
	{
		//In every iteration, the center, the point at angle theta and at angle (theta+delta) are fed into the buffer.
		vertices.push_back(center);
		vertices.push_back(VertexFormat(glm::vec3(radius * cos(glm::radians(i*theta)), radius * sin(glm::radians(i*theta)), 0.0f), glm::vec4(0.7f, 0.20f, 0.0f, 1.0f)));
		vertices.push_back(VertexFormat(glm::vec3(radius * cos(glm::radians((i + 1)*theta)), radius * sin(glm::radians((i + 1)*theta)), 0.0f), glm::vec4(0.7f, 0.20f, 0.0f, 1.0f)));
	}

	circle.base.initBuffer(NumberOfDivisions * 3, &vertices[0]);

	//Set up the waterbody
	// This is water body is supposed to extend to infinity, but fo the purpose of this demonstration it will extend from -10 to 10
	vertices.clear();
	water.origin = glm::vec3(0.0f, 0.5f, 0.0f);
	water.length = 10.0f;
	water.breadth = 20.0f;

	vertices.push_back(VertexFormat(glm::vec3(-water.breadth, 0.0f, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(-water.breadth, -water.length, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(water.breadth, 0.0f, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f)));
	
	vertices.push_back(VertexFormat(glm::vec3(water.breadth, 0.0f, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(-water.breadth, -water.length, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f)));
	vertices.push_back(VertexFormat(glm::vec3(water.breadth, -water.length, 0.0f), glm::vec4(0.5f, 0.5f, 0.9f, 1.0f)));

	water.base.initBuffer(6, &vertices[0]);
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
	circle.MVP = MVP;
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
	//Gravity acts in the negative y direction, and is of the magnitude 9.8N
	glm::vec3 gravity(0.0f, -9.8f, 0.0f);
	// The weight of the object, i.e the gravitational pull expereienced by the object
	glm::vec3 forceApplied = circle.mass * gravity;

	// If the object is in contact with the fluid, it expereinces bouyant  force.
	// In this example, since we are using static fluid set at a specific level, this level 
	// of check will suffice, but in more complex scenarios would require more complex checks.
	if (circle.origin.y - circle.radius < 0.0f)
	{	
		// calculate the the immersed depth i.e. the part of the object that is immersed in the fluid. 
		// This will be used to to calculate the immersed volume.
		float immersedDepth = circle.origin.y - circle.radius;
		//once the object is completly immersed, it will only expereince the same amount of buoyant force 
		//irrespective of the depth of the object. Because the amount of  water displaced does not vary with depth.
		if (immersedDepth >(circle.radius * 2.0f)) immersedDepth = circle.radius * 2.0f; // if the depth is more than the diameter then clamp it at the diameter value.

		//calculate the volume immersed in the liquid. This is equal to the volume of the fluid displaced.
		// This formula calculates the volume of the cap of the sphere.
		// V = PI x H^2 x ( 3r - H) / 3 
		// where H is the immersed depth and r is the radius of the sphere.
		float immersedVolume = PI * immersedDepth * immersedDepth * (3 * circle.radius - immersedDepth) / 3.0f;
		//calculating the weight of the fluid displaced, immersed volume x density of the fluid x gravitational force
		float WeightofWaterDisplaced = immersedVolume * density * 9.8f;

		//Converting the scalar bouyant force to a vector.
		glm::vec3 bouyantForce(0.0f, WeightofWaterDisplaced, 0.0f);
		
		//adding to the total amount of foces expereinced by the object
		forceApplied += bouyantForce;

		// the velocity is multiplied by 0.9999 to act as damping force. This is supposed to compensate 
		//for the damping caused during the motion in a denser medium.
		circle.velocity *= 0.9999f;
	}
	
	if (circle.origin.y - circle.radius <= -1.0f && forceApplied.y < 0)
	{
		forceApplied = glm::vec3(0.0f, 0.0f, 0.0f);
		circle.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	forceApplied *= 0.001f;

	circle.acceleration = forceApplied / circle.mass;

	// Integrate the position of the object using the integrator.
	circle.origin = RK4Intergrator(circle.origin, timestep, circle.velocity, circle.acceleration);

	//The position of the obejct needs to be changed and sent to the GPU in form of a matrix
	glm::mat4 translation = glm::translate(glm::vec3(circle.origin));
	circle.MVP = PV * translation;
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Draw the circle
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(circle.MVP));
	glBindBuffer(GL_ARRAY_BUFFER, circle.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, circle.base.numberOfVertices);

	//Draw the waterbody
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
	glBindBuffer(GL_ARRAY_BUFFER, water.base.vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	glDrawArrays(GL_TRIANGLES, 0, water.base.numberOfVertices);
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	////This set of controls are used to move one point (point1) of the line.
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS))
	{
		circle.acceleration = glm::vec3(0.0f,0.0f,0.0f);
		circle.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		circle.origin = glm::vec3(0.0f, 0.5f, 0.2f);
	}
}
#pragma endregion


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Buoyancy Sample", nullptr, nullptr);

	std::cout << "\n This program demonstrates the implementation of buoyant force in Games\n\n\n\n\n\n\n\n\n\n";
	std::cout << "\n Press \" Space \" to reset the simulation.";
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