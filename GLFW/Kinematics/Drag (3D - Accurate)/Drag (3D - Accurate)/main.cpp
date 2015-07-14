/*
Title: Drag (3D - Accurate)
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
This is a demonstration of calculating and applying form drag. The demo contains a yellow triangle. 
The triangle has a constant force being applied to it and will continue to accelerate.
Because of drag, the triangle will eventually hit its terminal velocity and stabilize at a constant velocity.

The algorithm Calculates & applies approximate drag due to an object's form. This is done through
the use of the Drag Equation discovered by Lord Rayleigh. This takes into account various
physical attributes such as a Drag Coefficient, cross-sectional length, the density of the medium,
and the relative velocity between the medium and the object. This is an accurate but not as efficient form of the algorithm.
The cross sectional area is accurately computed by first reducing the convex hull to a 2D projection, then
the signed area is calculated of that.

References:
Introduction to Algorithms by Thomas H. Hormen, Charles E. Leiserson, Ronald L. Rivest, and Clifford Stein
Ultra-Easy Polygon Area Algorithm With C Code Sample by Darel Rex Finley
PhysicsTimestep by Brockton Roth
Base by Srinivasan Thiagarajan
*/

#include "GLIncludes.h"

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

//A convex hull collider (3D) struct
struct ConvexHull
{
	//The points which make up this polygon
	std::vector<glm::vec3> vertices;	//For calculating drag we only need the vertices, not the edge normals.

	//Current orientation of the collider
	glm::mat3 rotation;
	//Current scale of the collider
	glm::mat3 scale;
	//The position is not needed for calculating drag

	///
	//Default constructor construct a basic triangle
	ConvexHull::ConvexHull()
	{
		vertices.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
		vertices.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
		vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

		rotation = glm::mat3(1.0f);
		scale = glm::mat3(1.0f);
	}

	///
	//Parameterized constructor, constructs a polygon from a vector of points
	ConvexHull::ConvexHull(std::vector<glm::vec3> points, glm::mat3 rotation, glm::mat3 scale)
	{
		vertices = points;
		this->rotation = rotation;
		this->scale = scale;
	}

	///
	//Parameterized constructor, construct a polygon from a mesh
	ConvexHull::ConvexHull(const Mesh &m)
	{
		int i = 0;
		while ( i < m.numVertices)
		{
			glm::vec3 point(m.vertices[i].x, m.vertices[i].y, m.vertices[i].z);
			//MAke sure point is not a duplicate
			int size = vertices.size();
			for(int j = 0; j < size; j++)
			{
				if(point == vertices[j]) goto EndOfOuterLoop;
			}
			this->vertices.push_back(point);

			EndOfOuterLoop: 
			++i;
		}

		this->rotation = glm::mat3(m.rotation);
		this->scale = glm::mat3(m.scale);
	}
};
//Struct for linear kinematics
struct RigidBody
{
	float inverseMass;				//I tend to use inverse mass as opposed to mass itself. It saves lots of divides when forces are involved.
	float inverseMomentOfInertia;	//An objects resistance to rotation
	float dragCoefficient;			//How much is this object affected by drag?

	glm::vec3 position;				//Position of the rigidbody
	glm::vec3 velocity;				//The velocity of the rigidbody
	glm::vec3 acceleration;			//The acceleration of the rigidbody

	glm::mat3 rotation;				//Orientation of rigidbody
	glm::vec3 angularVelocity;		//Angular velocity of rigidbody
	glm::vec3 angularAcceleration;	//Angular acceleration of rigidbody

	glm::vec3 netForce;				//Forces over time
	glm::vec3 netImpulse;			//Instantaneous forces
	float netTorque;				//Torque over time (2D -- only torque which exists is around Z axis)
	float netAngularImpulse;		//Instantaneous torque.

	//Note: The following is only for robustness.. You could get away with leaving these out.
	glm::vec3 previousNetForce;		//Forces over time from prior to the most recent physics update
	glm::vec3 previousNetImpulse;	//Instantaneous forces from prior to the most recent physics update

	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		inverseMass = 1.0f;
		dragCoefficient = 1.0f;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

		rotation = glm::mat3(1.0f);
		angularVelocity = glm::vec3(0.0f);
		angularAcceleration = glm::vec3(0.0f);

