/*
Title: Finite Element Method (1D)
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
This is a demonstration of using the finite element method to simulate deformable body physics.
The demo contains a beam made from 10 nodes which can be compressed and stretched.

The finite element method is an advanced, physically correct, and intensive numerical
algorithm used to take a complex problem and model it with a system of small simple problems
to approximate a solution.

Because of the given limitations of this example, we are able to pre-compute
most of the information needed at the startup of the program (notice the long startup time).
This means each physics timestep we simply solve a system of equations using the pre-computed information
and interpolate each nodes position using harmonic oscillation equations to simulate
the deformation of the body to an equilibrium state after external forces are applied.

The user can apply forces to the right end of the beam.
Hold the left mouse button to apply a force along the positive X axis.
Hold the right mouse button to apply a force along the negative X axis.

References:
Finite Element Analysis (MCEN 4173/5173) Fall 2006 course materials from University of Colorado Boulder as taught by Dr. H. "Jerry" Qi
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
*/

#include "GLIncludes.h"
#include "Matrix.h"

#define PI 3.14159f

// Global data members
#pragma region Base_data
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;
GLuint uniHue;

// Matrix for storing the View Projection transformation
glm::mat4 VP;

// This is a matrix to be sent to the shaders which can control global hue alteration
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
	GLuint EBO;
	GLuint VAO;
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 scale;
	int numVertices;
	int numIndices;
	struct Vertex* vertices;
	GLuint* indices;
	GLenum primitive;

	Mesh::Mesh(int numVert, struct Vertex* vert, int numInd, GLuint* inds, GLenum primType)
	{

		glm::mat4 translation = glm::mat4(1.0f);
		glm::mat4 rotation = glm::mat4(1.0f);
		glm::mat4 scale = glm::mat4(1.0f);

		this->numVertices = numVert;
		this->vertices = new struct Vertex[this->numVertices];
		memcpy(this->vertices, vert, this->numVertices * sizeof(struct Vertex));

		this->numIndices = numInd;
		this->indices = new GLuint[this->numIndices];
		memcpy(this->indices, inds, this->numIndices * sizeof(GLuint));

		this->primitive = primType;

		//Generate VAO
		glGenVertexArrays(1, &this->VAO);
		//bind VAO
		glBindVertexArray(VAO);

		//Generate VBO & EBO
		//We must use an element buffer here so that we do not need to worry about duplicate vertices
		//while we are repositioning the vertices of the mesh
		glGenBuffers(1, &this->VBO);
		glGenBuffers(1, &this->EBO);

		//Configure VBO & EBO
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * this->numVertices, this->vertices, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * this->numIndices, this->indices, GL_STATIC_DRAW);

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

	void Mesh::RefreshData(void)
	{
		//BEcause we are changing the vertices themselves and not transforming them
		//We must write the new vertices over the old on the GPU.
		glBindVertexArray(VAO);

		GLvoid* memory = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(memory, vertices, numVertices * sizeof(Vertex));
		glUnmapBuffer(GL_ARRAY_BUFFER);
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
		//glDrawArrays(this->primitive, 0, this->numVertices);
		glDrawElements(this->primitive, this->numIndices, GL_UNSIGNED_INT, 0);
	}
};

//A struct for 1D FEM deformable solid body
struct SoftBody
{
	int numNodes;			//Number of nodes in the finite element model
	int anchoredNode;		//Index of the node which is restricted by movement

	float* forceX;			//Force on each node in the X direction
	float* initDisp;		//Initial displacement of each node in the X direction

	float** posX;			//Position of each node in the X direction
	float* velX;			//Velocity of each node in the X direction
	float* accX;			//Acceleration of each node in the X direction

	float* finalPosX;		//Position of each node once deformation has completed
	float* lastPosX;		//Position of each node at the time that the last change in external forces occurred

	float* angularFrequency;//Angular frequency of the harmonic oscillator which governs each node's behavior over time

	float totalMass;		//Total mass of the object
	float nodalMass;		//Mass of each node (Here we assume the object is of uniform mass, this could become an array of nodal masses if it is not.

	float youngsModulus;	//The young's modulus of the material

	float* deformationTime;	//Time until object is completely deformed due to last applied force
	float* deformationTimer;//Time since the object began deforming


	Matrix* boundedInverseMatrix;


	//Default constructor.. Do not use.
	SoftBody::SoftBody()
	{
	}

