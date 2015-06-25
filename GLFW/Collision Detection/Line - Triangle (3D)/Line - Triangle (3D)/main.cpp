/*
Title: Line - Triangle (3D)
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
This is a demonstration of collision detection between a line and a triangle.
The demo contains a line and a triangle. When the objects are not 
colliding the triangle will appear blue and the line will appear green. When the 
two objects collide the triangle will become pink and the line will become yellow. 

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. The shapes
can be rotated by clicking and dragging the left mouse button. You can swap which shape is 
selected with spacebar. 

This algorithm tests for collisions between a line and an triangle by determining if
the point on the line which intersects the plane which the triangle lies in is contained within the triangle.
We are able to do this by observing and comparing the signs of a scalar triple product between vectors going from
the line to the points of the triangle & the direction of the line.

References:
Base by Srinivasan Thiagarajan
Plane - OBB by Nicholas Gallagher
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader variables
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

//Uniform variables
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

//Triangle collider struct
struct Triangle
{
	glm::vec3 a, b, c;

	///
	//Default constructor, constructs basic triangle
	Triangle::Triangle()
	{
		a = glm::vec3(-1.0f, -1.0f, 0.0f);
		b = glm::vec3(1.0f, -1.0f, 0.0f);
		c = glm::vec3(0.0f, 1.0f, 0.0f);
	}


	///
	//Parameterized constructor, constructs triangle pA pB pC
	Triangle::Triangle(glm::vec3 pA, glm::vec3 pB, glm::vec3 pC)
	{
		a = pA;
		b = pB;
		c = pC;
	}
};

//Line collider struct
struct Line
{
	glm::vec3 p1, p2;
	
	///
	//Default constructor, constructs a line collider along the X axis
	Line::Line()
	{
		p1 = glm::vec3(-1.0f, 0.0f, 0.0f);
		p2 = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	///
	//Parameterized constructor, constructs a line collider from a to b
	Line::Line(glm::vec3 a, glm::vec3 b)
	{
		p1 = a;
		p2 = b;
	}
	
};

struct Mesh* triangle;
struct Mesh* line;

struct Mesh* selectedShape;

struct Triangle* triangleCollider;
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

//Reads Shader Source
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

	//Create shader program
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

	glPointSize(3.0f);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a point and a line
//
//Overview:
//	This algorithm tests for collisions between a line and an triangle by determining if
//	the point on the line which intersects the plane which the triangle lies in is contained within the triangle.
//	We are able to do this by observing and comparing the signs of a scalar triple product between vectors going from
//	the line to the points of the triangle & the direction of the line.
//
//Parameters:
//	triCollider: The triangle to test
//	triModelMatrix: The triangle's model to world transformation matrix
//	lCollider: the line to test
//	lineModelMatrix: The lines model matrix
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const struct Triangle &triCollider, const glm::mat4 &triModelMatrix, const struct Line &lCollider, const glm::mat4 &lineModelMatrix)
{
	//We could test if a point P were in a triangle ABC in 2D by making sure that 
	//(Equation 1, 2, 3) n . (AB x AP), n . (BC x BP), and n . (CA x CP) 
	//all have the same sign.
	//where n is the normal of the triangle <0.0f, 0.0f, 1.0f>, 
	//AB, BC, CA, ... each denote the vector from point A to point B, Point B to point C, point C to point A, ...
	// x denotes the cross product, 
	//and . denotes the dot product.

	//The geometric representation of this idea is if the points ABC making up the triangle are in counter clockwise order, if a point P is inside of the triangle
	//the sub-triangles PBC, PCA, PAB will all also be in counter clockwise order!

	//So what we could do, is we could determine where the line collides with the plane, then check that the aforementioned remains true.
	//However, there is an easier way which stems from the use of the scalar triple product in the 2D case.
	
	//If the line l intersects the plane which contains the triangle at some point P,
	//Equation 1, 2, and 3 all have the same sign if and only if P is contained within the triangle.
	//
	//Alternatively, if the line l intersects the plane, and the points P1 and P2 are both on the line (but not necessary in the plane)
	//The line intersects the triangle if and only if:
	//P1P2 . (P1C x P1B), P1P2 . (P1A x P1C), and P1P2 . (P1B x P1A) all have the same sign

	//Step 1: We must convert the points of the triangle collider and the points on the line into worldspace
	glm::vec3 worldA = glm::vec3(triModelMatrix * glm::vec4(triCollider.a, 1.0f));
	glm::vec3 worldB = glm::vec3(triModelMatrix * glm::vec4(triCollider.b, 1.0f));
	glm::vec3 worldC = glm::vec3(triModelMatrix * glm::vec4(triCollider.c, 1.0f));

	glm::vec3 worldP1 = glm::vec3(lineModelMatrix * glm::vec4(lCollider.p1, 1.0f));
	glm::vec3 worldP2 = glm::vec3(lineModelMatrix * glm::vec4(lCollider.p2, 1.0f));

	//Step 2: Now we can compute the direction of the line, because it will be used in each of the equations
	glm::vec3 lineDir = worldP2 - worldP1;

	//Step 3: Perform each of the equations 1, 2, and 3
	float u = glm::dot(lineDir, glm::cross(worldC - worldP1, worldB - worldP1));
	float v = glm::dot(lineDir, glm::cross(worldA - worldP1, worldC - worldP1));
	float w = glm::dot(lineDir, glm::cross(worldB - worldP1, worldA - worldP1));

	//Step 4: Check for the special case where u, v, and w all equal 0
	//This would indicate that the line lies within the plane! This must be handled specially.
	if (fabs(u) - FLT_EPSILON <= 0.0f)		//Special case
	{
		if (fabs(v) - FLT_EPSILON <= 0.0f)
		{
			if (fabs(w) - FLT_EPSILON <= 0.0f)
			{
				//If u, v, and w are all equal to 0, the line lies within the plane.
				//To determine if they collide we must see if all points of the triangle lie on one side of the line
				//If all points of the triangle do lie on one side of the line, then the line and the triangle are not colliding. 
				//If this is not the case, then the line and the triangle are colliding

				//We can do this by inspecting another set of triple scalar products. If:
				//	n . (lineDir X P1A),	n . (lineDir x P1B),	n . (lineDir x P1C)
				//Where n is the normal of the plane they lie in
				//all have the same sign, there is no collision. else, we have a collision!

				//Step a: Determine the normal of the plane
				glm::vec3 worldNormal = glm::cross(worldB - worldA, worldC - worldA);

				//Step b: Test the scalar products for sign likeness
				if (glm::dot(worldNormal, glm::cross(lineDir, worldA - worldP1)) > 0.0f)
				{
					if (glm::dot(worldNormal, glm::cross(lineDir, worldB - worldP1)) > 0.0f)
						if (glm::dot(worldNormal, glm::cross(lineDir, worldC - worldP1)) > 0.0f)
							return false;
				}
				else
				{
					if (glm::dot(worldNormal, glm::cross(lineDir, worldB - worldP1)) < 0.0f)
						if (glm::dot(worldNormal, glm::cross(lineDir, worldC - worldP1)) < 0.0f)
							return false;
				}

				//If any of the signs do not match, collision!
				return true;
			}
		}
	}

	
	//Step 5: Check if they have the same sign!
	if (u < 0.0f)
	{
		if (v < 0.0f)
			if (w < 0.0f)
				return true;
	}
	else
	{
		if (v > 0.0f)
			if (w > 0.0f)
				return true;
			
	}

	return false;
}

// This runs once every physics timestep.
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

		glm::mat4 yaw;
		glm::mat4 pitch;

		//Rotate the selected shape by an angle equal to the mouse movement
		if (deltaMouseX != 0.0f)
		{
			yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
			
		}
		if (deltaMouseY != 0.0f)
		{
			pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
			
		}

		selectedShape->rotation = yaw * pitch * selectedShape->rotation;

		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

	}

	if (TestCollision(*triangleCollider, triangle->GetModelMatrix(), *lineCollider, line->GetModelMatrix()))
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
	triangle->Draw();
	line->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == line ? triangle : line;

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
	window = glfwCreateWindow(800, 800, "Line - Triangle Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();



	//Generate the triangle mesh
	struct Vertex triVerts[3] =
	{
		{ -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
		{ 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f }
	};

	triangle = new struct Mesh(3, triVerts, GL_TRIANGLES);

	//Translate the triangle
	triangle->translation = glm::translate(triangle->translation, glm::vec3(0.15f, 0.0f, 0.0f));

	//Scale the triangle
	triangle->scale = glm::scale(triangle->scale, glm::vec3(0.1f));

	//Rotate the triangle
	triangle->rotation = glm::rotate(triangle->rotation, 60.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	//Generate line mesh
	struct Vertex lineVerts[2] =
	{
		{ -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }
	};
	line = new struct Mesh(2, lineVerts, GL_LINES);

	//Translate the line
	line->translation = glm::translate(line->translation, glm::vec3(-0.15f, 0.0f, 0.0f));

	//scale the line
	line->scale = glm::scale(line->scale, glm::vec3(10.0f));

	//Set the selected shape
	selectedShape = triangle;

	//Generate triangle collider
	triangleCollider = new struct Triangle(
		glm::vec3(triVerts[0].x, triVerts[0].y, triVerts[0].z),
		glm::vec3(triVerts[1].x, triVerts[1].y, triVerts[1].z),
		glm::vec3(triVerts[2].x, triVerts[2].y, triVerts[2].z)
		);

	//Generate line collider
	lineCollider = new struct Line(glm::vec3(lineVerts[0].x, lineVerts[0].y, lineVerts[0].z), glm::vec3(lineVerts[1].x, lineVerts[1].y, lineVerts[1].z));

	//Print controls
	std::cout << "Use WASD to move the selected shape in the XY plane.\nUse left CTRL & left shift to move the selected shape along Z axis.\n";
	std::cout << "Left click and drag the mouse to rotate the selected shape.\nUse spacebar to swap the selected shape.\n";

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

	delete triangle;
	delete line;

	//Delete Colliders
	delete triangleCollider;
	delete lineCollider;

	// Frees up GLFW memory
	glfwTerminate();
}