		previousNetForce = netForce = glm::vec3(0.0f);
		previousNetImpulse = netImpulse = glm::vec3(0.0f);
		netTorque = 0.0f;
		netAngularImpulse = 0.0f;
	}

	///
	//Parameterized constructor, creates rigidbody with specified initial values
	//
	//Parameters:
	//	pos: Initial position
	//	vel: Initial velocity
	//	acc: Initial acceleration
	//	rot: Initial orientation
	//	aVel: Initial angular velocity
	//	aAcc: Initial angular acceleration
	//	mass: The mass of the rigidbody (0.0f for infinite mass)
	//	drag: the drag coefficient of the object
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, glm::mat3 rot, glm::vec3 aVel, glm::vec3 aAcc, float mass, float drag)	
	{
		inverseMass = mass == 0.0f ? 0.0f : 1.0f / mass;

		position = pos;
		velocity = vel;
		acceleration = acc;

		rotation = rot;
		angularVelocity = aVel;
		angularAcceleration = aAcc;

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);
		netTorque = 0.0f;
		netAngularImpulse = 0.0f;

		dragCoefficient = drag;
	}
};

struct Mesh* tetrahedron;

struct ConvexHull* hull;

struct RigidBody* body;

glm::vec3 constantForce(0.2f, 0.0f, 0.0f);