	///
	//Parameterized constructor
	//
	//Parameters:
	//	m: The mesh to build the 1D deformable body from
	//	nNodes: The number of nodes in the finite element model
	//	youngsMod: The young's modulus of the material which the solid is made from
	//	tMass: The total mass of the solid
	//	boundaryNode: The node which is fixed in place as a boundary condition
	SoftBody::SoftBody(const Mesh& m, int nNodes, float youngsMod, float tMass, int boundaryNode)
	{
		//Set the number Finite Element Model properties
		numNodes = nNodes;
		anchoredNode = boundaryNode;

		totalMass = tMass;
		nodalMass = totalMass / ((float)numNodes);

		youngsModulus = youngsMod;

		//Initialize arrays
		forceX = new float[numNodes];
		initDisp = new float[numNodes];

		posX = new float*[numNodes];
		velX = new float[numNodes];
		accX = new float[numNodes];

		deformationTime = new float[numNodes];
		deformationTimer = new float[numNodes];

		finalPosX = new float[numNodes];
		lastPosX = new float[numNodes];

		angularFrequency = new float[numNodes];

		//Assume each node in the finite element model is in an element with the previous node
		//and in a separate element with the next node
		for(int i = 0; i < numNodes; i++)
		{
			posX[i] = (float*)(m.vertices + i);		//Set the position of the ith node
			lastPosX[i] = initDisp[i] = *posX[i];	//Set this position as the equilibrium of this node with no external forces
			
			//Set kinematic values to 0
			velX[i] = 0.0f;
			accX[i] = 0.0f;
			forceX[i] = 0.0f;

			//Set the deformation timer to 0
			deformationTimer[i] = 0.0f;
		}

		//Calculate indices in the global stiffness matrix
		//The global stiffness matrix will look like this:
		//
		//	K11  K12  K13  ...  K1N
		//  K21  K22  ...       :
		//  K31  :    .         :
		//  :    :       .
		//  :               .
		//  KN1  ...            KNN
		//
		//Where Kij is the stiffness coefficient governing the elasticity equation between the ith and jth node such that
		//F = -(Kij) * X
		//Where F is the force applied on the ith node due to it's displacement from the jth node
		//and X is the difference in the displacement of the ith and jth nodes from their initial resting positions.
		//
		//In our 1D case, we will end up with a stiffness matrix:
		//
		//	K1		-K1		0		...		0
		// -K1		K1+K2	-K2		0		:
		//  0		-K2		K2+K3	-K3		:
		//  :		0		-K3		.	.	.
		//  :		:		0		.	.	-KN
		//  0  ...  0		...		.	-KN	KN
		//
		//Later we will apply this matrix to a nodal displacement vector, d
		//This vector will contain each nodes displacement from it's initial equilibrium position set above.
		//When we perform this multiplication, we will get a global forces vector, F.
		//
		//Let us perform the first row of this matrix multiplication as an example
		//F = K1 * d1 - K1 * d2
		//F = K1 * (d1 - d2)
		//F = K1 * -(d2 - d1)
		//F = -K1 * X
		//
		//As you can see this fulfills our equation above:
		//	F = -(Kij) * X
		//Properly solving for the forces on the first node due to its displacement from the second node.
		//Each subsequent row will fulfill the same requirement.
		Matrix* globalStiffnessMatrix = Matrix_Allocate();
		Matrix_Initialize(globalStiffnessMatrix, numNodes, numNodes);

		//Start at Zero matrix
		Matrix_Scale(globalStiffnessMatrix, 0.0f);

		//For each node
		for(int i = 0; i < numNodes - 1; i++)
		{
			//Calculate the stiffness coefficient between this node and the next node as
			//k = AE/L
			//
			//We assume the cross sectional area is 1 here, but if it wasn't you could apply a cross sectional area here.
			float k = youngsModulus / (initDisp[i + 1] - initDisp[i]);

			//Calculate the angular frequency of the harmonic oscillator which will govern the behavior of this node over time due to the ith element
			//w = sqrt(K/M)
			angularFrequency[i] = sqrtf(k / nodalMass);

			//Calculate the time it will take this harmonic oscillator to reach maximum compression / elongation when a constant force is applied
			//T = 2PI/w
			//However, the maximum compression will occur at exactly 1/4th of the total period given an offset of -PI/2 (starting from equilibrium)
			//MaxT = PI/(2w)
			deformationTime[i] = PI/(2.0f * angularFrequency[i]);

			//Set the ith element's values in the global stiffness matrix
			*Matrix_Index(globalStiffnessMatrix, i, i) += k;
			*Matrix_Index(globalStiffnessMatrix, i, i+1) -= k;
			*Matrix_Index(globalStiffnessMatrix, i+1, i) -= k;
			*Matrix_Index(globalStiffnessMatrix, i+1, i+1) += k;
		}

		//Unfortunately, the global stiffness matrix is an indeterminant matrix (Det(Global stiffness Matrix) = 0)
		//This means that no inverse exists. Because of this we must apply a boundary condition.
		//A boundary condition is a prescribed condition on the system which we know will remain true in order to solve the equation.
		//In this simulation the boundary condition is the fact that the left-most node is pinned in place and will not move.
		//This makes our equation:
		//	F = -K * X
		//Simpler, because we guarantee the first component of X will always be 0.
		//Because this will always be 0, we no longer have a need for the first column of our global stiffness matrix, K.
		//This makes our global stiffness matrix:
		//
		//	0		-K1		0		...		0
		//  :		K1+K2	-K2		0		:
		//  :		-K2		K2+K3	-K3		:
		//  :		0		-K3		.	.	0
		//  :		:		0		.	.	-KN
		//  0  ...  0		...		0	-KN	KN
		//
		//Also, if the first node will never be displaced, it must be such that the first node will never have a 
		//non-zero net force on it. As we showed above, the ith row of the matrix is responsible for calculating the
		//net force on the ith node. Therefore, the first row of our matrix (Or the row corresponding to the node
		//subject to the boundary condition) can be set to 0 without changing the results!
		//
		//This leaves us with:
		//
		//	0		...		0		...		0
		//  :		K1+K2	-K2		0		:
		//  :		-K2		K2+K3	-K3		:
		//  :		0		-K3		.	.	0
		//  :		:		0		.	.	-KN
		//  0  ...  0		...		0	-KN	KN
		//
		//Now we can create a bounded stiffness matrix with 1 less row and column of our global stiffness matrix
		//And we can pull out the row and column corresponding to the node with the boundary condition.
		//The significance of this matrix is that it now has a non-zero determinant which means it's inverse exists.
		//Create the bounded stiffness matrix
		Matrix* boundedStiffnessMatrix = Matrix_Allocate();
		Matrix_Initialize(boundedStiffnessMatrix, numNodes - 1, numNodes - 1);
		
		//Apply the boundary condition & get the bounded stiffness matrix
		Matrix_GetMinor(boundedStiffnessMatrix, globalStiffnessMatrix, anchoredNode, anchoredNode);

		//In our equation we are not going to be solving for the forces, we want to solve for the resulting displacements of all
		//nodes because of a force applied on a single node. To do this we need the inverse of our bounded stiffness matrix.
		//Determine the inverse of the bounded stiffness matrix
		boundedInverseMatrix = Matrix_Allocate();
		Matrix_Initialize(boundedInverseMatrix, numNodes - 1, numNodes - 1);
		Matrix_GetInverse(boundedInverseMatrix, boundedStiffnessMatrix);

		Matrix_Free(globalStiffnessMatrix);
		Matrix_Free(boundedStiffnessMatrix);

	}

