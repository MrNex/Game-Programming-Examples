/*
Title: Point - Triangle (Barycentric Method)
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
This is a demonstration of collision detection between a point and a triangle.
The demo contains a triangle. When the mouse is not detected as colliding with
the triangle, the triangle  will appear green. When the mouse is detected as colliding 
with the triangle, the triangle will become yellow. 

The triangle is able to be moved. you can translate the triangle in the XY plane with WASD.
The triangle can be rotated by with the Q and E keys. 

This algorithm tests for collisions between a point and a triangle by utilizing
a barycentric coordinate system. First, we transform the system into worldspace, then
translate the objects such that their new coordinate system has an origin on the center of the
triangle. From here we determine two scalars, t and s, which represent scales of the triangles sides
we must travel to reach the point. If 0.0f <= t + s <= 1.0f  then the point is contained
within the triangle.

This is a more mathematical approach to the solution. For a more geometric approach,
please see Point - Triangle (Normal Method).

References:
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

//An Triangle Collider struct
struct Triangle
{
	glm::vec2 a, b, c;

	///
	//Default constructor construct a basic triangle
	Triangle::Triangle()
	{
		a = glm::vec2(-1.0f, -1.0f);
		b = glm::vec2(1.0f, -1.0f);
		c = glm::vec2(0.0f, 1.0f);
	}

	///
	//Parameterized constructor, constructs a triangle from 3 points
	Triangle::Triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
	{
		a = p1;
		b = p2;
		c = p3;
	}
};

struct Mesh* triangle;

struct Triangle* triangleCollider;

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

	//Generate shader program
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

	//SEt options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	//Bigger points
	glPointSize(3.0f);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a point and a triangle
//
//Overview:
//	This algorithm tests for collisions between a point and a triangle by utilizing 
//	a barycentric coordinate system. First, we transform the system into worldspace, then
//	translate the objects such that their new coordinate system has an origin on the center of the
//	triangle. From here we determine two scalars, t and s, which represent scales of the triangles sides
//	we must travel to reach the point. If 0.0f <= t + s <= 1.0f  then the point is contained
//	within the triangle.
//
//Parameters:
//	triCollider: The triangle collider to test
//	triTranslation: The triangle's translation transformation matrix
//		(Tip: We just need the position of the box in worldspace, so feel free to just use a vec3
//			  if it suits your implementation better.)
//	triRotation: the triangle's rotation transformation matrix
//	triScale: The triangle's scale transformation matrix
//	point: The point in worldspace
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const Triangle &triCollider, const glm::mat4 &triTranslation, const glm::mat4 &triRotation, const glm::mat4 &triScale, glm::vec2 point)
{
	//Step 1: Get the points of both the triangle collider and the point in a space which has it's origin on the center of the triangle
	glm::mat4 orient = triRotation * triScale;
	//Rotate & scale the points of the triangle
	glm::vec2 worldA = glm::vec2(orient * glm::vec4(triCollider.a, 0.0f, 1.0f));
	glm::vec2 worldB = glm::vec2(orient * glm::vec4(triCollider.b, 0.0f, 1.0f));
	glm::vec2 worldC = glm::vec2(orient * glm::vec4(triCollider.c, 0.0f, 1.0f));

	//Translate the point into a coordinate system centered on the triangle
	point.x -= triTranslation[3][0];
	point.y -= triTranslation[3][1];
	
	//Step 2: Get 2 vectors which which represent 2 edges of the triangle
	glm::vec2 aToC = worldC - worldA;
	glm::vec2 aToB = worldB - worldA;

	//Step 3: Determine if aToC or aToB have an x component which is 0, this will help us decide which formula we must use (to avoid dividing by 0).
	//	Note: It would be impossible for both of them to have an x component which is 0, or this is not a triangle!

	if (fabs(aToC.x) <= FLT_EPSILON)
	{
		//We will use aToB as the base for our formula
		//Using barycentric coordinate systems we can express the point as such:
		//	point = worldA + t * aToC + s * aToB
		//
		//If we solve that system of equations and have values between 0.0f and 1.0f for
		//both t and s, then the point is contained within our triangle. If this is not the case,
		//The point is not within our triangle!
		//
		//We must divide the formula into one part for each dimension to get a system of equations, starting with:
		//	point.x = worldA.x + t * aToC.x + s * aToB.x
		//
		//Because we know aToB does not have a 0 in the x component, we will first solve for s.
		//	s = (point.x - worldA.x - t * aToC.x) / aToB.x
		//
		//Next we can substitute this into the system of equations in the Y dimension:
		//	point.y = worldA.y + t * aToC.y + s * aToB.y
		//Getting:
		//	point.y = worldA.y + t * aToC.y + (aToB.y / aToB.x) * (point.x - worldA.x - t * aToC.x)
		//
		//We will represent aToB.y / aToB.x by "quot"
		float quot = aToB.y / aToB.x;

		//Leaving us with:
		//	point.y = worldA.y + t * aToC.y + quot * (point.x - worldA.x - t * aToC.x)
		//
		//Now we must solve for t:
		//	point.y = worldA.y + t * aToC.y + quot * point.x - quot * worldA.x - t * quot * aToC.x
		//	point.y - worldA.y - quot * point.x + quot * worldA.x = t * aToC.y - t * quot * aToC.x
		//	t * (aToC.y - quot * aToC.x) = point.y - worldA.y - quot * point.x + quot * worldA.x
		//	t = (point.y - worldA.y - quot * point.x + quot * worldA.x) / (aToC.y - quot * aToC.x)
		float t = (point.y - worldA.y - quot * point.x + quot * worldA.x) / (aToC.y - quot * aToC.x);

		printf("t:\t%f\n", t);

		//If t is not between 0 and 1 there is no way we have a collision, we must only continue to check s if t is within this range
		if (0.0f <= t && t <= 1.0f)
		{
			//Now we can backtrack to the equation:
			//	s = (point.x - worldA.x - t * aToC.x) / aToB.x
			//and solve for t!
			float s = (point.x - worldA.x - t * aToC.x) / aToB.x;

			printf("s:\t%f\n", s);

			//If s + t is between 0 and 1, we have a collision!
			if (0.0f <= s && s <= 1.0f - t) return true;
		}
	}
	else
	{
		//We will use aToC as the base for our formula
		//Using barycentric coordinate systems we can express the point as such:
		//	point = worldA + t * aToC + s * aToB
		//
		//If we solve that system of equations and have values between 0.0f and 1.0f for
		//both t and s, then the point is contained within our triangle. If this is not the case,
		//The point is not within our triangle!
		//
		//We must divide the formula into one part for each dimension to get a system of equations, starting with:
		//	point.x = worldA.x + t * aToC.x + s * aToB.x
		//
		//Because we know aToC does not have a 0 in the x component, we will first solve for t.
		//	t = (point.x - worldA.x - s * aToB.x) / aToC.x
		//
		//Next we can substitute this into the system of equations in the Y dimension:
		//	point.y = worldA.y + t * aToC.y + s * aToB.y
		//getting:
		//	point.y = worldA.y + (aToC.y / aToC.x) * (point.x - worldA.x - s * aToB.x) + s * aToB.y
		//
		//Now we can solve for s:
		//	s = (point.y - worldA.y - (aToC.y / aToC.x) * point.x + (aToC.y / aToC.x) * worldA.x) / (aToB.y - (aToC.y / aToC.x) * aToB.x)
		//
		//We can simplify this to:
		//	s = (point.y - worldA.y - quot * point.x + quot * worldA.x) / (aToB.y - quot * aToB.x)
		//where quot is the quotient:	(aToC.y / aToC.x)
		float quot = aToC.y / aToC.x;
		float s = (point.y - worldA.y - quot * point.x + quot * worldA.x) / (aToB.y - quot * aToB.x);
		
		//If s is not between 0 and 1 there is no way we have a collision, we must only continue to check t if s is within this range
		if (0.0f <= s && s <= 1.0f)
		{
			//Now we can backtrack to the equation:
			//	t = (point.x - worldA.x - s * aToB.x) / aToC.x
			//and solve for t!
			float t = (point.x - worldA.x - s * aToB.x) / aToC.x;

			//If t + s is between 0 and 1, we have a collision!
			if (0.0f <= t && t <= 1.0f - s) return true;

		}
		
	}

	return false;
	
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

	if (TestCollision(*triangleCollider, triangle->translation, triangle->rotation, triangle->scale, mousePos))
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
}


// This function is used to handle key inputs.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{


		//This set of controls are used to move the selectedShape.
		if (key == GLFW_KEY_W)
			triangle->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, movementSpeed, 0.0f)) * triangle->translation;
		if (key == GLFW_KEY_A)
			triangle->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-movementSpeed, 0.0f, 0.0f)) * triangle->translation;
		if (key == GLFW_KEY_S)
			triangle->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -movementSpeed, 0.0f)) * triangle->translation;
		if (key == GLFW_KEY_D)
			triangle->translation = glm::translate(glm::mat4(1.0f), glm::vec3(movementSpeed, 0.0f, 0.0f)) * triangle->translation;

		//This set of controls is used to rotate the selected shape.
		if (key == GLFW_KEY_Q)
			triangle->rotation = glm::rotate(triangle->rotation, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
		if (key == GLFW_KEY_E)
			triangle->rotation = glm::rotate(triangle->rotation, -rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));

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
	window = glfwCreateWindow(800, 800, "Point - Triangle (2D - Barycentric) Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();



	//Generate the triangle mesh
	struct Vertex triangleVerts[3] = 
	{
		{ -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }
	};

	

	triangle = new struct Mesh(3, triangleVerts, GL_TRIANGLES);

	//Translate the triangle
	triangle->translation = glm::translate(triangle->translation, glm::vec3(0.0f, 0.0f, -5.0f));

	//Scale the triangle
	triangle->scale = glm::scale(triangle->scale, glm::vec3(0.1f));


	//Generate triangle collider
	triangleCollider = new struct Triangle(
		glm::vec2(triangleVerts[0].x, triangleVerts[0].y),
		glm::vec2(triangleVerts[1].x, triangleVerts[1].y),
		glm::vec2(triangleVerts[2].x, triangleVerts[2].y)
		);

	//Print controls
	std::cout << "Controls:\nMove the mouse to detect collisions between triangle and mouse position.\nUse WASD to move the triangle.\nUse Q and E to rotate the triangle.\n";

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

	delete triangle;

	//Delete Colliders
	delete triangleCollider;

	// Frees up GLFW memory
	glfwTerminate();
}