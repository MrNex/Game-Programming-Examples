/*
Title: Convex Hull (SAT - 3D)
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
This is a demonstration of convex hull collision detection between two convex polygons.
The demo contains a wireframe of a frustum and a tetrehedron. Both appear green.
You can move the shapes around the X-Y plane with WASD, and along the Z axis with
Left Shift & Left Control. Pressing space will toggle the shape being moved.
You can also rotate the selected shape by holding the left mouse button and dragging the mouse.

This demo uses the separating axis theorem test for collision detection. The
Theorem states that if you are able to separate two polygons by a plane then 
they must not be colliding. To test this, we develop a collection of potential axes which
the shapes might overlap if projected upon, and if each axis in our collection detects
an overlap between the shapes once they are projected onto it, there must be a collision.
However if there is a single axis which does not detect overlap then there must not be a collision.
The collection of necessary axes to test include the face normals of both polygons as well as the
edge normals which are also normal to an edge on the opposite polygon.

References:
Base by Srinivasan Thiagarajan
AABB-2D by Brockton Roth
NGen by Nicholas Gallagher
*/

#include "GLIncludes.h"

// Global data members
#pragma region Base_data
//Shader
GLuint program;
GLuint vertex_shader;
GLuint fragment_shader;

//Uniform
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

struct Edge
{
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::vec3 direction;
};

struct ConvexHull
{
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> edgeDirections;
	std::vector<glm::vec3> normals;

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

		//Loop through all mesh vertices and add them and
		//the edges which they make up to the convex hull points if they are not duplicates
		for (int i = 0; i < m.numVertices; i+=2)
		{
			//Construct a vec3 to represent the ith vertex
			glm::vec3 currentPoint(m.vertices[i].x, m.vertices[i].y, m.vertices[i].z);
			
			//Check if it is a duplicate
			bool contains = false;
			int size = points.size();
			for (int j = 0; j < size; j++)
			{
				if (points[j] == currentPoint)
				{
					contains = true;
					break;
				}
			}
			
			//If it is not a duplicate add it to the list
			if (!contains)
				points.push_back(currentPoint);

			//Construct a vec3 to represent the (i + 1)th vertex
			glm::vec3 nextPoint(m.vertices[i + 1].x, m.vertices[i + 1].y, m.vertices[i + 1].z);
			//Check if it's a duplicate
			contains = false;
			for (int j = 0; j < size; j++)
			{
				if (points[j] == nextPoint)
				{
					contains = true;
					break;
				}
			}
			//If it is not a duplicate add it to the list
			if (!contains)
				points.push_back(nextPoint);

			//Generate an edge from the two points
			struct Edge edge = 
			{
				currentPoint,
				nextPoint,
				glm::normalize(nextPoint - currentPoint)
			};

			//Add it to the list of edges
			edges.push_back(edge);

			//Check if the edges direction is contained in the list of edge directions yet
			contains = false;
			size = edgeDirections.size();
			for (int j = 0; j < size; j++)
			{
				if (edgeDirections[j] == edge.direction)
				{
					contains = true;
					break;
				}
			}

			//If it is not a duplicate add it to the list
			if (!contains)
				edgeDirections.push_back(edge.direction);
		}

		//We must loop through all edges and find the co-planar ones to take the cross product
		//And generate face normals for the convex hull
		int size = edges.size();
		for (int i = 0; i < size; i++)
		{
			for (int j = i; j < size; j++)
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
					int nSize = normals.size();
					for (int k = 0; k < nSize; k++)
					{
						if (normals[k] == norm)
						{
							contains = true;
							break;
						}
					}
					
					//If it is not a duplicate, add it to the list
					if (!contains)
						normals.push_back(norm);

				}
			}
		}
	}
};

struct Mesh* tetrahedron;
struct Mesh* frustum;

struct Mesh* selectedShape;

struct ConvexHull* tetraHull;
struct ConvexHull* frustumHull;

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

	//Alter the hue matrix
	//Set global hue to color objects green
	hue[0][0] = 0.0f;
	hue[2][2] = 0.0f;

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

}

#pragma endregion Helper_functions

// Functions called between every frame. game logic
#pragma region util_functions

