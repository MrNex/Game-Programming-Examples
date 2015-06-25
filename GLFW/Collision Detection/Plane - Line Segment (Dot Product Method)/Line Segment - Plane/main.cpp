/*
Title: Line Segment - Plane
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
This is a demonstration of collision detection between a line segement and a Plane. 
The demo contains a line and a plane. The plane is colored blue
And the line is colored green until the two collide, then the plane will change to the color
pink and the line will change to the color yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD
and along the Z axis with Left Shift & Left Control. You can also rotate the selected shape by clicking
and dragging the mouse.

This demo detects collisions by making sure the endpoints of the line lie on the same side of the plane.
We do this by translating the endpoints of the box into world space and the normal of the plane
into world space, then translating the entire system to to be centered on the origin of the plane.
From here, you are able to determine what side of the plane the endpoints fall on by observing the sign of dot
product of the endpoint's position vectors with the plane's normal. If one sign doesn't match the other,
there is a collision.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
2D Game Collision Detection by Thomas Schwarzl
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
	glm::mat4 scale;
	glm::mat4 rotation;
	int numVertices;
	struct Vertex* vertices;
	GLenum primitive;

	Mesh::Mesh(int numVert, struct Vertex* vert, GLenum primType)
	{
		this->translation = glm::mat4(1.0f);
		this->scale = glm::mat4(1.0f);
		this->rotation = glm::mat4(1.0f);

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

	void Mesh::Draw(void)
	{
		//Generate the model matrix
		glm::mat4 modelMatrix = this->translation * this->rotation * this->scale;

		//GEnerate the MVP for this model
		glm::mat4 MVP = VP * modelMatrix;

		//Bind the VAO being drawn
		glBindVertexArray(this->VAO);

		// Set the uniform matrix in our shader to our MVP matrix for this mesh.
		glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
		//Draw the mesh
		glDrawArrays(this->primitive, 0, this->numVertices);

	}

};

//Collider for line
struct Line
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;

	///
	//Generates a Line collider from -1 to 1 on the X axis
	Line::Line()
	{
		startPoint = glm::vec3(-1.0f, 0.0f, 0.0f);
		endPoint = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	///
	//Generates a Line collider from a given start and end point
	Line::Line(const glm::vec3 &start, const glm::vec3 &end)
	{
		startPoint = start;
		endPoint = end;
	}
};

struct Plane
{
	glm::vec3 normal;

	///
	//Generates a plane with a normal pointing down the X axis
	Plane::Plane()
	{
		normal = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	///
	//Generates a plane with a given normal
	Plane::Plane(glm::vec3 norm)
	{
		normal = norm;
	}
};

struct Mesh* plane;
struct Mesh* line;

struct Mesh* selectedShape;

struct Plane* planeCollider;
struct Line* lineCollider;


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

//Read shader source
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

// Create a shader from source
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

	// Create shader program
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
	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);
	VP = proj * view;

	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a plane and a line.
//
//Overview:
//	This algorithm detects collisions between a plane and a line segment by testing whether the two end points
//	of the line are on the same side of the plane. We do this by translating the end points of the box into world 
//	space and the normal of the plane into world space, then translating the entire system to to be centered on 
//	the origin of the plane. From here, you are able to determine what side of the plane the end points fall on 
//	by observing the dot product of the corner's position vectors with the plane's normal. If one sign doesn't 
//	match the other, there is a collision.
//
//Parameters:
//	lCollider: The line collider being tested for collision
//	lTrans: The line's translation transformation matrix
//	lRotation: The line's rotation transformation matrix
//	lScale: The line's scale transformation matrix
//
//	(Tip: The above 3 matrices are just going to be used to compute the model to world matrix. If you are
//			storing one model to world matrix instead of the 3 transformation matrices you can send just that one instead of these three!)
//
//	pCollider: The plane collider being tested for collision
//	pTrans: The plane's translation transformation matrix
//	pRotation: The plane's rotation transformation matrix
//
//	(Tip: In this case the scale transformation wouldn't make a difference if you applied it or not, just like the other case
//			if you are storing one model to world matrix instead of separate transformation matrices you can send that one in
//			for the same effect as we will be using these three for.)
//
//	(Note: While in the case of needing a yes or no answer for "Am I colliding" it would not matter whether or not you applied
//			The scale transformation to the plane's normal during the conversion to world space-- However if you wanted information
//			about the collision such as how far am I penetrating the plane you would need to either not apply the scale transformation
//			or re-normalize the normal afterwards.)
//Returns:
//	True if a collision is detected,
//	else false
bool TestCollision(
	const Line &lCollider, const glm::mat4 &lTrans, const glm::mat4 &lRotation, const glm::mat4 &lScale,
	const Plane &pCollider, const glm::mat4 &pTrans, const glm::mat4 &pRotation)
{

	//Step 1: Generate a transformation such that the endpoints of the line will be translated to a system which has it's center
	//		at the origin aft the plane.
	glm::mat4 transform = glm::translate(lTrans, glm::vec3(-pTrans[3][0], -pTrans[3][1], -pTrans[3][2])) * lRotation * lScale;

	//Step 2: Then we must transform the end points of the line & the normal of the plane into this new "plane space"
	glm::vec4 worldStart = transform * glm::vec4(lCollider.startPoint, 1.0f);
	glm::vec4 worldEnd = transform * glm::vec4(lCollider.endPoint, 1.0f);

	glm::vec4 worldNormal = pRotation * glm::vec4(pCollider.normal, 0.0f);	//Make sure the 4th component of the normal is 0.0f!
																			//A normal is a direction, not a point in space.



	//Step 3: observe the sign of the dot product (scalar projection) of the end points in "plane-space" with the plane normal.
	//	If they differ in sign, we have a collision, else there is no collision
	//	alternatively, if any dot product returns 0, the point is on the plane and there is a collision!

	//Note: With colliders this small it is very possible for the rounding errors of floating point numbers to cause error in the collision detection method.
	//			Because of this we must accept certain cases as collisions which fall within a small error range called epsilon.
	//			While this will sometimes cause cases to register as collisions even when they are not (though they will be almost imperceptibly off)
	//			It will make sure we never have a case that is a collision escape from being detected.
	float epsilon = 0.0001f;

	float dotProd = glm::dot(worldStart, worldNormal);
	if (dotProd < 0.0f)
	{
		if (glm::dot(worldEnd, worldNormal) + epsilon >= 0.0f) 
			return true;
	}
	else if (dotProd > 0.0f)
	{
		if (glm::dot(worldEnd, worldNormal) - epsilon <= 0.0f) 
			return true;
	}
	else     //If dotProd == 0.0f
	{
		return true;		
	}

	return false;
}

///
//Adjusts rotations, checks for collision, and adjusts colors
void update()
{
	//Check if the mouse button is being pressed
	if (isMousePressed)
	{
		//Get the current mouse position
		double currentMouseX, currentMouseY;
		glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

		//Get the difference in mouse position from last frame
		float deltaMouseX = (float)(currentMouseX - prevMouseX);
		float deltaMouseY = (float)(currentMouseY - prevMouseY);

		//Rotate the selected shape by an angle equal to the mouse movement
		if (deltaMouseX != 0.0f)
		{
			glm::mat4 yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
			selectedShape->rotation = selectedShape->rotation * yaw;
		}
		if (deltaMouseY != 0.0f)
		{
			glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * -rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
			selectedShape->rotation = pitch * selectedShape->rotation;
		}
		
		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

	}

	if (TestCollision(*lineCollider, line->translation, line->rotation, line->scale, *planeCollider, plane->translation, plane->rotation))
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
	line->Draw();
	plane->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == plane ? line : plane;

		//This set of controls are used to move the selectedShape.
		if (key == GLFW_KEY_W)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, movementSpeed, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_A)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-movementSpeed, 0.0f, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_S)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -movementSpeed, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_D)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(movementSpeed, 0.0f, 0.0f)) * selectedShape->translation;
		if (key == GLFW_KEY_LEFT_CONTROL)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, movementSpeed)) * selectedShape->translation;
		if (key == GLFW_KEY_LEFT_SHIFT)
			selectedShape->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -movementSpeed)) * selectedShape->translation;
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
	window = glfwCreateWindow(800, 800, "Line Segment - Plane Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate line vertices
	struct Vertex lineVerts[2];
	lineVerts[0] = { -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Left Corner
	lineVerts[1] = { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Left Corner

	line = new struct Mesh(2, lineVerts, GL_LINES);

	//Scale the box
	line->scale = line->scale * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));

	//Translate the box
	line->translation = glm::translate(line->translation, glm::vec3(-0.5f, 0.0f, 0.0f));


	//Generate the Plane mesh
	struct Vertex planeVerts[6];
	planeVerts[0] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[1] = { 0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[2] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[3] = { 0.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[4] = { 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	planeVerts[5] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };

	plane = new struct Mesh(6, planeVerts, GL_TRIANGLES);

	//Scale the plane
	plane->scale = plane->scale * glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));

	//Translate the plane
	plane->translation = glm::translate(plane->translation, glm::vec3(0.1f, 0.0f, 0.0f));

	//Set the selected shape
	selectedShape = plane;

	//Generate the colliders
	lineCollider = new struct Line(glm::vec3(lineVerts[0].x, lineVerts[0].y, lineVerts[0].z), glm::vec3(lineVerts[1].x, lineVerts[1].y , lineVerts[1].z));

	//Get two edges of the plane and take the cross product for the normal (Or just hardcode it, for example we know the normal to this plane
	//Will be the Z axis, because the plane mesh lies in the XY Plane to start.
	glm::vec3 edge1(planeVerts[0].x - planeVerts[1].x, planeVerts[0].y - planeVerts[1].y, planeVerts[0].z - planeVerts[1].z);
	glm::vec3 edge2(planeVerts[1].x - planeVerts[2].x, planeVerts[1].y - planeVerts[2].y, planeVerts[1].z - planeVerts[2].z);

	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
	
	planeCollider = new struct Plane(normal);

	//Print controls
	std::cout << "Controls:\nUse WASD to move the selected shape in the XY plane.\nUse left shift & left CTRL to move selected shape along Z axis.\n";
	std::cout << "Left click and drag the mouse to rotate the selected shape.\nUse spacebar to swap the selected shape.";

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

	delete line;
	delete plane;

	delete lineCollider;
	delete planeCollider;

	// Frees up GLFW memory
	glfwTerminate();
}