glm::vec2 minimumTranslationVector;
float overlap;
glm::vec2 pointOfCollision;

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
	glm::mat4 proj = glm::ortho(-1.0f,1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	VP = proj * view;

	//Create uniforms
	uniMVP = glGetUniformLocation(program, "MVP");
	uniHue = glGetUniformLocation(program, "hue");

	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

///
//Calculates the moment of inertia around the Z axis of a thin solid plate with given dimensions and mass
//
//Parameters:
//	width: The width of the plate
//	height: the height of the plate
//	m: The mass of the plate
float CalculateMomentOfInertiaOfRectangle(float width, float height, float m)
{
	return  m * (powf(width, 2.0f)  + powf(height, 2.0f)) / 12.0f;
}


#pragma endregion Helper_functions

///
//Calculate the area of a convex hull. A re-implementation of
//Public-domain function by Darel Rex Finley, 2006.
//
//Parameters:
//	vectors: An array of 2D vectors
//	numVectors: the number of vectors in the array
//
//Returns:
//	the signed area of the polygon
float polygonArea(glm::vec2* vectors, int numVectors) {

  float area=0.0f;
  int i, j=numVectors-1;

  for (i=0; i<numVectors; i++) 
  {
    area += (vectors[j].x + vectors[i].x) * (vectors[j].y - vectors[i].y); 
	j=i; 
  }

  return area * 0.5f; 
}

///
//Accurately Calculates and applied the force due to form drag on an object.
//
//Parameters:
//	fluidDensity: The density of the medium of travel
//	fluidVelocity: The velocity of the medium of travel
//	body: The rigidbody travelling through the medium
//	collider: The collider of the rigidbody travelling through the medium
void CalculateDrag(float fluidDensity, glm::vec3 fluidVelocity, struct RigidBody &body, const struct ConvexHull &collider)
{
	//Step 0: Calculate the relative velocity of the object with respect to the fluid (air) velocity
	glm::vec3 relativeVelocity = body.velocity - fluidVelocity;

	//If there is no relative velocity, there is no drag.
	if(glm::length(relativeVelocity) <= FLT_EPSILON) return;
	//Get the direction
	glm::vec3 relVelDir = glm::normalize(relativeVelocity);


	//Step 1: Compute the vectors which span the subspace of the plane perpendicular to the direction of velocity.
	//For this we will use the Graham Schmidt process.
	glm::vec3 x, y;
	x = glm::vec3(1.0f, 0.0f, 0.0f);
	x = x - glm::dot(relVelDir, x) * relVelDir;
	if(glm::length(x) <= FLT_EPSILON)
	{
		x = glm::vec3(0.0f, 0.0f, 1.0f);
		x = x - glm::dot(relVelDir, x) * relVelDir;
	}
	glm::normalize(x);

	y = glm::vec3(0.0f, 1.0f, 0.0f);
	y = y - glm::dot(relVelDir, y) * relVelDir;
	if(glm::length(y) <= FLT_EPSILON)
	{
		y = glm::vec3(0.0f, 0.0f, 1.0f);
		y = y - glm::dot(relVelDir, y) * relVelDir;
	}
	glm::normalize(y);

	//Step 2: Project the convex hull onto the subspace formed by the basis xy and determine the minimum 2D convex hull created from the projected set of points
	//Instead of performing the full projection of these points, we can determine what is called the coordinate of the projected points.
	//The coordinate can be thought of as a measurement in terms of another basis (A basis is a set of vectors which spans a subspace)
	//For example, if we project some vector U onto the subspace formed by the basis {x, y} which we found above, the coordinate of U'
	//would be some vector V such that V.x * x + v.y * y = U'
	//
	//We can easily calculate this using a matrix who's rows are the vectors in the basis!
	glm::mat2x3 coordinateMatrix
	(
		x,
		y
	);

	std::vector<glm::vec2>coordinates;

	//We should also keep track of the left-most Vertex, as we will need it for the next step
	glm::vec2 pointOnHull = (collider.rotation * collider.scale * collider.vertices[0]) * coordinateMatrix;
	coordinates.push_back(pointOnHull);
	int size = collider.vertices.size();
	for(int i = 1; i < size; i++)
	{
		coordinates.push_back((collider.rotation * collider.scale * collider.vertices[i]) * coordinateMatrix);
		if(coordinates[i].x < pointOnHull.x) pointOnHull = coordinates[i];
	}

	//Once we have all of the coordinates we must compute the convex hull vertices
	//For this we will use the simplistic algorithm known as the Jarvis March
	std::vector<glm::vec2>hullPoints;
	glm::vec2 endPoint;
	//Until the hull loops back around to the leftmost point
	while(hullPoints.size() <= 0 || endPoint != hullPoints[0])
	{
		//Add the point on the hull to the list
		hullPoints.push_back(pointOnHull);
		endPoint = coordinates[0];
		int coordinateSize = coordinates.size();
		for(int i = 1; i < coordinateSize; i++)
		{
			//If the edge connecting the previous point on the hull to the coordinate at index i is
			//a counter clockwise rotation relative to the edge connecting the previous point on the hull to the current endpoint
			//Set the endpoint to the coordinate at index i. This will effectively wrap (clockwise) around the points to create the convex hull.
			glm::vec2 edge1 = coordinates[i] - pointOnHull;
			glm::vec2 edge2 = endPoint - pointOnHull;
			if(endPoint == pointOnHull || edge1.x * edge2.y - edge2.x * edge1.y > FLT_EPSILON)
			{
				endPoint = coordinates[i];
			}
		}
		pointOnHull = endPoint;
	}

	//We now have the points of a convex hull ordered in clockwise order.

	//Step 3: Compute the cross sectional area of the collider, this is the area of the projection of the collider onto the subspace perpendicular to the direction of the velocity.
	//We created a convex hull representing the cross section out of coordinate vectors. The area of this convex hull will be equal to the area of the actual convex hull
	//of the cross section.
	//The fun part is computing the area of a convex hull!
	//Luckily for us, it's not that hard-- thanks to Darel Rex Finley who contributed a function to do just this to the public domain in 2006!
	//Shoutout to Darel Rex Finley.
	float area = fabs(polygonArea(&hullPoints[0], hullPoints.size()));

	//Step 4: calculate the magnitude of the drag force
	//The magnitude of the drag force can be found to be:
	//	DragMag = 0.5f * p * v . v * Cd * A
	//
	//Where p is the fluid density, v is the relative velocity, Cd is the objects drag coefficient, and L is the cross-sectional length
	float dragMag = 0.5f * fluidDensity * glm::dot(relativeVelocity, relativeVelocity) * body.dragCoefficient * area;

	//Step 5: apply the drag force
	//Drag is always opposite the direction of motion
	glm::vec3 dragForce = -relVelDir;
	dragForce *= dragMag;

	body.netForce += dragForce;
}



///
//Performs second order euler integration for linear motion
//
//Parameters:
//	dt: The timestep
//	body: The rigidbody being integrated
void IntegrateLinear(float dt, RigidBody &body)
{
	//Calculate the current acceleration
	body.acceleration = body.inverseMass * body.netForce;

	//Calculate new position with
	//	X = X0 + V0*dt + (1/2) * A * dt^2
	glm::vec3 v0dT = dt * body.velocity;						//Solve for the first degree term
	glm::vec3 aT2 = (0.5f) * body.acceleration * powf(dt, 2);	//Solve for the second degree term
	body.position += v0dT + aT2;								//Take sum

	//determine the new velocity
	body.velocity += dt * body.acceleration + body.inverseMass * body.netImpulse;

	//Set the previous net impulse and net force
	body.previousNetForce = body.netForce;
	body.previousNetImpulse = body.netImpulse;

	//Zero the net impulse and net force!
	body.netForce = body.netImpulse = glm::vec3(0.0f);
}

void IntegrateAngular(float dt, RigidBody &body)
{
	//Calculate new angular acceleration
	body.angularAcceleration = glm::vec3(0.0f, 0.0f, body.netTorque * body.inverseMomentOfInertia);

	//Find change in position with W0 * dt + (1/2)aA * dt ^ 2
	//Where W0 is initial angular velocity, and aA is angular acceleration
	glm::vec3 dr = dt * body.angularVelocity + 0.5f * powf(dt, 2.0f) * body.angularAcceleration;

	float magR = glm::length(dr);
	if(magR > 0.0f)
	{
		glm::mat3 R = glm::mat3(glm::rotate(glm::mat4(1.0f), magR, dr));

		body.rotation = R * body.rotation;

	}
	//body.angularVelocity += dt * body.angularAcceleration + body.netAngularImpulse / body.momentOfInertia;
	body.angularVelocity += dt * body.angularAcceleration + glm::vec3(0.0f, 0.0f, body.netAngularImpulse * body.inverseMomentOfInertia);
	body.netTorque = body.netAngularImpulse = 0.0f;
}

///
//Wraps a moving circle around the edges of the screen
//
//Parameters:
//	body: The rigidbody of the circle
//	circle: The collider of the circle
void Wrap(struct RigidBody &body)
{
	if(body.position.x  < -1.0f)
		body.position.x = 1.0f;
	if(body.position.x > 1.0f)
		body.position.x = -1.0f;
	if(body.position.y < -1.0f)
		body.position.y = 1.0f;
	if(body.position.y > 1.0f)
		body.position.y = -1.0f;
}

// This runs once every physics timestep.
void update(float dt)
{	
	//Apply acceleration due to gravity to the objects
	body->netForce += constantForce;


	//Update the rigidbodies
	IntegrateLinear(dt, *body);
	IntegrateAngular(dt, *body);

	//Move colliders
	hull->rotation = body->rotation;

	//Calculate and apply drag
	CalculateDrag(1.0f, glm::vec3(0.0f), *body, *hull);

	Wrap(*body);

	//translate appearance to new position/orientation
	tetrahedron->translation = glm::translate(glm::mat4(1.0f), body->position);
	tetrahedron->rotation = glm::mat4(tetrahedron->rotation);
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

	// Draw the Gameobjects
	tetrahedron->Draw();
}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Drag (3D - Accurate)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	//Generate the tetrahedron mesh

	float tetrahedralArr[] = { 
		0.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		0.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		0.0f,-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
	};

	
	Vertex *tetrahedralVerts = (Vertex*)tetrahedralArr;

	//tetrahedron creation
	tetrahedron = new struct Mesh(12, tetrahedralVerts, GL_LINES);

	//Scale the tetrahedron
	tetrahedron->scale = glm::scale(tetrahedron->scale, glm::vec3(0.2f));

	//Generate the rigidbody
	body = new RigidBody(
		glm::vec3(-0.75f, 0.0f, 0.0f),		//Initial position
		glm::vec3(0.0f),		//Initial velocity
		glm::vec3(0.0f),					//Zero acceleration
		glm::mat3(1.0f),					//Initial orientation
		glm::vec3(0.0f),		//Initial angular velocity
		glm::vec3(0.0f),					//Initial Angular Acceleration
		1.0f,								//Mass
		1.0f								//Drag coefficient
		);
	body->inverseMomentOfInertia = body->inverseMass == 0.0f ?
		0.0f : 1.0f/CalculateMomentOfInertiaOfRectangle(2.0f, 2.0f, 1.0f / body->inverseMass);

	//Generate the tetrahedron collider
	hull = new ConvexHull(*tetrahedron);


	//Position tetrahedron
	tetrahedron->translation = glm::translate(tetrahedron->translation, body->position);

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

	delete tetrahedron;
	delete hull;
	delete body;

	// Frees up GLFW memory
	glfwTerminate();
}