	SoftBody::~SoftBody()
	{
		delete[] forceX;
		delete[] initDisp;

		delete[] posX;
		delete[] velX;
		delete[] accX;

		delete[] finalPosX;
		delete[] lastPosX;

		delete[] deformationTime;
		delete[] deformationTimer;

		delete[] angularFrequency;

		Matrix_Free(boundedInverseMatrix);
	}
};

struct Mesh* lattice;

struct SoftBody* body;

double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.

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

//Creates shader program
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

	// Create shaders
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

	//Create uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glPointSize(5.0f);
}


#pragma endregion Helper_functions

// This runs once every physics timestep.
void update(float dt)
{	
	//Step 0: Calculate external force on a node(s) if there is any
	Vector externalForce;
	Vector_INIT_ON_STACK(externalForce, 3);

	if(glfwGetMouseButton(window, 0) == GLFW_PRESS)
	{
		*(externalForce.components) = 5.0f;
	}
	if(glfwGetMouseButton(window, 1) == GLFW_PRESS)
	{
		*(externalForce.components) = -5.0f;
	}

	//Step 1: Construct the nodal displacement vector
	Vector* nodalDisp = Vector_Allocate();
	Vector_Initialize(nodalDisp, body->numNodes - 1);

	int node = 0;
	for(int i = 0; i < body->numNodes; i++)
	{
		if(i != body->anchoredNode)
		{
			nodalDisp->components[node] = body->initDisp[i] - *body->posX[i];
			++node;
		}
	}

	//Step 2: Construct the global forces vector
	Vector* forces = Vector_Allocate();
	Vector_Initialize(forces, body->numNodes - 1);

	forces->components[body->numNodes - 2] = *(externalForce.components);

	//Step 3: Calculate the nodal displacement vector to achieve these forces
	Matrix_GetProductVector(nodalDisp, body->boundedInverseMatrix, forces);

	//Step 4: save final position
	node = 0;
	for(int i = 0; i < body->numNodes; i++)
	{
		if(i != body->anchoredNode)
		{
			//If there was a change in final position, reset the timer
			float fPos = nodalDisp->components[node] + body->initDisp[i];
			if(fPos != body->finalPosX[i])
			{
				body->lastPosX[i] = *body->posX[i];
				body->finalPosX[i] = fPos;
				body->deformationTimer[i] = 0.0f;
			}

			//Step 5: Interpolate to final equilibrium position using harmonic oscillation formulas
			body->deformationTimer[i] += dt;
			if(body->deformationTimer[i] > body->deformationTime[i]) body->deformationTimer[i] = body->deformationTime[i];

			//Use harmonic oscillator to move nodes to final position
			//X = A * cos(wt + phi)
			*body->posX[i] = (body->finalPosX[i] - body->lastPosX[i]) * cosf(body->angularFrequency[i] * body->deformationTimer[i] - PI/2.0f) + body->lastPosX[i];

			++node;
		}
	}

	//Step 6: Free memory
	Vector_Free(nodalDisp);
	Vector_Free(forces);
}

