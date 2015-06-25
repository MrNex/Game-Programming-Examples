/*
Title: Convex Hull - Line Segment
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
This is a demonstration of collision detection between a convex hull and a line segment.
The demo contains a line segment wireframe of a tetrehedron. The line will appear green and the 
tetrahedron will appear blue. When the shapes collide, their colors will change to red and pink, respectively.

You can move the shapes around the X-Y plane with WASD, and along the Z axis with
Left Shift & Left Control. Pressing space will toggle the shape being moved.
You can also rotate the selected shape by holding the left mouse button and dragging the mouse.

This function uses a series of half-space tests to perform collision detection between
a line segment and a convex hull. A convex hull can be described as a set of intersecting planes.
What this algorithm will do is compute the intersection scalar value denoting the point of intersection of the line segment with the plane
in terms of the parametric equation of the line. If this scalar value is between 0 and 1 then that segment must intersect the plane.
Using the dot product we can determine if the line segment at that intersection is entering the hull or exiting the hull. If we keep track
of the largest t at which it enters, and the smallest t at which it exits, we can determine the intersection interval of the line.
if the end of that interval (the smallest t at which it exits) is smaller than the start of that interval (the largest t at which it enters)
there is no collision. However, if this is not the case, we do have a collision!

References:
Base by Srinivasan Thiagarajan
Real Time Collision Detection by Christer Ericson
AABB-2D by Brockton Roth
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader program
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

	glm::mat4 Mesh::GetModelMatrix()
	{
		return this->translation * this->rotation * this->scale;
	}

	void Mesh::Draw(void)
	{
		//Generate the model matrix
		glm::mat4 modelMatrix = this->GetModelMatrix();

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

struct Line
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;

	Line::Line(glm::vec3 start, glm::vec3 end)
	{
		startPoint = start;
		endPoint = end;
	}
};

struct Edge
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::vec3 direction;
};

struct Plane
{
	glm::vec3 normal;	//Normal of the plane
	glm::vec3 point;	//A point on the plane
};


//Convex hull collider as a collection of planes
struct ConvexHull
{
	std::vector<struct Plane> planes;

	///
	//Generates a convex hull from a given mesh
	//This method assumes the vertices of the mesh are given as lines
	//Where each set of two vertices represents two endpoints of a line which makes up the mesh.
	//And every two lines are coplanar
	//
	//Parameters:
	//	m: A reference to the mesh to fit this convex hull to
	ConvexHull::ConvexHull(const Mesh &m)
	{
		//We must be able to track adjacent edges
		std::vector<struct Edge> edges;

		//Loop through all mesh vertices creating the edges they make up
		//and add them to a list if they are not duplicates
		for (int i = 0; i < m.numVertices; i+=2)
		{
			//Construct a vec3 to represent the ith vertex
			glm::vec3 currentPoint(m.vertices[i].x, m.vertices[i].y, m.vertices[i].z);
			
			//Construct a vec3 to represent the (i + 1)th vertex
			glm::vec3 nextPoint(m.vertices[i + 1].x, m.vertices[i + 1].y, m.vertices[i + 1].z);


			//Generate an edge from the two points
			struct Edge edge = 
			{
				currentPoint,
				nextPoint,
				glm::normalize(nextPoint - currentPoint)
			};

			//Add it to the list of edges
			edges.push_back(edge);

		}

		//We must loop through all edges and find the co-planar ones to take the cross product
		//And generate planes for the convex hull
		int size = edges.size();
		for (int i = 0; i < size; i++)
		{
			for (int j = i + 1; j < size; j++)
			{
				//If the ith edge and the jth edge have a point in common we must cross their directions to get a face normal
				if (edges[i].startPoint == edges[j].startPoint ||
					edges[i].startPoint == edges[j].endPoint ||
					edges[i].endPoint == edges[j].startPoint ||
					edges[i].endPoint == edges[j].endPoint)
				{
					glm::vec3 norm = glm::normalize(glm::cross(edges[i].direction, edges[j].direction));
					//Check if normal is contained in the list of normals yet
					bool contains = false;
					int nSize = planes.size();
					for (int k = 0; k < nSize; k++)
					{
						if (planes[k].normal == norm)
						{
							contains = true;
							break;
						}
					}
					
					//If it is not a duplicate, generate plane and add it to the list
					if (!contains)
					{
						struct Plane p;
						p.normal = norm;
						p.point = edges[i].startPoint;
						planes.push_back(p);

					}
				}
			}
		}
	}
};

struct Mesh* tetrahedron;
struct Mesh* line;

struct Mesh* selectedShape;

struct ConvexHull* tetraHull;
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
//Reads in shader source
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

	//Alter the hue matrix
	hue[0][0] = 0.0f;

	//Get uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	//Set options
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);

	//Set glfw event callbacks to handle input
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	//Thicker lines
	glLineWidth(3.0f);
}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for the collision between a convex hull and a line segment
//
//Overview:
//	This function uses a series of half-space tests to perform collision detection between
//	a line segment and a convex hull, or convex polyhedron. A convex hull can be described as a set of intersecting planes.
//	What this algorithm will do is compute the intersection scalar value denoting the point of intersection of the line segment with the plane
//	in terms of the parametric equation of the line. If this scalar value is between 0 and 1 then that segment must intersect the plane.
//	Using the dot product we can determine if the line segment at that intersection is entering the hull or exiting the hull. If we keep track
//	of the largest t at which it enters, and the smallest t at which it exits, we can determine the intersection interval of the line.
//	if the end of that interval (the smallest t at which it exits) is smaller than the start of that interval (the largest t at which it enters)
//	there is no collision. However, if this is not the case, we do have a collision!
//
//Parameters:
//	hull: The convex hull being tested
//	hullModelMatrix: The model to world matrix of the convex hull
//	lCollider: The line collider being tested
//	lineModelMatrix: The model to world matrix of the line
bool TestCollision(const struct ConvexHull &hull, const glm::mat4 &hullModelMatrix, const struct Line &lCollider, const glm::mat4 &lineModelMatrix)
{
	//Step 1: We must convert all normals and points for both objects into world space
	std::vector<glm::vec3> worldPoints;
	std::vector<glm::vec3> worldNormals;

	glm::vec3 hullCenter = glm::vec3(hullModelMatrix[3][0], hullModelMatrix[3][1], hullModelMatrix[3][2]);

	int size = hull.planes.size();
	for (int i = 0; i < size; i++)
	{
		worldPoints.push_back(glm::vec3(hullModelMatrix * glm::vec4(hull.planes[i].point, 1.0f)));
		worldNormals.push_back(glm::normalize(glm::vec3(hullModelMatrix * glm::vec4(hull.planes[i].normal, 0.0f))));

		//Determine if the normal is pointing in the right direction! (This is important only if you do not know for sure that your normals are facing outward)
		if (glm::dot(hullCenter - worldPoints[i], worldNormals[i]) > 0.0f) worldNormals[i] *= -1.0f;
	}

	glm::vec3 worldStart = glm::vec3(lineModelMatrix * glm::vec4(lCollider.startPoint, 1.0f));
	glm::vec3 worldEnd = glm::vec3(lineModelMatrix * glm::vec4(lCollider.endPoint, 1.0f));

	//Step 2: Determine the direction of the line
	glm::vec3 lineDir = worldEnd - worldStart;

	//Step 3: Determine the scalar interval along the line that is inside of the hull
	float tStart = 0.0f;
	float tEnd = 1.0f;

	//For every plane
	for (int i = 0; i < size; i++)
	{
		//The scalar value indicating the intersection of a plane and a line (in terms of the parametric equation of the line)
		//can be solved by substituting the parametric equation of a line:
		//	X = worldStart + t * (worldEnd - worldStart)
		//into the equation for a plane
		//	plane.normal . plane.point = normal . X
		//Giving us:
		//	normal . point = normal . worldStart + t * (normal . lineDir)
		//And solving for t
		//t = (normal . point - normal . worldStart) / (normal . lineDir)
		//t = (normal . (point - worldStart)) / (normal . lineDir)
		//So lets find t!
		float numerator = glm::dot(worldNormals[i], worldPoints[i] - worldStart);
		float denominator = glm::dot(worldNormals[i], lineDir);

		//If the denominator is 0, the line runs parallel to the plane!
		if (fabs(denominator) <=  FLT_EPSILON)
		{
			//If the numerator is greater than 0, the line runs on the outside! In this case intersection with the hull (yet alone the plane) would be impossible
			if (numerator < 0.0f) return false;
		}
		else
		{
			float t = numerator / denominator;

			//If the denominator is less than 0, the plane and the line direction point opposite directions
			//Therefore, the line is entering the hull, so we should see if we have a new largest interval start value
			if (denominator < 0.0f)
			{
				if (t > tStart) tStart = t;
			}
			else
				if (t < tEnd) tEnd = t;

		}

		//If tStart ever gets past tEnd, there is no intersection!
		if (tStart > tEnd) return false;
	}

	return 0.0f <= tStart && tStart <= 1.0f && 0.0f <= tEnd && tEnd <= 1.0f;	//Make sure it intersects within range of the segment!
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

		if (selectedShape == line)
		{
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
		}
		else
		{
			glm::mat4 yaw, pitch;
			//Rotate the selected shape by an angle equal to the mouse movement
			if (deltaMouseX != 0.0f)
			{
				yaw = glm::rotate(glm::mat4(1.0f), deltaMouseX * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
				//selectedShape->rotation = selectedShape->rotation * yaw;
			}
			if (deltaMouseY != 0.0f)
			{
				pitch = glm::rotate(glm::mat4(1.0f), deltaMouseY * -rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
				//selectedShape->rotation = pitch * selectedShape->rotation;
			}
			selectedShape->rotation = yaw * pitch * selectedShape->rotation;
		}
		
		
		//Update previous positions
		prevMouseX = currentMouseX;
		prevMouseY = currentMouseY;

	}

	if (TestCollision(*tetraHull, tetrahedron->GetModelMatrix(), *lineCollider, line->GetModelMatrix()))
	{
		//Change hue to red
		hue[0][0] = 1.0f;
		hue[1][1] = 0.0f;
	}
	else
	{
		//Change hue to green
		hue[0][0] = 0.0f;
		hue[1][1] = 1.0f;
	}
}

// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Draw the Gameobjects
	tetrahedron->Draw();
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
			selectedShape = selectedShape == line ? tetrahedron : line;

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
	//Create window
	window = glfwCreateWindow(800, 800, "Convex Hull - Line Segment", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate tetrahedral mesh
	struct Vertex tetrahedralVerts[12];
	tetrahedralVerts[0] = { 0.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[1] = {-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[2] = {-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[3] = { 1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[4] = { 1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[5] = { 0.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[6] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[7] = { 0.0f,-1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[8] = {-1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[9] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[10] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };
	tetrahedralVerts[11] = { 1.0f,-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };	

	tetrahedron = new struct Mesh(12, tetrahedralVerts, GL_LINES);

	//Scale the tetrahedron
	tetrahedron->scale = tetrahedron->scale * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
	//Position the tetrahedron
	tetrahedron->translation = tetrahedron->translation * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));


	//Generate the line mesh
	struct Vertex lineVerts[2];
	lineVerts[0] = {-1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
	lineVerts[1] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };


	line = new struct Mesh(2, lineVerts, GL_LINES);

	//Scale the line
	line->scale = line->scale * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
	//Position the line
	line->translation = line->translation * glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));

	//Set the selected shape
	selectedShape = tetrahedron;

	//Generate the tetrahedron's convex hull
	tetraHull = new struct ConvexHull(*tetrahedron);
	//Generate the lines's collider
	lineCollider = new struct Line(glm::vec3(lineVerts[0].x, lineVerts[0].y, lineVerts[0].z), glm::vec3(lineVerts[1].x, lineVerts[1].y, lineVerts[1].z));

	//Print Controls
	std::cout << "Controls:\nUse WASD to move the selected object in the XY plane.\nUse left shift & left CTRL to move selected object along the Z axis.\n";
	std::cout << "Left click & drag the mouse to rotate the selected object.\nUse spacebar to swap the selected object.\n";

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

	delete tetrahedron;
	delete line;

	delete tetraHull;
	delete lineCollider;

	// Frees up GLFW memory
	glfwTerminate();
}