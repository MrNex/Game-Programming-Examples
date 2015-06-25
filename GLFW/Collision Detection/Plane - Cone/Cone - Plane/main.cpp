/*
Title: Cone - Plane
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
This is a dmonstration of collision detection between a cone and a plane.
The demo contains a wireframe of a cone and a solid plane. When the objects are
not colliding the plane will appear blue and the cone will appear green. When
the two objects collide the plane will become pink and the cone will become yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD.
You can also move the shape along the Z axis with left shift and left control. Lastly, you
can rotate the objects by clicking the left mouse button and dragging the mouse.

This demo detects collisions by computing the point on the cone's base most in the direction
of the plane, then determining if that extreme point and the tip of the cone are on the same side
of the plane. This is done by observing the sign of the dot product of the plane normal with both
the point on the tip of the cone and the extreme point on the base of the cone after the system has
been shifted to have it's origin on the center of the plane. If the signs are different the two points
reside on different sides of the plane and there must be a collision. Else, there is no collision.

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader variables
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

//A cone collider struct
struct Cone
{
	glm::vec3 tip;			//Offset of tip of cone from origin of mesh
	glm::vec3 direction;	//Direction from tip to base of cone
	float height;			//Distance to center of base from tip
	float radius;			//Radius of base

	///
	//Generates a unit cone (-1 to 1 on all axes)
	Cone::Cone()
	{
		tip = glm::vec3(0.0f, 1.0f, 0.0f);
		direction = glm::vec3(0.0f, -1.0f, 0.0f);
		height = 2.0f;
		radius = 2.0f;
	}

	///
	//Generates a cone with a given tip, direction, height, and radius
	Cone::Cone(const glm::vec3 &tipOffset, const glm::vec3 &dir, float h, float r)
	{
		tip = tipOffset;
		direction = dir;
		height = h;
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
struct Mesh* cone;

struct Mesh* selectedShape;

struct Plane* planeCollider;
struct Cone* coneCollider;


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

//Reads in shader source code
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

//creates a shader from source code
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

	// Set various openGL options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

}

///
//Generates a cone mesh with given attributes and it's collider
//
//Parameters:
//	height: The height of the cone
//	radius: The radius of the cone
//	subdivisions: The number of subdivisions the cone has
void GenerateCone(float height, float radius, int subdivisions)
{
	std::vector<Vertex> vertexSet;

	float pitch;
	pitch = 0.0f;
	int i;

	float PI = 3.14159f;
	float pitchDelta = (2 * PI) / subdivisions;

	//Generate the tip of the cone
	Vertex tip;
	tip.x = 0.0f;
	tip.y = height / 2.0f;
	tip.z = 0.0f;
	tip.r = 1.0f;
	tip.g = 1.0f;
	tip.b = 0.0f;
	tip.a = 1.0f;

	//Generate the center of the base
	Vertex base;
	base.x = 0.0f;
	base.y = -height / 2.0f;
	base.z = 0.0f;
	base.r = 1.0f;
	base.g = 1.0f;
	base.b = 0.0f;
	base.a = 1.0f;



	Vertex p1, p2;

	for (i = 0; i < subdivisions; i++)
	{
		//Current point on base
		p1.x = radius * cos(pitch);
		p1.y = base.y;
		p1.z = radius * sin(pitch);
		p1.r = 1.0f;
		p1.g = 1.0f;
		p1.b = 0.0f;
		p1.a = 1.0f;

		//Connect to next point on base
		p2.x = radius * cos(pitch + pitchDelta);
		p2.y = base.y;
		p2.z = radius * sin(pitch + pitchDelta);
		p2.r = 1.0f;
		p2.g = 1.0f;
		p2.b = 0.0f;
		p2.a = 1.0f;


		//Current point to next point
		vertexSet.push_back(p1);
		vertexSet.push_back(p2);
		//Current point to tip
		vertexSet.push_back(p1);
		vertexSet.push_back(tip);
		//Current point to base
		vertexSet.push_back(p1);
		vertexSet.push_back(base);

		pitch += pitchDelta;
	}

	cone = new Mesh(vertexSet.size(), &vertexSet[0], GL_LINES);
	glm::vec3 tipv3(tip.x, tip.y, tip.z);
	glm::vec3 basev3(base.x, base.y, base.z);
	coneCollider = new struct Cone(tipv3, basev3, height, radius);

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for collisions between a Cone and a plane.
//
//Overview:
//	This algorithm detects collisions by computing the point on the cone's base most in the direction
//	of the plane, then determining if that extreme point and the tip of the cone are on the same side
//	of the plane.This is done by observing the sign of the dot product of the plane normal with both
//	the point on the tip of the cone and the extreme point on the base of the cone after the system has
//	been shifted to have it's origin on the center of the plane. If the signs are different the two points
//	reside on different sides of the plane and there must be a collision. Else, there is no collision.
//
//Parameters:
//	cCollider: The cone's collider
//	cModelMatrix: The cone's model to world transformation matrix
//	pCollider: The plane's collider
//	pModelMatrix: The plane's model to world transformation matrix
//
//Returns:
//	true if a collision is detected, else false
bool TestCollision(const Cone &cCollider, const glm::mat4 &cModelMatrix, const Plane &pCollider, const glm::mat4 &pModelMatrix)
{
	//Sometimes, collision detection ends up being inaccurate because of the rounding issues which arise with floating point numbers.
	//As such, we must be prepared to register collisions within some acceptable error range we will call epsilon.
	//You can choose to ignore epsilon, but certain cases involving very small intersections which should register as collisions will not.
	//Alternatively, including epsilon may cause cases very very close to collision but not quite intersecting yet to register as collisions.
	//I recommend playing with this value to find something that suits your specific need better.
	float epsilon = FLT_EPSILON;
	//Step1: We must get the cone's tip, and direction into a space which has the plane at the origin & the plane's normal in world space
	glm::vec3 planePos(pModelMatrix[3][0], pModelMatrix[3][1], pModelMatrix[3][2]);
	glm::vec3 planeNorm = glm::vec3( glm::normalize(pModelMatrix * glm::vec4(pCollider.normal, 0.0f)));
	glm::vec3 coneTip = glm::vec3(cModelMatrix * glm::vec4(cCollider.tip, 1.0f)) - planePos;
	glm::vec3 coneDir = glm::vec3( cModelMatrix * glm::vec4(cCollider.direction, 0.0f));		//Note: coneDir is not a unit vector! Now scaled!


	//Step 2: We must determine what side of the plane the tip of the cone lies on. We can do this by observing the sign of the dot product of the tip of the cone with the
	//	plane's normal.
	float side = glm::dot(coneTip, planeNorm);
	if (fabs(side) - epsilon <= 0.0f) return true;	//Tip of cone is on the plane!
	side = side / fabs(side);		//Get 1.0f or -1.0f to indicate positive or negative halfspace of plane

	//Step 3: Find the point on the cone's base closest to the plane
	glm::vec3 normTowardsPlane = side * planeNorm;		//Get the normal of the plane pointing towards the plane relative to the tip
	//The point on the base closest to the plane will be on the edge of the base iff the base is not parallel to the plane.
	//Otherwise we can use the center of the base, but we do not need a special case for this.

	//We can determine the point closest on the base by starting at the tip, and moving cone.height in the direction to the base,
	//Then we can use the normTowardsPlane to generate a vector which lies in the plane of the cone base pointing most in the direction of the plane.
	//This can be thought of as the equivilent of projecting the normTowardsPlane onto the 2D subspace in which the cone's base lies.

	//However, we can use a special formula to calculate this, the direction vector baseCenterToClosest can be calculated as:
	//	baseCenterToClosest = normalize(normTowardsPlane X coneDir) X coneDir
	glm::vec3 baseCenterToClosest = glm::cross(glm::normalize(glm::cross(normTowardsPlane, coneDir)), coneDir);


	//Now the closest point on base can be found as:
	//	closestBase = coneTip + height * coneDir + radius * baseCenterToClosest
	glm::vec3 closestBase = coneTip + cCollider.height * coneDir + cCollider.radius * baseCenterToClosest;

	//Step 4: Determine what side of the plane the closest point on the base lies on.
	//	If this is equal in sign to the side of the plane the tip lies on, there is no collision.
	//	Else, we have a collision.
	//
	//	Alternatively, if you wanted to always return true when any point on the cone is on one side of the plane
	//	You can alter this test to simply check if any side comes back as negative or positive (to check the negative or positive half-spaces formed
	//	by the plane respectively)
	if (side - epsilon > 0.0f)
	{
		if (glm::dot(closestBase, planeNorm) <= epsilon) return true;
	}
	else if (side + epsilon < 0.0f)
	{
		if (glm::dot(closestBase, planeNorm) >= -epsilon) return true;
	}
	else//side == 0.0f, means closest point on base is on plane
	{
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

	if (TestCollision(*coneCollider, cone->GetModelMatrix(), *planeCollider, plane->GetModelMatrix()))
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
	cone->Draw();
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
			selectedShape = selectedShape == plane ? cone : plane;

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
	window = glfwCreateWindow(800, 800, "Cone - Plane Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Generate cone & collider
	float radius = 1.0f;
	float height = 2.0f;
	float scale = 0.25f;
	GenerateCone(height, radius, 40);

	//Scale the cone
	cone->scale = cone->scale * glm::scale(glm::mat4(1.0f), glm::vec3(scale));

	//Translate the cone
	cone->translation = glm::translate(cone->translation, glm::vec3(-0.15f, 0.0f, 0.0f));


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

	//Generate plane collider

	//Get two edges of the plane and take the cross product for the normal (Or just hardcode it, for example we know the normal to this plane
	//Will be the Z axis, because the plane mesh lies in the XY Plane to start.
	glm::vec3 edge1(planeVerts[0].x - planeVerts[1].x, planeVerts[0].y - planeVerts[1].y, planeVerts[0].z - planeVerts[1].z);
	glm::vec3 edge2(planeVerts[1].x - planeVerts[2].x, planeVerts[1].y - planeVerts[2].y, planeVerts[1].z - planeVerts[2].z);

	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

	planeCollider = new struct Plane(normal);

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

	delete cone;
	delete plane;

	delete planeCollider;
	delete coneCollider;

	// Frees up GLFW memory
	glfwTerminate();
}