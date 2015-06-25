/*
Title: Sphere - Plane
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
This is a demonstration of collision detection between a sphere and a plane.
The demo contains a wireframe of a sphere and a solid plane. When the objects are not colliding
the plane will appear blue and the sphere green. When the two objects collide the plane will become
pink and the sphere will become yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with Left Shift and Left Control. Lastly,
you can rotate the plane by clicking and dragging the mouse.

This demo detects collisions by by computing the distance between the plane and the center of the sphere.
This is done by taking a vector from the center of the plane to the center of the sphere and computing the dot product (Or scalar projection)
of this vector and the normalized normal vector of the plane. Once this distance is computed, if the distance is less than the radius, we must have a collision.

References:
Base by Srinivasan Thiagarajan
Sphere Collision 3D by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
// shader variables
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

// uniform variables
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

//A sphere collider struct
struct Sphere
{
	float radius;

	///
	//Generates a sphere with a radius of 1
	Sphere::Sphere()
	{
		radius = 1.0f;
	}

	///
	//Generates a sphere with a given radius
	Sphere::Sphere(float r)
	{
		radius = r;
	}
};

//A plane collider struct
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
struct Mesh* sphere;

struct Mesh* selectedShape;

struct Plane* planeCollider;
struct Sphere* sphereCollider;


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

//create shader from source code
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

	//create shader program
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

	// Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

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
			p1.g = 1.0f;
			p1.b = 0.0f;
			p1.a = 1.0f;

			p2.x = radius * sin((pitch)* PI / 180.0f) * cos((yaw + yawDelta)* PI / 180.0f);
			p2.y = radius * sin((pitch)* PI / 180.0f) * sin((yaw + yawDelta)* PI / 180.0f);
			p2.z = radius * cos((pitch)* PI / 180.0f);
			p2.r = 1.0f;
			p2.g = 1.0f;
			p2.b = 0.0f;
			p2.a = 1.0f;

			p3.x = radius * sin((pitch + pitchDelta)* PI / 180.0f) * cos((yaw + yawDelta)* PI / 180.0f);
			p3.y = radius * sin((pitch + pitchDelta)* PI / 180.0f) * sin((yaw + yawDelta)* PI / 180.0f);
			p3.z = radius * cos((pitch + pitchDelta)* PI / 180.0f);
			p3.r = 1.0f;
			p3.g = 1.0f;
			p3.b = 0.0f;
			p3.a = 1.0f;

			p4.x = radius * sin((pitch + pitchDelta)* PI / 180.0f) * cos((yaw)* PI / 180.0f);
			p4.y = radius * sin((pitch + pitchDelta)* PI / 180.0f) * sin((yaw)* PI / 180.0f);
			p4.z = radius * cos((pitch + pitchDelta)* PI / 180.0f);
			p4.r = 1.0f;
			p4.g = 1.0f;
			p4.b = 0.0f;
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

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a sphere and a plane.
//
//Overview:
//	This algorithm detects collisions between a sphere and a plane by computing the distance between the plane and the center of the sphere.
//	This is done by taking a vector from the center of the plane to the center of the sphere and computing the dot product (Or scalar projection)
//	of this vector and the normalized normal vector of the plane. Once this distance is computed, if the distance is less than the radius, we must have a collision.
//
//Parameters:
//	sCollider: The sphere collider
//	sTrans: The sphere's translation transformation matrix (All that's needed here is the sphere position, feel free to pass that as a vec3 instead).
//			Often it is just easier to extract the data from the matrix used for rendering than have a copy of it that is continuously updated elsewhere.
//	pCollider: The plane collider
//	pTrans: The plane's translation transformation matrix (Again, the only thing needed here is the position-- no need for it to be a 4x4 matrix)
//	pRotation: The plane's rotation transformation matrix
//
//	(Alert!: If you were trying to accomplish something like having a sphere which is constantly changing size, you would need to take into account the changes in scale here
//				This would force you to include an extra parameter.)
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const Sphere &sCollider, const glm::mat4 &sTrans, const Plane &pCollider, const glm::mat4 &pTrans, const glm::mat4 &pRotation)
{
	//Step 1: We must get a vector from the plane's center to the sphere's center
	glm::vec4 planeToSphere(			//Vec4 so we do not need to convert the plane's normal to a vec3 later.
		sTrans[3][0] - pTrans[3][0],
		sTrans[3][1] - pTrans[3][1],
		sTrans[3][2] - pTrans[3][2],
		0.0f							// (a) This could be one, but making it zero ensures we don't make a mistake later.
		);								//	Although it is a position, and positions do usually have a 1.0f in the last component.

	//Step 2: We must convert the plane normal to world coordinates
	glm::vec4 worldNormal = pRotation * glm::vec4(pCollider.normal, 0.0f);		//If we made (a) 1.0f instead of 0.0f, we would need to make sure this was 0.0f.
																				//But there is no harm in making both of them 0.0f! So that is what we will do.

	//Step 3: Use the scalar projection to compute the distance from the center of the sphere to the plane.
	//	The scalar projection onto a unit vector is the equivilent of the dot product.
	//	Because our plane normal is normalized, we can use the dot product. If the plane normal wasn't normalized the equation would be:
	//	sProj(X, Y) = (X . Y) / |Y|
	//	Where we are computing the scalar projection of a vector X onto a vector Y, " . " denotes the dot product operation, and "| |" denotes the magnitude

	float distance = fabs(glm::dot(planeToSphere, worldNormal));	//Distances can't be negative!

	//Step 4: Determine if the distance is less than the radius, if so we have a collision,
	//	Else there is no collision.
	if (distance < sCollider.radius) return true;
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

		//Rotate the selected shape by an angle equal to the mouse movement
		if (deltaMouseX != 0.0f)
		{
			glm::mat4 yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
			plane->rotation = plane->rotation * yaw;
		}
		if (deltaMouseY != 0.0f)
		{
			glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * -rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
			plane->rotation = pitch * plane->rotation;
		}
		
		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

	}

	if (TestCollision(*sphereCollider, sphere->translation, *planeCollider, plane->translation, plane->rotation))
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
			selectedShape = selectedShape == plane ? sphere : plane;

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
	window = glfwCreateWindow(800, 800, "Sphere - Plane Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate Sphere
	float radius = 1.0f;
	float scale = 0.25f;
	GenerateSphereMesh(radius, 40);

	//Scale the sphere
	sphere->scale = sphere->scale * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	//Translate the sphere
	sphere->translation = glm::translate(sphere->translation, glm::vec3(-0.15f, 0.0f, 0.0f));


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
	plane->translation = glm::translate(plane->translation, glm::vec3(0.15f, 0.0f, 0.0f));

	//Set the selected shape
	selectedShape = plane;

	//Generate the colliders
	sphereCollider = new Sphere(radius * scale);	//Be sure when calculating the radius of the collider you include / adjust for the scale of the mesh.

	//Get two edges of the plane and take the cross product for the normal (Or just hardcode it, for example we know the normal to this plane
	//Will be the Z axis, because the plane mesh lies in the XY Plane to start.
	glm::vec3 edge1(planeVerts[0].x - planeVerts[1].x, planeVerts[0].y - planeVerts[1].y, planeVerts[0].z - planeVerts[1].z);
	glm::vec3 edge2(planeVerts[1].x - planeVerts[2].x, planeVerts[1].y - planeVerts[2].y, planeVerts[1].z - planeVerts[2].z);

	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
	
	planeCollider = new struct Plane(normal);

	//Print controls
	std::cout << "Use WASD to move the selected shape in the XY plane.\nUse left CTRL & left shift to move the selected shape along Z axis.\n";
	std::cout << "Left click and drag the mouse to rotate the plane.\nUse spacebar to swap the selected shape.\n";

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

	delete sphere;
	delete plane;

	delete planeCollider;
	delete sphereCollider;

	// Frees up GLFW memory
	glfwTerminate();
}