// This runs once every frame to determine the FPS and how often to call update based on the physics step.
void checkTime()
{
	// Get the current time.
	time = glfwGetTime();

	// Get the time since we last ran an update.
	double dt = time - timebase;

	// If more time has passed than our physics timestep.
	if (dt > physicsStep)
	{

		timebase = time; // set new last updated time

		// Limit dt
		if (dt > 0.25)
		{
			dt = 0.25;
		}
		accumulator += dt;

		// Update physics necessary amount
		while (accumulator >= physicsStep)
		{
			update(physicsStep);
			accumulator -= physicsStep;
		}
	}
}



// This function runs every frame
void renderScene()
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(1.0f);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//Set hue uniform
	glUniformMatrix4fv(uniHue, 1, GL_FALSE, glm::value_ptr(hue));

	//Refresh the rope vertices
	lattice->RefreshData();
	// Draw the Gameobjects
	lattice->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Finite Element Method 1D", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	const int subX = 9;

	//Generate the element mesh
	float latticeArr[subX * (sizeof(struct Vertex) / sizeof(float))];
	for(int j = 0; j < subX; j++)
	{
		int offset = sizeof(struct Vertex) / sizeof(float);
		int index = j * offset;
		Vertex* v = (Vertex*)(latticeArr + index);
		v->x = (1.0f / subX) * (float)j;
		v->y = 0.0f;
		v->z = 0.0f;

		v->r = 0.0f;
		v->g = 1.0f;
		v->b = 1.0f;
		v->a = 1.0f;
	}

	Vertex *latticeVerts = (Vertex*)latticeArr;

	const int numIndices = (subX - 1);
	GLuint latticeElems[numIndices];
	for(int i = 0; i < subX - 1; i++)
	{
		latticeElems[i] = i;
	}


	//lattice creation
	lattice = new struct Mesh(subX , latticeVerts, numIndices, latticeElems, GL_POINTS);

	//Scale the lattice
	lattice->scale = glm::scale(lattice->scale, glm::vec3(1.0f));
	//Translate truss
	lattice->translation = glm::translate(lattice->translation, glm::vec3(-0.8f, 0.0f, 0.0f));

	//Set spring constant, rest length, and dampening constant
	float coeff = 10.0f;
	float damp = 0.75f;

	//Generate the softbody
	body = new SoftBody(*lattice, subX, coeff, 100.0f, 0);

	//Print controls
	printf("Controls:\nPress and hold the left mouse button to apply a positive constant force\n on the right-most node.\n");
	printf("Press and hold the right mouse button to apply a negative constant force\n on the right most node.\n");


	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		//Check time will update the programs clock and determine if & how many times the physics must be updated
		checkTime();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// After the program is over, cleanup your data!
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);
	// Note: If at any point you stop using a "program" or shaders, you should free the data up then and there.

	delete lattice;
	delete body;


	// Frees up GLFW memory
	glfwTerminate();
}