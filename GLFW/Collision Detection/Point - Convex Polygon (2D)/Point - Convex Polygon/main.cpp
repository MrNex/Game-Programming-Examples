/*
Title: Point - Convex Polygon
File Name: main.cpp
Copyright © 2015
Original authors: Nicholas Gallagher
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
This is a demonstration of collision detection between a point and a polygon.
The demo contains a polygon wireframe. When the mouse is not detected as colliding with
the polygon, the polygon  will appear green. When the mouse is detected as colliding 
with the polygon, the polygon will become yellow. 

The polygon is able to be moved. you can translate the polygon in the XY plane with WASD.
The polygon can be rotated by with the Q and E keys. 

This algorithm tests for collisions between a point and a polygon by first sub-sectioning the polygon
into a triangle fan, then determining which portion of the fan the point lies in. At this point there is
chance that the point does not lie in any portion of the fan, in which case there is no collision. If that
is not the case, we then consider the directed edge (counter clockwise in relation to the rest of the vertices)
spanning the far end of the triangular segment and determine if the point lies to the left or right of this edge.
If the point lies to the left, it must fall within our polygon. If the point lies to the right, then it must not
be within the polygon.

References:
Real Time Collision Detection by Christer Ericson
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;
//Uniforms
GLuint uniMVP;
GLuint uniHue;
glm::mat4 VP;
glm::mat4 hue;
// Reference to the window object being created by GLFW.
GLFWwindow* window;

struct Vertex
{
	float
		x, y, z,
		r, g, b, a;
};

//Struct for rendering
struct Mesh
{
	GLuint VBO;
	GLuint VAO;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	int numVertices;
	struct Vertex* vertices;
	GLenum primitive;

	Mesh::Mesh(int numVert, struct Vertex* vert, GLenum primType)
	{

		glm::mat4 translation = glm::mat4(1.0f);
		glm::mat4 rotation = glm::mat4(1.0f);
		glm::mat4 scale = glm::mat4(1.0f);

		this->numVertices = numVert;
		this->vertices = new struct Vertex[this->numVertices];
		memcpy(this->vertices, vert, this->numVertices * sizeof(struct Vertex));

		this->primitive = primType;

		//Generate VAO
		glGenVertexArrays(1, &this->VAO);
		//bind VAO
		glBindVertexArray(VAO);

		//Generate VBO
		glGenBuffers(1, &this->VBO);

		//Configure VBO
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * this->numVertices, this->vertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)12);
	}

	Mesh::~Mesh(void)
	{
		delete[] this->vertices;
		glDeleteVertexArrays(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);
	}

	glm::mat4 Mesh::GetModelMatrix()
	{
		return translation * rotation * scale;
	}

	void Mesh::Draw(void)
	{
		//GEnerate the MVP for this model
		glm::mat4 MVP = VP * this->GetModelMatrix();

		//Bind the VAO being drawn
		glBindVertexArray(this->VAO);

		// Set the uniform matrix in our shader to our MVP matrix for this mesh.
		glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
		//Draw the mesh
		glDrawArrays(this->primitive, 0, this->numVertices);

	}

};

//An polygon collider struct
struct Polygon
{
	//The points which make up this polygon in a counter clockwise order
	std::vector<glm::vec2> vertices;

	///
	//Default constructor construct a basic triangle
	Polygon::Polygon()
	{
		vertices.push_back(glm::vec2(-1.0f, -1.0f));
		vertices.push_back(glm::vec2(1.0f, -1.0f));
		vertices.push_back(glm::vec2(0.0f, 1.0f));
	}

	///
	//Parameterized constructor, constructs a polygon from a vector of points
	Polygon::Polygon(std::vector<glm::vec2> points)
	{
		vertices = points;
	}

	///
	//Parameterized constructor, construct a polygon from a mesh
	Polygon::Polygon(const Mesh &m)
	{
		for (int i = 0; i < m.numVertices; i++)
		{
			this->vertices.push_back(glm::vec2(m.vertices[i].x, m.vertices[i].y));
		}
	}
};

struct Mesh* polygon;

struct Polygon* polygonCollider;

float movementSpeed = 0.02f;
float rotationSpeed = 0.01f;

bool isMousePressed = false;
double prevMouseX = 0.0f;
double prevMouseY = 0.0f;

//Out of order Function declarations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, int button, int action, int mods);

#pragma endregion Base_data								  

// Functions called only once every time the program is executed.
#pragma region Helper_functions

//Reads shader source
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	std::ifstream file(fileName, std::ios::in);
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;
		return "";
	}

	file.seekg(0, std::ios::end);
	shaderCode.resize((unsigned int)file.tellg());
	file.seekg(0, std::ios::beg);

	file.read(&shaderCode[0], shaderCode.size());
	file.close(); 

	return shaderCode;
}

//Creates shader from source
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str();
	const int shader_code_size = sourceCode.size();

	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		glDeleteShader(shader); 
	}
	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	//Generate the View Projection matrix
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

	VP = proj * view;
	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set Options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	glPointSize(3.0f);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a point and a polygon specified by a counter clockwise set of points
//
//Overview:
//	This algorithm tests for collisions between a point and a polygon by first sub-sectioning the polygon
//	into a triangle fan, then determining which portion of the fan the point lies in. At this point there is
//	chance that the point does not lie in any portion of the fan, in which case there is no collision. If that
//	is not the case, we then consider the directed edge (counter clockwise in relation to the rest of the vertices) 
//	spanning the far end of the triangular segment and determine if the point lies to the left or right of this edge.
//	If the point lies to the left, it must fall within our polygon. If the point lies to the right, then it must not
//	be within the polygon.
//
//Parameters:
//	polyCollider: The polygon collider to test
//	triModelMatrix: The model-to-world matrix of the triangle.
//	point: The point in worldspace
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const Polygon &polyCollider, const glm::mat4 &polyModelMatrix, glm::vec2 point)
{
	//VERTICES MUST BE COUNTER CLOCKWISE!

	//Consider the line from the 0th vertex of the collider to the n/2th vertex of the collider,
	//assuming n is the number of vertices the collider has. We can test if the point is on the left or right of
	//this line. Once we know, if it is on the left we only test the vertices after n/2. If it is on the right
	//we test the vertices before n/2.

	//Each time we test, when the point is on the left of the line we are testing we set the new lower bound to the 
	//vertex which is the other end point of the line (n/2 to start- because the first line we test goes from the 0th vertex to the n/2 vertex). 
	//When the point is on the right of the line we are testing we set the new upper bound to the vertex at the end point of the line.

	//The next vertex we always test is the vertex between our lower and upper bound, where the lower and upper bound start at 0 and n respectively.
	//As this test repeats we will find two lines going from the 0th vertex to two others which the point P falls between.

	//Step 1: Set the lower and upper bound to start
	int lowerBound = 0;
	int upperBound = polyCollider.vertices.size();


	//Step 2: Begin testing to arrive at the two lines the point falls between
	//We must remember to transform the vertex we are testing into world space each time we use it.
	//We will be using vertex at index 0 a lot, so we will transform it into world space before we begin the loop.
	glm::vec2 baseVertex = glm::vec2(polyModelMatrix * glm::vec4(polyCollider.vertices[0], 0.0f, 1.0f));
	while (lowerBound < upperBound - 1)		//Test until the lower and upper bounds are next to each other
	{
		//Step 3: Compute the index of the vertex we are currently testing
		int testIndex = (upperBound + lowerBound) / 2;

		//Step 4: Determine if the point falls on the right or left side of the vector from vertices[0] to vertices[testIndex]
		//We can do this by taking the Vector from vertices[0] to vertices[testingIndex] and the vector from vertices[0] to point
		//and taking their cross product. if the cross product is <0.0f, 0.0f, 1.0f> the point falls on the left, because
		//crossing two vectors where the operands are going counter clockwise gives a positive number. Crossing two vectors where operands are going
		//clockwise gives a negative number, so we would expect <0.0f, 0.0f, -1.0f> if the point fell on the right!

		//Get the test vertex in world space
		glm::vec2 testVertex = glm::vec2(polyModelMatrix * glm::vec4(polyCollider.vertices[testIndex], 0.0f, 1.0f));

		//Construct the vectors
		glm::vec3 vToTest = glm::vec3(testVertex - baseVertex, 0.0f);
		glm::vec3 vToP = glm::vec3(point - baseVertex, 0.0f);

		glm::vec3 normal = glm::cross(vToTest, vToP);

		if (normal.z > 0.0f)	//If left side
			lowerBound = testIndex;
		else                    //Else right side
			upperBound = testIndex;
	}

	//If the lower bound or the upper bound hasn't changed it would indicate that the point is...
	//To the left of the entire left half of the shape
	//or
	//To the right of the entire right half of the shape
	//In which case it is clear there is no collision.
	if (lowerBound == 0 || upperBound == polyCollider.vertices.size()) return false;

	//Step 5: Once we finish testing, we do one final test.
	//Consider the polygon edge from vertices[lowerBound] to vertices[upperBound]
	//If we perform the same test using the cross product, and the point p falls to the left of the edge, then it must be inside of the polygon
	glm::vec2 lowVertex = glm::vec2(polyModelMatrix * glm::vec4(polyCollider.vertices[lowerBound], 0.0f, 1.0f));
	glm::vec2 upVertex = glm::vec2(polyModelMatrix * glm::vec4(polyCollider.vertices[upperBound], 0.0f, 1.0f));

	glm::vec3 lowerToHigher = glm::vec3(upVertex - lowVertex, 0.0f);
	glm::vec3 lowerToPoint = glm::vec3(point - lowVertex, 0.0f);

	return glm::cross(lowerToHigher, lowerToPoint).z > 0.0f;
	
}

// This runs once every physics timestep.
void update()
{

	//Get the current mouse position
	double currentMouseX, currentMouseY;
	glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

	//Translate to normalized device coordinates
	//Since the cursor posiiton is represented in pixel values, 
	//dividing it by the window dimension will reduce the position to 0.0 - 1.0 range.
	//multiplying it with 2 and then subtracting it with 1 wil bring the 
	//mouse position to -1 ot 1.
	glm::vec2 mousePos(
		(((float)currentMouseX / 800.0f) * 2.0f) - 1.0f,
		1.0f - (((float)currentMouseY / 800.0f) * 2.0f)
		);

	if (TestCollision(*polygonCollider, polygon->GetModelMatrix(), mousePos))
	{
		//Turn red on
		hue[0][0] = 1.0f;
	}
	else
	{
		//Turn red off
		hue[0][0] = 0.0f;
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Draw the Gameobjects
	polygon->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{


		//This set of controls are used to move the selectedShape.
		if (key == GLFW_KEY_W)
			polygon->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, movementSpeed, 0.0f)) * polygon->translation;
		if (key == GLFW_KEY_A)
			polygon->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-movementSpeed, 0.0f, 0.0f)) * polygon->translation;
		if (key == GLFW_KEY_S)
			polygon->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -movementSpeed, 0.0f)) * polygon->translation;
		if (key == GLFW_KEY_D)
			polygon->translation = glm::translate(glm::mat4(1.0f), glm::vec3(movementSpeed, 0.0f, 0.0f)) * polygon->translation;

		//This set of controls is used to rotate the selected shape.
		if (key == GLFW_KEY_Q)
			polygon->rotation = glm::rotate(polygon->rotation, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
		if (key == GLFW_KEY_E)
			polygon->rotation = glm::rotate(polygon->rotation, -rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));

	}

}

///
//Inturrupt triggered by mouse buttons
//
//Parameters:
//	window: The window which recieved the mouse click event
//	button: The mouse button which was pressed
//	action: GLFW_PRESS or GLFW_RELEASE
//	mods: The modifier keys which were pressed during the mouse click event
void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	//Set the boolean indicating whether or not the mouse is pressed
	isMousePressed = button == GLFW_MOUSE_BUTTON_LEFT ?
		(action == GLFW_PRESS ? true : false)
		: false;

	//Update the previous mouse position
	glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
}

#pragma endregion util_Functions


void main()
{
	glfwInit();

	// Creates a window
	window = glfwCreateWindow(800, 800, "Point - Convex Polygon (2D) Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();



	//Generate the polygon mesh
	struct Vertex polygonVerts[4] = 
	{
		{ 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }
	};

	

	polygon = new struct Mesh(4, polygonVerts, GL_LINE_LOOP);

	//Scale the polygon
	polygon->scale = glm::scale(polygon->scale, glm::vec3(0.1f));


	//Generate polygon collider
	polygonCollider = new struct Polygon(*polygon);

	//Print controls
	std::cout << "Controls:\nMove the mouse to have collisions detected between the polygon and the mouse position.\nUse WASD to move the polygon in the XY plane.\n";
	std::cout << "Use Q and E to rotate the polygon.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	delete polygon;

	//Delete Colliders
	delete polygonCollider;

	// Frees up GLFW memory
	glfwTerminate();
}