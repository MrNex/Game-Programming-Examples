/*
Title: Line Segment - Triangle (3D)
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
This is a demonstration of collision detection between a line segment and a triangle.
The demo contains a line and a triangle. When the objects are not 
colliding the triangle will appear blue and the line will appear green. When the 
two objects collide the triangle will become pink and the line will become yellow. 

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. The shapes
can be rotated by clicking and dragging the left mouse button. You can swap which shape is 
selected with spacebar. 

This algorithm tests for collisions between a line segment and a triangle by determining if
the line segment intersects the plane on which the triangle lies, and if it does the
algorithm attempts to determine a point on the line which contains barycentric coordinates
u, v, and w such that 1 >= v + w >= 0 and u = 1 - v - w.

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection - Christer Ericson
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
//Tests if a line is colliding with the edge of a triangle using the parametric equation of lines
//
//Parameters:
//	P1: One endpoint of the line
//	P2; other endpoint of the line
//	E1: One endpoint of the edge
//	E2: Another endpoint of the edge
bool CheckEdge(glm::vec3 P1, glm::vec3 P2, glm::vec3 E1, glm::vec3 E2)
{
	glm::vec3 lineDir = P2 - P1;
	glm::vec3 edgeDir = E2 - E1;

	//Using the parametric equation for a line we have:
	//P1 + t * lineDir = A + s * edgeDir
	//And reducing this to a system of equations:
	//P1.x + t * lineDir.x = A.x + s * edgeDir.x
	//p1.y + t * lineDir.y = A.y + s * edgeDir.y
	//p1.z + t * lineDir.z = A.z + s * edgeDir.z
	//
	//From here we can easily solve for t and s using any system, however we must be careful not to divide by 0!

	if (lineDir.x != 0.0f)
	{
		//We will use the first equation to solve for t
		//t = (A.x + s * AB.x - P1.x) / lineDir.x
		//Next we can substitute t into one of the other equations, we will use the second equation
		//P1.y + (lineDir.y / lineDir.x) * (A.x + s * AB.x - P1.x) = A.y + s * AB.y
		float quot = lineDir.y / lineDir.x;

		//Finally, we solve for s
		//P1.y + quot * A.x + s * quot * AB.x - quot * P1.x = A.y + s * AB.y
		//P1.y - A.y + quot * A.x - quot * P1.x = s * AB.y - s * quot * AB.x
		//s * (AB.y - quot * AB.x) = P1.y - A.y + quot * A.x - quot * P1.x
		//s = (P1.y - A.y + quot * A.x - quot * P1.x) / (AB.y - quot * AB.x)
		float s = (P1.y - E1.y + quot * E1.x - quot * P1.x) / (edgeDir.y - quot * edgeDir.x);

		//Solve for t using t = (A.x + s * AB.x - P1.x) / lineDir.x
		float t = (E1.x + s * edgeDir.x - P1.x) / lineDir.x;

		//If these give us a point that is within the segments, then there is a collision, if not we must check the other edges
		if (0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f) return true;
	}
	else if (lineDir.y != 0.0f)
	{
		//We will use the second equation to solve for t
		//t = (A.y + s * AB.y - P1.y) / lineDir.y
		//Next we can substitute t into one of the other equations, we will use the first because we know lineDir.x is 0
		//P1.x + (lineDir.x / lineDir.y) * (A.y + s * AB.y - P1.y) = A.x + s * AB.x
		//P1.x = A.x + s * AB.x
		//s = (P1.x - A.x) / AB.x
		float s = (P1.x - E1.x) / edgeDir.x;

		//Solve for t using t = (A.y + s * AB.y - P1.y) / lineDir.y
		float t = (E1.y + s * edgeDir.y - P1.y) / lineDir.y;

		//If these give us a point that is within the segments, then there is a collision, if not we must check the other edges
		if (0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f) return true;
	}
	else
	{
		//We will use the third equation to solve for t
		//t = (A.z + s * AB.z - P1.z) / lineDir.z
		//Next we can substitute t into one of the other equations, we will use the first because we know lineDir.x is 0
		//P1.x + (lineDir.x / lineDir.z) * (A.z + s * AB.z - P1.z) = A.x + s * AB.x
		//P1.x = A.x + s * AB.x
		//s = (P1.x - A.x) / AB.x
		float s = (P1.x - E1.x) / edgeDir.x;

		//Solve for t using t = (A.z + s * AB.z - P1.z) / lineDir.z
		float t = (E1.z + s * edgeDir.z - P1.z) / lineDir.z;

		//If these give us a point that is within the segments, then there is a collision, if not we must check the other edges
		if (0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f) return true;
	}

	return false;
}

///
//Tests for collisions between a triangle and a line segment
//
//Overview:
//	This algorithm tests for collisions between a line segment and a triangle (ABC) by determining if
//	the line segment intersects the plane on which the triangle lies, and if it does the
//	algorithm attempts to determine a point on the line which contains barycentric coordinates
//	u, v, and w such that 1 >= v + w >= 0 and u = 1 - v - w.
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
	//Step 1: transform the triangle points and line points in world space
	glm::vec3 transformedA = glm::vec3(triModelMatrix * glm::vec4(triCollider.a, 1.0f));
	glm::vec3 transformedB = glm::vec3(triModelMatrix * glm::vec4(triCollider.b, 1.0f));
	glm::vec3 transformedC = glm::vec3(triModelMatrix * glm::vec4(triCollider.c, 1.0f));

	glm::vec3 transformedP1 = glm::vec3(lineModelMatrix * glm::vec4(lCollider.p1, 1.0f));
	glm::vec3 transformedP2 = glm::vec3(lineModelMatrix * glm::vec4(lCollider.p2, 1.0f));

	//Step 2: Transform all points into a space where the origin is on the center of the triangle
	glm::vec3 trianglePos(triModelMatrix[3][0], triModelMatrix[3][1], triModelMatrix[3][2]);
	transformedA -= trianglePos;
	transformedB -= trianglePos;
	transformedC -= trianglePos;

	transformedP1 -= trianglePos;
	transformedP2 -= trianglePos;


	//Step 3: Compute the normal of the plane which the triangle lies in
	//Find triangle's edges
	glm::vec3 AB = transformedB - transformedA;	//Edge from point A to point B
	glm::vec3 AC = transformedC - transformedA;	//Edge from point A to point C


	glm::vec3 normal = glm::cross(AB, AC);

	//Step 4: Check if both points on the line are on the same side of the plane, if so there cannot be a collision!
	float p1Result = glm::dot(normal, transformedP1);
	float p2Result = glm::dot(normal, transformedP2);

	if ((p1Result < -FLT_EPSILON && p2Result < -FLT_EPSILON) || (p1Result > FLT_EPSILON && p2Result > FLT_EPSILON))
	{
		//Both points are on the same side, no collision!
		return false;
	}
	

	//Step 5: Compute the line direction & determine if the line is parallel to the plane
	glm::vec3 lineDir = transformedP2 - transformedP1;

	if (fabs(glm::dot(lineDir, normal)) <= FLT_EPSILON)
	{
		//Special case, line is in the plane of the triangle!
		//Here we must check if the line segment intersects any of the edges of the triangle.
		if (CheckEdge(transformedP1, transformedP2, transformedA, transformedB)) return true;
		if (CheckEdge(transformedP1, transformedP2, transformedB, transformedC)) return true;
		if (CheckEdge(transformedP1, transformedP2, transformedC, transformedA)) return true;
		return false;
	}

	//Step 6: Test whether the line intersects the plane by using the parametric equation of a line and the definition of a plane
	//First, if the line segment is parallel to the plane we have a special case

	//Parametric equation of a line:
	//	X = P1 + t * (P2 - P1)
	//Where P1 is the starting point on the line segment, P2 is the ending point, and 0.0f <= t <= 1.0f
	//X is the resulting point on the line segment.
	//
	//Equation of a plane:
	//	normal . X = d
	//Where  normal is the normal to the plane, X is a resulting point on the plane, and d is a constant which holds true for all points on the plane
	//Because our plane is at the origin of the new coordinate system, d will be 0.
	//
	//Substituting X, we have:
	//	normal . (P1 + t * (P2 - P1)) = 0 = normal . (P1 + t * lineDir)
	//If we distribute the dot product:
	//	normal . P1 + t * (normal . lineDir) = 0
	//And solving for t:
	//	t = -(normal . P1) / (normal . lineDir)
	float t = -glm::dot(normal, transformedP1) / glm::dot(normal, lineDir);

	//If t is not between 0 and 1, there is no collision.
	if (t < 0.0f || t > 1.0f) return false;

	//Step 7: Determine the barycentric coordinates of the point on the line at parametric time t within the triangle
	//As shown in the Point - Triangle (Barycentric Method) we stated that the barycentric coordinates can be found by
	//	transformedA + v * BA + w * CA = X
	//Where X is the point, and v and w are two out of the three barycentric coordinate. The third coordinate, u, can be solved:
	//	u = 1 - v - w
	//
	//Now, if we substitute X for our line equation:
	//	A + v * AB + w * AC = P1 + t * lineDir
	//But after some re-arranging:
	//	t * -lineDir + v * AB + w * AC = P1 - A
	//However, this can be represented by the matrix equation:
	//	[-lineDir, AB, AC] * <t, v, w> = (P1 - A)
	//
	//There is a theorem in linear algebra known as cramers rule, which states that when you have a system of equations A*x = b
	//where A is a nxn matrix and x and b are vectors of n dimension, you can solve for an individual component of x using the following theorem:
	//1) Find the determinant of A = detA
	//2) Substitute the vector b into the column of a corresponding to the index of x you wish to solve = A# (i.e. A1 is A with b substituted into column 1)
	//3) Find the determinent of A# = detA#
	//4) x[#] = detA# / detA 
	//		Where x[#] indicates the # component of the vector x
	//
	//There is another handy linear algebra identity which states that the determinent of a matrix is equal to the scalar triple product of the columns of A
	//That is:
	//	detA = A[1] . (A[2] x A[3])
	//where A[#] indicates the # column of A, . denotes the dot product, and x denotes the cross product.

	//So first, we will solve for the determinent of A = [-lineDir, AB, AC]
	float detA = glm::dot(-lineDir, glm::cross(AB, AC));
	//Now, we already have a value for t, we only need to solve for v and w! These correspond to the 2nd and 3rd indices of x in our system A*x=b, respectively.
	//So lets determine detA2 and detA3
	//We will need the vector P1 - A to solve these
	glm::vec3 AP = transformedP1 - transformedA;

	float detA2 = glm::dot(-lineDir, glm::cross(AP, AC));
	float detA3 = glm::dot(-lineDir, glm::cross(AB, AP));

	//Now we know for the intersection to be true v + w must fall between 0.0f and 1.0f
	//However, v = detA2 / detA and w = detA3 / detA

	float v = detA2 / detA;
	if (v < 0.0f || v > 1.0f) return false;
	float w = detA3 / detA;
	if (w < 0.0f || v + w > 1.0f) return false;
	
	return true;
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
	window = glfwCreateWindow(800, 800, "Line Segment - Triangle Collision Detection", nullptr, nullptr);
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
	line->scale = glm::scale(line->scale, glm::vec3(0.2f));

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

	delete triangle;
	delete line;

	delete triangleCollider;
	delete lineCollider;

	// Frees up GLFW memory
	glfwTerminate();
}