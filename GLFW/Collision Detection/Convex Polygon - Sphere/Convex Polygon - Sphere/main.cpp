/*
Title: Convex Polygon - Sphere
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
This is a demonstration of collision detection between a polygon and a Sphere.
The demo contains a polygon and a wireframe of a sphere. When the objects are not 
colliding the sphere will appear blue and the polygon will appear green. When the 
two objects collide the sphere will become pink and the polygon will become yellow. 

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. By clicking the 
left mouse button and dragging the mouse you will rotate the selected shape. You can also
swap which shape is selected with spacebar. 

This algorithm tests for collisions between a sphere and a convex polygon through three different tests.
First we perform a halfspace test to determine if the sphere completely lies on one side of the polygon.
Then, if this is not the case, we project the sphere's center onto the plane in which the polygon lies &
determine if the projected center falls inside of the polygon. If it does we have a collision, if not however,
we must perform one more test: Find the closest point on every edge to the sphere & determine if that point
falls inside of the sphere using a distance test.

References:
Base by Srinivasan Thiagarajan
Sphere Collision 3D by Srinivasan Thiagarajan
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

//An Sphere Collider struct
struct Sphere
{
	float radius;

	///
	//Default constructor creating an AABB of unit
	//Width, height, and depth (-1.0f to 1.0f on each axis)
	Sphere::Sphere()
	{
		radius = 1.0f;
	}

	///
	//Parameterized constructor creating an AABB of specified
	//width, height, and depth
	Sphere::Sphere(float r)
	{
		radius = r;
	}
};

//A polygon collider struct
struct Polygon
{
	std::vector<glm::vec3> points;	//Assuming sequential order
	
	//Default constructor, creates basic triangle-collider
	Polygon::Polygon()
	{
		points.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
		points.push_back(glm::vec3( 1.0f, -1.0f, 0.0f));
		points.push_back(glm::vec3( 0.0f,  1.0f, 0.0f));
	}

	//Parameterized constructor, creates a polygon collider from a set of points
	Polygon::Polygon(std::vector<glm::vec3> pts)
	{
		points = pts;
	}
};

//Meshes which are drawn
struct Mesh* sphere;
struct Mesh* polygon;

//The mesh currently being transformed by input
struct Mesh* selectedShape;

//The colliders
struct Sphere* sphereCollider;
struct Polygon* polygonCollider;

//Transformation speeds
float movementSpeed = 0.02f;
float rotationSpeed = 0.01f;

//Input variables
bool isMousePressed = false;
double prevMouseX = 0.0f;
double prevMouseY = 0.0f;

//Function declarations
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
///
//Generates a sphere mesh with a given radius
void GenerateSphereMesh(float radius, int subdivisions)
{
	std::vector<Vertex> vertexSet;

	float pitch, yaw;
	yaw = 0.0f;
	pitch = 0.0f;
	int i, j;
	float pitchDelta = 360.0f / subdivisions;
	float yawDelta = 360.0f / subdivisions;

	float PI = 3.14159f;

	Vertex p1, p2, p3, p4;

	for (i = 0; i < subdivisions; i++)
	{
		for (j = 0; j < subdivisions; j++)
		{

			p1.x = radius * sin((pitch)* PI / 180.0f) * cos((yaw)* PI / 180.0f);
			p1.y = radius * sin((pitch)* PI / 180.0f) * sin((yaw)* PI / 180.0f);
			p1.z = radius * cos((pitch)* PI / 180.0f);
			p1.r = 1.0f;
			p1.g = 0.0f;
			p1.b = 1.0f;
			p1.a = 1.0f;

			p2.x = radius * sin((pitch)* PI / 180.0f) * cos((yaw + yawDelta)* PI / 180.0f);
			p2.y = radius * sin((pitch)* PI / 180.0f) * sin((yaw + yawDelta)* PI / 180.0f);
			p2.z = radius * cos((pitch)* PI / 180.0f);
			p2.r = 1.0f;
			p2.g = 0.0f;
			p2.b = 1.0f;
			p2.a = 1.0f;

			p3.x = radius * sin((pitch + pitchDelta)* PI / 180.0f) * cos((yaw + yawDelta)* PI / 180.0f);
			p3.y = radius * sin((pitch + pitchDelta)* PI / 180.0f) * sin((yaw + yawDelta)* PI / 180.0f);
			p3.z = radius * cos((pitch + pitchDelta)* PI / 180.0f);
			p3.r = 1.0f;
			p3.g = 0.0f;
			p3.b = 1.0f;
			p3.a = 1.0f;

			p4.x = radius * sin((pitch + pitchDelta)* PI / 180.0f) * cos((yaw)* PI / 180.0f);
			p4.y = radius * sin((pitch + pitchDelta)* PI / 180.0f) * sin((yaw)* PI / 180.0f);
			p4.z = radius * cos((pitch + pitchDelta)* PI / 180.0f);
			p4.r = 1.0f;
			p4.g = 0.0f;
			p4.b = 1.0f;
			p4.a = 1.0f;

			vertexSet.push_back(p1);
			vertexSet.push_back(p2);
			vertexSet.push_back(p2);
			vertexSet.push_back(p3);
			vertexSet.push_back(p3);
			vertexSet.push_back(p4);
			vertexSet.push_back(p4);
			vertexSet.push_back(p1);

			yaw = yaw + yawDelta;
		}

		pitch += pitchDelta;
	}

	sphere = new Mesh(vertexSet.size(), &vertexSet[0], GL_LINES);

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
//Tests for collisions between a sphere and a convex polygon
//
//Overview:
//	This algorithm tests for collisions between a sphere and a convex polygon through three different tests.
//	First we perform a halfspace test to determine if the sphere completely lies on one side of the polygon.
//	Then, if this is not the case, we project the sphere's center onto the plane in which the polygon lies &
//	determine if the projected center falls inside of the polygon. If it does we have a collision, if not however,
//	we must perform one more test: Find the closest point on every edge to the sphere & determine if that point
//	falls inside of the sphere using a distance test.
//
//Parameters:
//	sphereCollider: The Sphere collider to test
//	spherePosition: The sphere's position in worldspace
//	polyCollider:	The polygon collider to test
//	polyModelMatrix:The model to world matrix of the polygon
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const Sphere &sphereCollider, const glm::vec3 &spherePosition, const Polygon &polyCollider, const glm::mat4 &polyModelMatrix)
{
	//Convert all points to worldspace
	std::vector<glm::vec3> worldPoints;
	for (int i = 0; i < polyCollider.points.size(); i++)
	{
		worldPoints.push_back(glm::vec3(polyModelMatrix * glm::vec4(polyCollider.points[i], 1.0f)));
	}

	//Find two vectors in plane of polygon
	glm::vec3 u, v;
	u = worldPoints[1] - worldPoints[0];
	v = worldPoints[2] - worldPoints[0];

	//Get normal to plane
	glm::vec3 normal = glm::normalize(glm::cross(u, v));

	//Determine distance from origin to plane
	float d = glm::dot(worldPoints[0], normal);

	//Find distance from sphere to plane
	float sphereToPlane = fabs(glm::dot(spherePosition, normal) - d);

	//If distance is greater than radius of sphere, return false
	if (sphereToPlane > sphereCollider.radius) return false;

	//Else, we must project sphere center onto plane
	glm::vec3 projSphereCenter = spherePosition - glm::dot(normal, spherePosition) * normal;

	//Now we must test if it is inside of the polygon. Take a vector from the sphere center to each of the polygon vertices.
	//Consider the angle between the vector and two adjacent polygon vertices. If the center is inside of the polygon, the angle between these will add to 360 degrees!
	float totalAngle = 0.0f;
	for (int i = 0; i < worldPoints.size(); i++)
	{
		glm::vec3 v1 = glm::normalize(worldPoints[i] - projSphereCenter);
		glm::vec3 v2 = glm::normalize((i < worldPoints.size() - 1 ? worldPoints[i + 1] : worldPoints[0]) - projSphereCenter);


		//Compute dot product between angles
		totalAngle += glm::degrees(acosf(glm::dot(v1, v2)));
	}

	float errorRange = 0.0001f;
	//If the dot product is 1.0f (cos 2PI = 1.0f) we have a collision
	if (totalAngle >= 360.0f - errorRange && totalAngle <= 360.0f + errorRange) return true;

	//If not, there is still one final edge case.
	//We must compute the vertex on each edge of the polygon closest to the sphere, and see if it falls within the sphere.
	for (int i = 0; i < worldPoints.size(); i++)
	{
		glm::vec3 v1 = worldPoints[i];
		glm::vec3 v2 = i < worldPoints.size() - 1 ? worldPoints[i + 1] : worldPoints[0];
		glm::vec3 edge = v2 - v1;
		float mag = glm::length(edge);
		edge = glm::normalize(edge);

		glm::vec3 v1ToSphere = spherePosition - v1;


		float closestDist = glm::dot(edge, v1ToSphere);
		if (closestDist < 0.0f) closestDist = 0.0f;
		if (closestDist > mag) closestDist = mag;

		glm::vec3 closestPoint = v1 + edge * closestDist;
		if (glm::distance(closestPoint, spherePosition) <= sphereCollider.radius) return true;

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

	if (TestCollision(*sphereCollider,	glm::vec3(sphere->translation[3][0], sphere->translation[3][1], sphere->translation[3][2]),
		*polygonCollider, polygon->GetModelMatrix()))
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
	sphere->Draw();
	polygon->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == sphere ? polygon : sphere;

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
	window = glfwCreateWindow(800, 800, "Convex Polygon - Sphere Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate sphere mesh
	float radius = 1.0f;
	float scale = 0.25f;
	GenerateSphereMesh(radius, 40);

	//Translate the sphere
	sphere->translation = glm::translate(sphere->translation, glm::vec3(0.3f, 0.0f, 0.0f));

	//Scale the sphere
	sphere->scale = glm::scale(sphere->scale, glm::vec3(scale));

	//Generate polygon mesh
	struct Vertex polyVerts[16] = { 
		{ 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },

		{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f },
		{ 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f }
	};

	polygon = new struct Mesh(16, polyVerts, GL_TRIANGLES);

	//Translate the polygon
	polygon->translation = glm::translate(polygon->translation, glm::vec3(-0.3f, 0.0f, 0.0f));

	//Scale the polygon
	polygon->scale = glm::scale(polygon->scale, glm::vec3(0.2f, 0.2f, 0.2f));

	//Set the selected shape
	selectedShape = sphere;

	//Generate Sphere collider
	sphereCollider = new struct Sphere(radius * scale);
	//Generate polygon collider
	std::vector<glm::vec3> pts;
	pts.push_back(glm::vec3(polyVerts[0].x, polyVerts[0].y, polyVerts[0].z));
	pts.push_back(glm::vec3(polyVerts[1].x, polyVerts[1].y, polyVerts[1].z));
	pts.push_back(glm::vec3(polyVerts[5].x, polyVerts[5].y, polyVerts[5].z));
	pts.push_back(glm::vec3(polyVerts[7].x, polyVerts[7].y, polyVerts[7].z));
	polygonCollider = new struct Polygon(pts);

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

	delete sphere;
	delete polygon;

	//Delete Colliders
	delete sphereCollider;
	delete polygonCollider;

	// Frees up GLFW memory
	glfwTerminate();
}