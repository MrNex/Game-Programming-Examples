/*
Title: OBB - Plane
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
This is a demonstration of an Oriented Bounding Box and a Plane. 
The demo contains a wireframe of a box and a solid plane. The plane is colored blue
And the box is colored green until the two collide, then the plane will change to the color
pink and the box will change to the color yellow.

Both shapes are able to be moved, you can move the selected shape in the XY plane with WASD
and along the Z axis with Left Shift & Left Control. You can also rotate the selected shape by clicking
and dragging the mouse.

This demo detects collisions by making sure all corners of the box lie on the same side of the plane.
We do this by translating the corners of the box into world space and the normal of the plane
into world space, then translating the entire system to to be centered on the origin of the plane.
From here, you are able to determine what side of the plane the corners fall on by observing the dot
product of the corner's position vectors with the plane's normal. If any sign doesn't match another,
there is a collision.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
2D Game Collision Detection by Thomas Schwarzl
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader variables
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

// Uniforms
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

struct OBB
{
	float width;
	float height;
	float depth;

	///
	//Generates an OBB of unit length
	OBB::OBB()
	{
		width = 2.0f;
		height = 2.0f;
		depth = 2.0f;
	}

	///
	//Generates an OBB of a given width and height
	OBB::OBB(float w, float h, float d)
	{
		width = w;
		height = h;
		depth = d;
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
struct Mesh* box;

struct Mesh* selectedShape;

struct Plane* planeCollider;
struct OBB* boxCollider;


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

// create shader from source code
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
//Tests for collisions between a plane and an Oriented bounding box.
//
//Overview:
//	This algorithm detects collisions by testing whether all corners of the collider are on the same side of the plane.
//	If the corners are not on one side of the plane, and the plane extends infinitely, then there must be a collision.
//	Because the plane extends infinitely we do not need to worry about the scale of the plane, only the position and rotation.
//	If the planes position is not centered at the origin the algorithm becomes more difficult, to deal with this we will translate the
//	origin of the system to be on the plane by translating both the plane and the OBB by the negative of the plane's position.
//
//Parameters:
//	obb: The oriented bounding box being tested for collision
//	obbTrans: The translation transformation matrix of the oriented bounding box
//	obbRotation: The rotation transformation matrix of the oriented bounding box
//	OBBScale: The scale transformation matrix of the oriented bounding box
//	(Tip: Instead of using the above three matrices, you could just send one model to world matrix and accomplish the same effect)
//	plane: The plane being tested for collision
//	planeTrans: The translation transformation matrix of the plane (We really just need the position vector, which we will extract from this)
//	planeRotation: The rotation transformation matrix of the plane
//
//Returns:
//	True if a collision is detected,
//	else false
bool TestCollision(
	const OBB &obb, const glm::mat4 &obbTrans, const glm::mat4 &obbRotation, const glm::mat4 &obbScale,
	const Plane &plane, const glm::mat4 &planeTrans, const glm::mat4 &planeRotation)
{
	//Step 1: We must construct a list of the corners of the obb in model space
	std::vector<glm::vec4> corners;

	corners.push_back(glm::vec4(obb.width  / 2.0f, obb.height  / 2.0f, obb.depth  / 2.0f, 1.0f));
	corners.push_back(glm::vec4(-obb.width / 2.0f, obb.height  / 2.0f, obb.depth  / 2.0f, 1.0f));
	corners.push_back(glm::vec4(-obb.width / 2.0f, obb.height  / 2.0f, -obb.depth / 2.0f, 1.0f));
	corners.push_back(glm::vec4(obb.width  / 2.0f, obb.height  / 2.0f, -obb.depth / 2.0f, 1.0f));
	corners.push_back(glm::vec4(obb.width  / 2.0f, -obb.height / 2.0f, obb.depth  / 2.0f, 1.0f));
	corners.push_back(glm::vec4(-obb.width / 2.0f, -obb.height / 2.0f, obb.depth  / 2.0f, 1.0f));
	corners.push_back(glm::vec4(-obb.width / 2.0f, -obb.height / 2.0f, -obb.depth / 2.0f, 1.0f));
	corners.push_back(glm::vec4(obb.width  / 2.0f, -obb.height / 2.0f, -obb.depth / 2.0f, 1.0f));

	//Step 2: We must create a transformation matrix which will translate all corners of the box into a coordinate system such that
	//The center of the system is the center of the plane.
	glm::mat4 transform = glm::translate(obbTrans, glm::vec3(-planeTrans[3][0], -planeTrans[3][1], -planeTrans[3][2])) * obbRotation * obbScale;

	//Step 3: We must orient our normal to face the correct direction in world space
	glm::vec4 worldNorm = planeRotation * glm::vec4(plane.normal, 0.0f);

	//Step 4: We must translate these corners into this new space, and test which side of the plane they are on.
	//We are able to tell which side of the plane the corners fall on by observing the sign of the dot product operation.
	//If all of the signs are the same, there cannot be a collision, but if there is at least one sign different from the rest,
	//We have a collision.

	glm::vec4 transformedCorner = transform * corners[0];
	float currentDotProd = glm::dot(transformedCorner, worldNorm);
	
	int numCorners = corners.size();
	//Check if any of the other corners do not match the sign of the first corner
	//When dotted with the plane normal
	if (currentDotProd < 0)
	{

		for (int i = 1; i < numCorners; i++)
		{
			transformedCorner = transform * corners[i];
			currentDotProd = glm::dot(transformedCorner, worldNorm);

			//If it does not match
			if (currentDotProd >= 0)
				return true;
		}
	}
	else if (currentDotProd > 0)
	{
		for (int i = 1; i < numCorners; i++)
		{
			transformedCorner = transform * corners[i];
			currentDotProd = glm::dot(transformedCorner, worldNorm);

			//If it does not match
			if (currentDotProd <= 0)
				return true;
		}
	}
	else
	{
		//IF the dot product returns 0, the position vector of the corner is orthogonal to the normal
		//When contained in a system where the plane is centered at the origin

		//Therefore, by mathematical definition of a plane, the first corner we checked was located on the plane.
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

	if (TestCollision(*boxCollider, box->translation, box->rotation, box->scale, *planeCollider, plane->translation, plane->rotation))
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
	box->Draw();
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
			selectedShape = selectedShape == plane ? box : plane;

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

		//This set of controls is used to rotate the selectedShape
		if (key == GLFW_KEY_Q)
			selectedShape->translation = glm::rotate(selectedShape->translation, rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
		if (key == GLFW_KEY_E)
			selectedShape->translation = glm::rotate(selectedShape->translation, -rotationSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
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
	window = glfwCreateWindow(800, 800, "OBB - Plane Collision Detection", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate box mesh
	struct Vertex boxVerts[24];
	boxVerts[0] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Left Corner
	boxVerts[1] = { -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Left Corner

	boxVerts[2] = { -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Left Corner
	boxVerts[3] = { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Right Corner

	boxVerts[4] = { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Right Corner
	boxVerts[5] = { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Right Corner

	boxVerts[6] = { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Right Corner
	boxVerts[7] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Left Corner


	boxVerts[8] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Left Corner
	boxVerts[9] = { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Top Back Left Corner

	boxVerts[10] = { -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Left Corner
	boxVerts[11] = { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Top Front Left Corner

	boxVerts[12] = { 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Right Corner
	boxVerts[13] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Top Front Right Corner

	boxVerts[14] = { 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Right Corner
	boxVerts[15] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Top Back Right Corner


	boxVerts[16] = { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Left Corner
	boxVerts[17] = { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Left Corner

	boxVerts[18] = { -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Left Corner
	boxVerts[19] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Right Corner

	boxVerts[20] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Front Right Corner
	boxVerts[21] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Right Corner

	boxVerts[22] = { 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Right Corner
	boxVerts[23] = { -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f };	//Bottom Back Left Corner

	box = new struct Mesh(24, boxVerts, GL_LINES);

	//Scale the box
	box->scale = box->scale * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

	//Translate the box
	box->translation = glm::translate(box->translation, glm::vec3(-0.1f, 0.0f, 0.0f));


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
	boxCollider = new struct OBB(boxVerts[3].x - boxVerts[2].x, boxVerts[9].y - boxVerts[8].y, boxVerts[1].z - boxVerts[0].z);

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
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	delete box;
	delete plane;

	delete boxCollider;
	delete planeCollider;

	// Frees up GLFW memory
	glfwTerminate();
}