///
//Tests for the collision between two convex hulls
//
//Overview:
//	This function uses the separating axis theorem test to check for collisions between two convex hull objects.
//	The separating axis theorem states that if two objects are colliding, there is no axis which exists that you can
//	project the shapes onto without recieving overlap.
//
//	This is analogous to testing if two axis aligned boxes overlap on the X, Y, and Z axes, however in that case no projection is necessary
//	because the shapes are already being represented in terms of the X, Y, and Z axes. By projecting the shapes we can determine how to represent
//	their corners in terms of a different coordinate system. In this test we will be utilizing the scalar projection instead of the projection.
//	The scalar projection, instead of giving us a vector representation of the coordinate after the projection, will give us a scalar value representing
//	how far the point is in the direction of a given axis.
//
//	By comparing the maximum and minimum scalar projection values of both shapes on an arbitrary axis we can determine if the shapes overlap on that axis.
//	The separating axis theorem test uses the fact that the only necessary axes to test are the face and edge normals of both shapes.
//
//Parameters:
//	hull1: The first convex hull being tested
//	trans1: The translation transformation matrix positioning hull1 in worldspace
//	rotation1: The rotation transformation matrix orienting hull1 in worldspace
//	scale1: The scale translation matrix sizing hull1 in worldspace
//	hull2: The second convex hull being tested
//	trans2: The translation transformation matrix positioning hull2 in worldspace
//	rotation2: The rotation transformation matrix orienting hull2 in worldspace
//	scale2: The scale transformation matrix sizing hull2 in worldspace
bool TestCollision(
	const ConvexHull &hull1, const glm::mat4 &trans1, const glm::mat4 &rotation1, const glm::mat4 &scale1,
	const ConvexHull &hull2, const glm::mat4 &trans2, const glm::mat4 &rotation2, const glm::mat4 &scale2)
{
	//Step 1: In order to perform the test we must calculate the orientation of the face normals and edges in world space
	//	As well as the corners once transformed into world space
	
	//Normals and edges of hull1
	std::vector<glm::vec3> worldNormals1;
	std::vector<glm::vec3> worldEdges1;
	
	int size = hull1.normals.size();
	glm::mat3 rotation = glm::mat3(rotation1);
	for (int i = 0; i < size; i++)
	{
		worldNormals1.push_back(rotation * hull1.normals[i]);
	}

	size = hull1.edgeDirections.size();
	for (int i = 0; i < size; i++)
	{
		worldEdges1.push_back(rotation * hull1.edgeDirections[i]);
	}
	
	//Normals and edges of hull2
	std::vector<glm::vec3> worldNormals2;
	std::vector<glm::vec3> worldEdges2;

	size = hull2.normals.size();
	rotation = glm::mat3(rotation2);
	for (int i = 0; i < size; i++)
	{
		worldNormals2.push_back(rotation * hull2.normals[i]);
	}

	size = hull2.edgeDirections.size();
	for (int i = 0; i < size; i++)
	{
		worldEdges2.push_back(rotation * hull2.edgeDirections[i]);
	}

	//Points of hull1
	std::vector<glm::vec3> worldPoints1;

	size = hull1.points.size();
	glm::mat4 modelMatrix = trans1 * rotation1 * scale1;
	for (int i = 0; i < size; i++)
	{
		glm::vec4 worldPoint = modelMatrix * glm::vec4(hull1.points[i], 1.0f);
		worldPoints1.push_back(glm::vec3(worldPoint));
	}

	//Points of hull2
	std::vector <glm::vec3> worldPoints2;

	size = hull2.points.size();
	modelMatrix = trans2 * rotation2 * scale2;
	for (int i = 0; i < size; i++)
	{
		glm::vec4 worldPoint = modelMatrix * glm::vec4(hull2.points[i], 1.0f);
		worldPoints2.push_back(glm::vec3(worldPoint));
	}

	//Step 2:
	//	Now that we have the information needed to perform the test we will begin by testing the axes represented by the face normals
	//	We must test the face normals of both hulls, but we will start with hull1
	size = worldNormals1.size();
	//For each normal in worldspace
	for (int i = 0; i > size; i++)
	{
		//we must find the minimum & maximum scalar projection value given by the points of hull1 being projected onto the ith face normal
		float min1, max1;
		//A projection of a vector X onto another vector Y can be given by the following formula:
		//	Proj(X, Y) = ((X . Y) / (Y . Y)) * Y
		//Where " . " represents the dot product operation.
		//But because we know the direction of the projection will be in the direction of Y, we can represent a scalar projection
		//(Or the amount a vector X travels in the direction of Y) as:
		//	SProj(X, Y) = (X . Y) / (Y . Y)

		//Luckily for us, we know that the dot product of a vector with itself is equal to the magnitude of the vector squared.
		//And our axes (Y in the above equation) are all normalized, meaning they have a magnitude of 1. Therefore:
		//	SProj(X, Y) = (X . Y) / (1 * 1)
		//	SProj(X, Y) = X . Y

		//We have our equation so we will start by setting min1 and max1 to the scalar projection of the first point on the ith axis
		min1 = max1 = glm::dot(worldPoints1[0], worldNormals1[i]);

		//Next we must find the rest of the scalar projections of the points of hull1 onto the ith axis / normal of hull1, keeping only the minimum and maximum
		int pSize = worldPoints1.size();
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(worldPoints1[j], worldNormals1[i]);
			if (sProj < min1) min1 = sProj;
			else if (sProj > max1) max1 = sProj;
		}

		//Now we perform the same algorithm on the points of hull2
		float min2, max2;
		min2 = max2 = glm::dot(worldPoints2[0], worldNormals1[i]);
		pSize = worldPoints2.size();
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(worldPoints2[j], worldNormals1[i]);
			if (sProj < min2) min2 = sProj;
			else if (sProj > max2) max2 = sProj;
		}

		//Finally we check for overlap on the ith axis, if the max and mins of both hulls overlap, there is a collision on this axis.
		//If they do not overlap, we immediately know there cannot be a collision and we can stop testing.
		if (min1 < max2 && max1 > min2) continue;
		else return false;
	}	//End hull 1 normals test

	//Once we have checked all of the normals of hull1, we must do the same for hull2.
	size = worldNormals2.size();
	//For each normal in worldspace
	for (int i = 0; i > size; i++)
	{
		//we must find the minimum & maximum scalar projection value given by the points of hull1 being projected onto the ith face normal
		float min1, max1;

		//We have our equation so we will start by setting min1 and max1 to the scalar projection of the first point on the ith axis
		min1 = max1 = glm::dot(worldPoints1[0], worldNormals2[i]);

		//Next we must find the rest of the scalar projections of the points of hull1 onto the ith axis / normal of hull2, keeping only the minimum and maximum
		int pSize = worldPoints1.size();
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(worldPoints1[j], worldNormals2[i]);
			if (sProj < min1) min1 = sProj;
			else if (sProj > max1) max1 = sProj;
		}

		//Now we perform the same algorithm on the points of hull2
		float min2, max2;
		min2 = max2 = glm::dot(worldPoints2[0], worldNormals2[i]);
		pSize = worldPoints2.size();
		for (int j = 1; j < pSize; j++)
		{
			float sProj = glm::dot(worldPoints2[j], worldNormals2[i]);
			if (sProj < min2) min2 = sProj;
			else if (sProj > max2) max2 = sProj;
		}

		//Finally we check for overlap on the ith axis, if the max and mins of both hulls overlap, there is a collision on this axis.
		//If they do not overlap, we immediately know there cannot be a collision and we can stop testing.
		if (min1 < max2 && max1 > min2) continue;
		else return false;
	}	//End hull 2 normals test

	//Step 3: If the code reaches this point we can assume a collision was registered on all axes corresponding to the face normals of either hull.
	//	Once we have checked all possible face normals for a collision we must check one final case, the edge normals.
	//	The problem which arises is an edge is represented by a line, and lines in 3-D have an infinite amount of normals! In fact, the subspace
	//	considered to be "normal" (or orthogonal) to a line is a plane, and planes do not have a single direction we can project onto.
	//	The solution is not to consider the subspace orthogonal to an edge, but the subspace which is orthogonal to two edges on opposite hulls!
	//	If you consider two planes in 3-D, so long as the planes are not parallel they must intersect in line! If we generate all possible lines
	//	created by the intersection of the subspaces orthogonal to each edge on hull1 with each edge on hull2 we will have all necessary edge normals
	//	for testing whether or not there is a collision.
	//
	//	It turns out, these intersections of the orthogonal subspaces of two edges can be easily found by taking the cross product of the two edges.
	//	So the last set of axes we must test is the set of axes created by the cross product of each edge on hull1 with each edge on hull2.

	//We will start by looping through the edges of hull1
	size = worldEdges1.size();
	int size2 = worldEdges2.size();
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size2; j++)
		{
			//Get cross product of two edges
			glm::vec3 axis = glm::cross(worldEdges1[i], worldEdges2[j]);
			//Make sure the axis is not the zero vector
			if (glm::length(axis) != 0.0f)
			{
				//Perform the test on the new axis
				//we must find the minimum & maximum scalar projection value given by the points of hull1 being projected onto the axis
				float min1, max1;

				//We have our equation so we will start by setting min1 and max1 to the scalar projection of the first point on the ith axis
				min1 = max1 = glm::dot(worldPoints1[0], axis);

				//Next we must find the rest of the scalar projections of the points of hull1 onto the ith axis / normal of hull2, keeping only the minimum and maximum
				int pSize = worldPoints1.size();
				for (int k = 1; k < pSize; k++)
				{
					float sProj = glm::dot(worldPoints1[k], axis);
					if (sProj < min1) min1 = sProj;
					else if (sProj > max1) max1 = sProj;
				}

				//Now we perform the same algorithm on the points of hull2
				float min2, max2;
				min2 = max2 = glm::dot(worldPoints2[0], axis);
				pSize = worldPoints2.size();
				for (int k = 1; k < pSize; k++)
				{
					float sProj = glm::dot(worldPoints2[k], axis);
					if (sProj < min2) min2 = sProj;
					else if (sProj > max2) max2 = sProj;
				}

				//Finally we check for overlap on the ith axis, if the max and mins of both hulls overlap, there is a collision on this axis.
				//If they do not overlap, we immediately know there cannot be a collision and we can stop testing.
				if (min1 < max2 && max1 > min2) continue;
				else return false;
			}
		}
	}

	//If the code reaches this point we have detected a collision on every tested axis, therefore there is a collision.
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

	if (TestCollision(*tetraHull, tetrahedron->translation, tetrahedron->rotation, tetrahedron->scale, *frustumHull, frustum->translation, frustum->rotation, frustum->scale))
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
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	// Draw the Gameobjects
	tetrahedron->Draw();
	frustum->Draw();
}


// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		//This selects the active shape
		if (key == GLFW_KEY_SPACE)
			selectedShape = selectedShape == frustum ? tetrahedron : frustum;

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
	window = glfwCreateWindow(800, 800, "Convex Hull (SAT - 3D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//TODO: Generate tetrahedral mesh
	struct Vertex tetrahedralVerts[12];
	tetrahedralVerts[0] = { 0.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[1] = {-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[2] = {-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[3] = { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[4] = { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[5] = { 0.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[6] = { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[7] = { 0.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[8] = {-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[9] = { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[10] = { 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	tetrahedralVerts[11] = { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };	

	tetrahedron = new struct Mesh(12, tetrahedralVerts, GL_LINES);

	//Scale the tetrahedron
	tetrahedron->scale = tetrahedron->scale * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
	//Position the tetrahedron
	tetrahedron->translation = tetrahedron->translation * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));


	//Generate the frustum mesh
	struct Vertex frustumVerts[24];
	frustumVerts[0] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[1] = {-0.5f, 0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[2] = {-0.5f, 0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[3] = {-0.5f,-0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[4] = {-0.5f,-0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[5] = {-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[6] = {-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[7] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[8] = {-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[9] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[10] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[11] = { 0.5f, 0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[12] = { 0.5f, 0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[13] = {-0.5f, 0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[14] = { 0.5f, 0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[15] = { 0.5f,-0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[16] = { 0.5f,-0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[17] = { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[18] = { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[19] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[20] = { 1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[21] = {-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[22] = { 0.5f,-0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	frustumVerts[23] = {-0.5f,-0.5f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	frustum = new struct Mesh(24, frustumVerts, GL_LINES);

	//Scale the frustum
	frustum->scale = frustum->scale * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
	//Position the frustum
	frustum->translation = frustum->translation * glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));

	//Set the selected shape
	selectedShape = tetrahedron;

	//Generate the tetrahedron's convex hull
	tetraHull = new struct ConvexHull(*tetrahedron);
	//Generate the frustum's convex hull
	frustumHull = new struct ConvexHull(*frustum);

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

	delete tetrahedron;
	delete frustum;

	delete tetraHull;
	delete frustumHull;

	// Frees up GLFW memory
	glfwTerminate();
}