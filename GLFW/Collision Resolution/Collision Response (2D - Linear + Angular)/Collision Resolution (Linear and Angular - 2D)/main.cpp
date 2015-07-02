/*
Title: Collision Resolution (Linear & Angular - 2D)
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
This is a demonstration resolving a collision between two colliding circles in 2D.
There are Pink and Yellow circles. When the circles collide the collision is resolved in a physically accurate manner.
The yellow circle has twice the mass of the pink circle. Both circles have perfect elasticity resulting in
perfectly elastic collisions.

This demo resolves a collision by calculating the collision impulse needed to apply to both objects in opposite directions (Newtons third law)
in order to simulate a collision between two rigidbodies. We calculate this impulse by using the definition of an impulse- a change in momentum.
In order to calculate the impulse we must first find a final velocity after the collision which we can do using The user can move a circle using WASD.
Newton's law of Restitution. To make all of this easier, we first translate the system to a case where only one of the objects is moving
and relatively the other is still.

References:
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

//A basic polygon consists of a set of points surrounding a center
//which they are rotated about by a given angle.
struct Polygon
{
	glm::vec2 center;					//Center of polygon
	glm::mat2 rotation;					//2x2 rotation matrix
	std::vector<glm::vec2> points;		//list of points which make up polygon (Arranged in order of a counter clockwise loop)
};

struct ConvexHull
{
	std::vector<glm::vec2> points;		//list of points which make up polygon (Arranged in order of a counter clockwise loop)
	std::vector<glm::vec2> normals;		//List of normals of edges between points
	glm::mat2 rotation;					//2x2 rotation matrix
};

//Struct for linear kinematics
struct RigidBody
{
	float inverseMass;			//I tend to use inverse mass as opposed to mass itself. It saves lots of divides when forces are involved.
	float restitution;			//How elastic this object is (1.0f is perfectly elastic, 0.0f is perfectly inelastic)
	float momentOfInertia;		//When dealing with moment of inertia in 2D, only a single float value is needed for resistance of rotation about the Z axis.

	glm::vec3 position;			//Position of the rigidbody
	glm::vec3 velocity;			//The velocity of the rigidbody
	glm::vec3 acceleration;		//The acceleration of the rigidbody

	glm::mat3 rotation;
	glm::vec3 angularVelocity;
	glm::vec3 angularAcceleration;

	glm::vec3 netForce;			//Forces over time
	glm::vec3 netImpulse;		//Instantaneous forces
	float netTorque;			//Torque over time (2D -- only torque which exists is around Z axis)
	float netAngularImpulse;	//Instantaneous torque.


	///
	//Default constructor, created rigidbody with all properties set to zero
	RigidBody::RigidBody()
	{
		inverseMass = 1.0f;
		restitution = 1.0f;

		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

		rotation = glm::mat3(1.0f);
		angularVelocity = glm::vec3(0.0f);
		angularAcceleration = glm::vec3(0.0f);

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);

		netTorque = netAngularImpulse = 0.0f;
	}

	///
	//Parameterized constructor, creates rigidbody at certain position with certain velocity and acceleration
	RigidBody::RigidBody(glm::vec3 pos, glm::vec3 vel, glm::vec3 acc, glm::mat3 rot, glm::vec3 aVel, glm::vec3 aAcc, float mass, float coeffOfRestitution)
	{
		inverseMass = mass == 0.0f ? 0.0f : 1.0f / mass;
		restitution = coeffOfRestitution;

		position = pos;
		velocity = vel;
		acceleration = acc;

		rotation = rot;
		angularVelocity = aVel;
		angularAcceleration = aAcc;

		netForce = glm::vec3(0.0f);
		netImpulse = glm::vec3(0.0f);

		netTorque = netAngularImpulse = 0.0f;
	}
};

struct Polygon polygon1;
struct Polygon polygon2;

struct ConvexHull convexHull1;
struct ConvexHull convexHull2;

struct RigidBody* rigidBody1;
struct RigidBody* rigidBody2;

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

	//EAsier to not use a shader for this simple 2D example.


	// Set options
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT, GL_FILL);
}

///
//Generates a convex hull to fit a given polygon
//
//Parameters:
//	destination: A pointer to a convex hull structure where the convex hull data generated from the polygon should be stored
//	poly: The polygon to generate the convex hull data from
void GenerateConvexHull(struct ConvexHull* destination, const struct Polygon &poly)
{
	//Copy the points from the polygon to the convex hull
	destination->points = poly.points;
	//Copy the current rotation of the polygon to the convex hull
	destination->rotation = poly.rotation;

	//Lastly we must generate the normals to the edges of the polygon we wish to generate a hull from.
	//In 2D, if we have an edge <x, y> we can find the normal with <-y, x>.
	//We can get each edge by subtracting each point from the previous.
	//We must also be sure to normalize these to make the collision algorithm easier later.
	int size = poly.points.size();
	glm::vec2 edge;
	for (int i = 0; i < size - 1; i++)
	{
		edge = poly.points[i + 1] - poly.points[i];
		destination->normals.push_back(glm::normalize(glm::vec2(-edge.y, edge.x)));
	}
	edge = poly.points[0] - poly.points[size - 1];
	destination->normals.push_back(glm::normalize(glm::vec2(-edge.y, edge.x)));

}

#pragma endregion Helper_functions

float CalculateMomentOfInertiaOfRectangle(float width, float height, float m)
{
	return  m * (powf(width, 2.0f)  + powf(height, 2.0f)) / 12.0f;
}

///
//Resolves a collision between two rigidbodies
//
//Overview:
//	Resolves a collision by calculating the collision impulse needed to apply to both objects in opposite directions (Newtons third law)
//	in order to simulate a collision between two rigidbodies. We calculate this impulse by using the definition of an impulse- a change in momentum.
//	In order to calculate the impulse we must first find a final velocity after the collision which we can do using 
//	Newton's law of Restitution. To make all of this easier, we first translate the system to a case where only one of the objects is moving
//	and relatively the other is still.
//
//Parameters:
//	body1: the rigidbody which the MTV points toward
//	body2: The rigidbody which the MTV points away
//	MTV: the minimum translation vector
//	pointOfCollision: The collision point between the two objects (Actually not needed in the strictly linear case).
void ResolveCollision(RigidBody &body1, RigidBody &body2, const glm::vec2 &MTV, const glm::vec2 &collisionPoint)
{
	//Step 1: Compute the relative velocity of object 1 from the point of collision on object 2
	//The point of collision on object 2 must be moving with object 2's velocity. Therefore if we subtract the velocity of object 2 from
	//the velocity of object 1 we can relatively determine how fast object 1 is travelling from the point on object 2.
	//This effectively reduces our scenario to object 2 being stationary, and object 1 moving with this relative speed!
	//
	//However, because the objects may also be rotating we must also factor in the linear velocity of the point of collision relative to the center of mass of each object
	//To do this, we will use the equation to calculate the linear velocity at a point due to angular motion:
	//	VPoint1 = body1.angularVelocity.z * Perp(radius1)
	//
	//Where radius 1 is the vector from the center of mass of object 1, to the point of collision.
	//And we can add VPoint1 to the linear velocity at the center of mass, to get the total velocity of the collision point on body1.
	//
	//However, because we are storing our angular acceleration as a vector, we can calculate VPoint1 in the correct direction by instead using the cross product!
	glm::vec3 radius1 = glm::vec3(collisionPoint, 0.0f) - body1.position;
	glm::vec3 radius2 = glm::vec3(collisionPoint, 0.0f) - body2.position;

	glm::vec3 velTotal1 = body1.velocity + glm::cross(body1.angularVelocity, radius1);
	glm::vec3 velTotal2 = body2.velocity + glm::cross(body2.angularVelocity, radius2);

	glm::vec3 relativeVelocity = velTotal1 - velTotal2;

	//Step 2: Determine the magnitude of the relative velocity in the direction of the collision.
	//Say you are riding a bike about 20 MPH, and you are riding past a long building. You take one hand off of the handle bars and slide it along
	//the wall as you ride by. Does it feel like you hit the wall going 20 MPH? No! Of Course not.
	//This is because, even though you are going 20 MPH relative to the point of collision on the wall, you are going ~0 MPH in the direction
	//of the collision normal-- or in the direction of the wall!
	//
	//We can determine this value by using the geometric properties of the dot product, because we know the MTV is already normalized.
	float relativeVelocityPerp = glm::dot(relativeVelocity, glm::vec3(MTV, 0.0f));	//Keep in mind, because the MTV by our convention always points toward object 1, and
	//We found the relative velocity of object 1, this should be negative if object 1 is moving
	//towards object 2 relatively (The objects are converging)

	//Step 3: Calculate the relative velocity after the collision
	//Newtons law of restitution states that e = |V2perp| / |V1perp|
	//Or more simply, the coefficient of restitution (e) of an object hitting a stationary object is the ratio of the 
	//final speed in the direction of the collision (|V2perp|) to the initial speed in the direction of the collision (|V1|).
	//This holds
	//We know our coefficient of restitution by the design of the game, so we can use this to compute the final relative velocity!
	//It should be noted that the final coefficient of restitution is the product of each objects created by design.
	float e = body1.restitution * body2.restitution;
	float finalRelativeVelocityPerp = -e * relativeVelocityPerp;	//Remember, the final velocity will have it's direction switched (So make e negative!) 

	//Step 4: Calculate the collision impulse
	//Assume there is some collision impulse, j, which is in the direction of the MTV that is going to alter the velocity of our rigidbodies.
	//And impulse is a change in momentum:
	//	j = m * dV 
	//	j = m * (VAfter - VBefore)
	//And we simply solved for VAfter.
	//
	//We can calculate the velocity after the collision for body1, V1after, as:
	//	V1After = body1.velocity + body1.inverseMass * j
	//
	//And if we take angular kinematics into account:
	//	VAngular1After = body1.angularVelocity + (Perp(radius1) . j * MTV) / body1.momentOfInertia
	//
	//It is also worth noting, V2After, the velocity of body2 after the collision, will be nearly the same-- except in the opposite direction so we would have:
	//	V2After = body2.velocity - body2.inverseMass * j
	//	VAngular2After = body2.angularVelocity - (Perp(Radius1) . j * MTV) / body2.momentOfInertia
	//
	//Furthermore, these relationships hold true for the relative velocity & the sum of the masses and moments of inertia!
	//	j = (VRelAfter - VRelBefore)/(m1 + m2 + (Perp(Radius1) . MTV)^2 / body1.momentOfInertia + (Perp(Radius2) . MTV)^2/body2.momentOfInertia)
	//
	//And because our impulse is only going to be in the direction of the collision, like we discussed earlier:
	//	j = (finalRelativeVelocityPerp - relativeVelocityPerp)/(1.0f/m1 + 1.0f/m2 + (Perp(Radius1) . MTV)^2 / body1.momentOfInertia + (Perp(Radius2) . MTV)^2/body2.momentOfInertia)
	//
	glm::vec3 perpRadius1 = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), radius1);
	glm::vec3 perpRadius2 = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), radius2);
	float j = (finalRelativeVelocityPerp - relativeVelocityPerp) / (body1.inverseMass + body2.inverseMass + (powf(glm::dot(perpRadius1, glm::vec3(MTV, 0.0f)), 2.0f) / body1.momentOfInertia + ( powf(glm::dot(perpRadius2, glm::vec3(MTV, 0.0f)), 2.0f)/body2.momentOfInertia)));

	//Newtons third law says for every action there is an equal and opposite reaction. This means that the impulse, j, we calculated is
	//the correct impulse for both objects-- simply the direction we apply it in changes!

	//Step 5: Apply the impulse
	//Remember, the MTV always points away from object 2, toward object 1 by our convention.
	//We can apply the impulse to body1 along the MTV, and the impulse to body2 away from the MTV.
	//(The MTV is our collision normal)
	glm::vec3 impulse = glm::vec3(j * MTV, 0.0f);
	body1.netImpulse += impulse;

	//And determine the net angular impulse from this
	//Torque = radius x force
	body1.netAngularImpulse += glm::cross(radius1, impulse).z;

	impulse *= -1.0f;
	body2.netImpulse += impulse;

	//Determine angular impulse for body 2
	body2.netAngularImpulse += glm::cross(radius2, impulse).z;

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

	//Zero the net impulse and net force!
	body.netForce = body.netImpulse = glm::vec3(0.0f);
}

void IntegrateAngular(float dt, RigidBody &body)
{
	//Calculate new angular acceleration
	body.angularAcceleration = glm::vec3(0.0f, 0.0f, body.netTorque / body.momentOfInertia);

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
	body.angularVelocity += dt * body.angularAcceleration + glm::vec3(0.0f, 0.0f, body.netAngularImpulse / body.momentOfInertia);
	body.netTorque = body.netAngularImpulse = 0.0f;
}


///
//Determines the point of intersection
//
//Overview:
//	This algorithm determines the point of intersection between two objects by determining the set of points
//	most in the direction of the other object along the axis made by the Minimum Translation Vector.
//	This is done using the dot product.
//
//Parameters:
//	hull1: The convex hull which the minimumTranslationVector points toward
//	position1: The position of hull1 in worldspace
//	hull2: The convex hull which the minimumTranslation vector points away from
//	position2: The position of hull2 in worldspace
//	MTV: The minimum translation vector
//
//Returns:
//	A vec2 containing the point of collision in worldspace
glm::vec2 DeterminePointOfCollision(const struct ConvexHull &hull1, const glm::vec2 &position1, 
									const struct ConvexHull &hull2, const glm::vec2 &position2, 
									const glm::vec2 &MTV)
{
	//Sometimes edges may be so close to being flush that it appears they lie upon each other
	//But the angle between them is so small we cannot see it. Therefore we must accept a certain tolerance of error
	//to be considered as edge - edge collision
	float tolerance = 0.01f;

	//Find the point least in the direction of the MTV on hull1
	std::vector<glm::vec2> closestPoints1;
	float currentMin;
	float dotProd;
	int numPoints = hull1.points.size();
	glm::vec2 currentPoint;

	//Begin by setting the current minimum to the first point of hull1.
	currentPoint = hull1.rotation * hull1.points[0] + position1;
	currentMin = glm::dot(currentPoint, MTV);
	closestPoints1.push_back(currentPoint);

	//Loop through remaining points

	for(int i = 1; i < numPoints; i++)
	{
		//Translate to world space
		currentPoint = hull1.rotation * hull1.points[i] + position1;
		//Calculate distance towards object 2
		dotProd = glm::dot(currentPoint, MTV);
		//If the current point is the same distance in the direction of obj2 along the MTV (within tolerance)
		if(fabs(dotProd - currentMin) < FLT_EPSILON + tolerance)
		{
			//Add the new point
			closestPoints1.push_back(currentPoint);
		}
		//else If it is less than the current distance (more in the direction of obj2-- remember MTV Points toward object 1!)
		else if(dotProd < currentMin - FLT_EPSILON)
		{
			//Clear the list of current points and set the new one
			currentMin = dotProd;
			closestPoints1.clear();
			closestPoints1.push_back(currentPoint);
		} 
	}

	//If hull1 has only one closest point, we have found the point of collision!
	if(closestPoints1.size() == 1)
	{
		return closestPoints1[0];
	}
	//If we have not yet found the point of collision we must check hull2
	//Find the point most in the direction of the MTV on hull2
	std::vector<glm::vec2> closestPoints2;
	float currentMax;
	numPoints = hull2.points.size();

	//Begin by setting the current maximum to the first point of hull2.
	currentPoint = hull2.rotation * hull2.points[0] + position2;
	currentMax = glm::dot(currentPoint, MTV);
	closestPoints2.push_back(currentPoint);

	//Loop through remaining points
	for(int i = 1; i < numPoints; i++)
	{
		//Translate to world space
		currentPoint = hull2.rotation * hull2.points[i] + position2;
		//Calculate distance towards object 1
		dotProd = glm::dot(currentPoint, MTV);

		//If the current point is the same distance in the direction of obj1 along the MTV (within tolerance)
		if(fabs(dotProd - currentMax) < FLT_EPSILON + tolerance)
		{
			//Add the new point
			closestPoints2.push_back(currentPoint);
		}
		//If it is more than the current distance
		else if(dotProd > currentMax + FLT_EPSILON)
		{
			//Clear the list of current points and set the new one
			currentMax = dotProd;
			closestPoints2.clear();
			closestPoints2.push_back(currentPoint);
		} 
	}

	//If hull2 has only one closest point, we have found the point of collision!
	if(closestPoints2.size() == 1) 
	{
		return closestPoints2[0];
	}
	//If this is not the case, we must find the two "inner points"
	//We can do this by disarding the minimum and maximum points along an axis.
	//The axis we can check should be the edge axis, or the axis perpendicular to the MTV
	glm::vec2 edge(-MTV.y, MTV.x);

	//We can assume that the edges are not degenerate.. Or something else is wrong.
	//Concatenate our two sets of closest points into one list
	closestPoints1.insert(closestPoints1.end(), closestPoints2.begin(), closestPoints2.end());	//Appends set 2 to set 1


	//Determine the minimum and maximum
	currentMin = currentMax = glm::dot(closestPoints1[0], edge);
	int minIndex, maxIndex;
	minIndex = maxIndex = 0;

	numPoints = closestPoints1.size();
	for(int i = 0; i < numPoints; i++)
	{
		dotProd = glm::dot(closestPoints1[i], edge);
		if(dotProd < currentMin)
		{
			currentMin = dotProd;
			minIndex = i;
		}
		if(dotProd > currentMax)
		{
			currentMax = dotProd;
			maxIndex = i;
		}
	}

	//Remove the min and max indices
	closestPoints1.erase(closestPoints1.begin() + minIndex);
	if(minIndex < maxIndex) --maxIndex;
	closestPoints1.erase(closestPoints1.begin() + maxIndex);

	//Take the average of the two remaining indices!
	glm::vec2 closestPoint = (closestPoints1[0] + closestPoints1[1]) * 0.5f;
	return closestPoint;
}

///
//Separates two intersecting objects back to a state of contact
//
//Parameters:
//	c1: The first of the intersecting hulls (Which the MTV points toward by our convention)
//	c2: The second of the intersecting circles (Which the MTV points away by our convention)
//	MTV: The axis of minimal translation needed to separate the circles
//	mag: The magnitude of overlap along the minimum translation vector
void DecoupleObjects(struct RigidBody &body1, struct RigidBody &body2, const glm::vec2 &MTV, const float &mag)
{
	//The first step in decoupling a pair of objects is to figure how much you must move each one.
	//you can do this by taking the sum of the magnitudes of their velocities along the MTV
	//And performing a ratio of each individual velocities magnitude along the MTV to that sum.
	//
	//For example, if we wanted to figure out how much to move circle c1:
	float individual1 = fabs(glm::dot(glm::vec2(body1.velocity), MTV));
	float individual2 = fabs(glm::dot(glm::vec2(body2.velocity), MTV));

	float sum = individual1 + individual2;
	float ratio1 = individual1/sum;
	float ratio2 = individual2/sum;

	//From here, we can figure out the magnitude of of how much to move poly 1 along the MTV by taking the product of the ratio with the magnitude of the overlap
	//	mag1 = ratio * mag
	float mag1, mag2;
	mag1 = ratio1 * mag;
	mag2 = ratio2 * mag;

	//Now, remember, the MTV always points toward object1, by the convention we established. So we want to move circle1 along the MTV, and circle2 opposite the MTV!
	body1.position += glm::vec3(mag1 * MTV, 0.0f);
	body2.position -= glm::vec3(mag2 * MTV, 0.0f);
}

///
//Determines if a collision needs to be resolved
//
//Parameters:
//	body1: The rigidbody of the object that the minimum translation vector points toward
//	body2: The rigidbody of the object that the minimum translation vector points away from
//	MTV: the minimum translation vector
bool IsResolutionNeeded(struct RigidBody &body1, struct RigidBody &body2, glm::vec2 &MTV, glm::vec2 collisionPoint)
{
	//Find the relative velocity of the point of collision on object 2 from the point of collision on object 1
	glm::vec3 radius1 = glm::vec3(collisionPoint, 0.0f) - body1.position;
	glm::vec3 radius2 = glm::vec3(collisionPoint, 0.0f) - body2.position;

	glm::vec3 velTotal1 = body1.velocity + glm::cross(body1.angularVelocity, radius1);
	glm::vec3 velTotal2 = body2.velocity + glm::cross(body2.angularVelocity, radius2);

	glm::vec2 relativeVelocity = glm::vec2(velTotal2 - velTotal1);
	//Check it's dot product with the MTV to make sure it is in the direction of the MTV
	if(glm::dot(MTV, relativeVelocity) > 0.0f)
	{
		return true;
	}
	//Else, the objects will resolve their own collision (They are moving away from each other)
	else
	{
		return false;
	}
}

///
//Performs the separating axis test in 2D to see if two convex hulls are colliding,
//and tracks the direction of least separation.
//
//Overview:
//	This algorithm works by creating a single dimensional axis from each edge normal belonging to both polygons.
//	Each axis represents a direction away from the edge it belongs to.
//	By taking the scalar projection of a point onto an axis we can see how close that point lies away from the edge if it were to extend outward in every direction.
//	If we keep track of the maximum and minimum measurements of these scalar projections, we can see the boundary of a polygon on these axes.
//	By comparing the boundaries of the polygons on each axis we can determine which axes the polygons overlap on,
//	and if they overlap on every axis, we must have a collision.
//
//PArameters:
//	hull1: The first convex hull being tested for a collision
//	position1: The position of the first convex hull
//	hull2: The second convex hull being tested for a collision
//	position2: The position of the second convex hull
//	MTV: the minimumTranslationVector
//	mag: The magnitude of overlap in the direction of the MTV
bool TestIntersection(const struct ConvexHull &hull1, const glm::vec2 &position1, const struct ConvexHull &hull2, const glm::vec2 &position2, glm::vec2 &MTV, float &mag)
{
	//First we must get the points of both convex hulls in world space.
	std::vector<glm::vec2> worldPoints1;
	int numPoints1 = hull1.points.size();
	for (int i = 0; i < numPoints1; i++)
	{
		worldPoints1.push_back(position1 + hull1.rotation * hull1.points[i]);
	}

	std::vector<glm::vec2> worldPoints2;
	int numPoints2 = hull2.points.size();
	for (int i = 0; i < numPoints2; i++)
	{
		worldPoints2.push_back(position2 + hull2.rotation * hull2.points[i]);
	}

	//Next we must get the rotated normals of each hull
	std::vector<glm::vec2> rNormals1;
	int numNormals1 = hull1.normals.size();
	for (int i = 0; i < numNormals1; i++)
	{
		rNormals1.push_back(hull1.rotation * hull1.normals[i]);
	}

	std::vector<glm::vec2> rNormals2;
	int numNormals2 = hull2.normals.size();
	for (int i = 0; i < numNormals2; i++)
	{
		rNormals2.push_back(hull2.rotation * hull2.normals[i]);
	}

	//After we have the needed information we must begin to check all of our possible axes for collision.
	//First we will check hull1's normals
	for (int i = 0; i < numNormals1; i++)
	{
		//For each normal, we must determine the scalar projection of all points from both hulls onto the current normal.
		//The projection formula can be given as follows:
		//	Proj(x, y) = ((x . y) / (y . y)) * y
		//where "Proj(x, y)" denotes the projection of the vector x onto the vector y, and " . " denotes the dot product.
		//We can simplify this because our normal (y in the above example) is normalized. A vector dotted with itself is equal to the magnitude squared:
		//	Proj(x, y) = ((x . y)/1) * y
		//	Proj(x, y) = (x . y) * y

		//Finally, we can simplify this one step further. Because we do not care for a vector representing the projection, but rather a means of
		//comparing (not correctly quantifying) distances in a direction, we can assume that if (x . y) is a larger value than (q . y), (x . y) * y must also be larger
		//than (q . y) * y. So by using the dot product of each point, x, with each normal, y, we can effectively get a scalar representation of how far x is in the direction
		//Of y, and therefore how far x is away from the edge.

		//While this value is not the real distance, we can compare the maximum and minimum values found from each point set and check if they overlap
		//Which will imply a collision on that axis.

		//First we will test hull1's points keeping track of the minimum and maximum values.
		float min1, max1;
		//Start by setting both min and max to the scalar projection of the first point
		min1 = max1 = glm::dot(rNormals1[i], worldPoints1[0]);
		//Begin checking all other points
		for (int j = 1; j < numPoints1; j++)
		{
			//Get the current scalar projection of point j
			float current = glm::dot(rNormals1[i], worldPoints1[j]);
			if (current < min1) min1 = current;			//Check if it is smaller than the minimum
			else if (current > max1) max1 = current;	//Check if it is larger than the maximum
		}

		//Perform the same algorithm with hull 2's points
		float min2, max2;
		//Start by setting both min and max to the scalar projection of the first point
		min2 = max2 = glm::dot(rNormals1[i], worldPoints2[0]);
		//Begin checking all other points
		for (int j = 1; j < numPoints2; j++)
		{
			//Get the current scalar projection of point j
			float current = glm::dot(rNormals1[i], worldPoints2[j]);
			if (current < min2) min2 = current;			//Check if it is smaller than the minimum
			else if (current > max2) max2 = current;	//Check if it is larger than the maximum
		}

		//If the mins and maxes from both hulls do not overlap, there cannot be a collision and we can stop testing.
		if (min1 < max2 && max1 > min2)
		{
			//If they do overlap, we must see if this is the smallest overlap we have found so far
			float overlap1 = max2 - min1;
			float overlap2 = max1 - min2;
			//Let overlap1 be the smaller overlap
			overlap1 = overlap2 < overlap1 ? overlap2 : overlap1;
			//If i == 0 or, this is smaller than the current overlap
			if(i == 0 || overlap1 < mag)
			{
				mag = overlap1;
				MTV = rNormals1[i];
			}
		}
		else return false;
	}

	//Now we must check all axes from hull2
	for (int i = 0; i < numNormals2; i++)
	{
		//First we will test hull1's points keeping track of the minimum and maximum values.
		float min1, max1;
		//Start by setting both min and max to the scalar projection of the first point
		min1 = max1 = glm::dot(rNormals2[i], worldPoints1[0]);
		//Begin checking all other points
		for (int j = 1; j < numPoints1; j++)
		{
			//Get the current scalar projection of point j
			float current = glm::dot(rNormals2[i], worldPoints1[j]);
			if (current < min1) min1 = current;			//Check if it is smaller than the minimum
			else if (current > max1) max1 = current;	//Check if it is larger than the maximum
		}

		//Perform the same algorithm with hull 2's points
		float min2, max2;
		//Start by setting both min and max to the scalar projection of the first point
		min2 = max2 = glm::dot(rNormals2[i], worldPoints2[0]);
		//Begin checking all other points
		for (int j = 1; j < numPoints2; j++)
		{
			//Get the current scalar projection of point j
			float current = glm::dot(rNormals2[i], worldPoints2[j]);
			if (current < min2) min2 = current;			//Check if it is smaller than the minimum
			else if (current > max2) max2 = current;	//Check if it is larger than the maximum
		}

		//If the mins and maxes from both hulls do not overlap, there cannot be a collision and we can stop testing.
		if (min1 < max2 && max1 > min2)
		{
			//If they do overlap, we must see if this is the smallest overlap we have found so far
			float overlap1 = max2 - min1;
			float overlap2 = max1 - min2;
			//Let overlap1 be the smaller overlap
			overlap1 = overlap2 < overlap1 ? overlap2 : overlap1;
			//If i == 0 or, this is smaller than the current overlap
			if(overlap1 < mag)
			{
				mag = overlap1;
				MTV = rNormals2[i];
			}
		}
		else return false;
	}

	//If there is a collision the code will reach here.
	//We always want to know if the MTV is pointing towards object 1 or object 2
	//By convention, I always make the MTV point towards object 1.
	//Although I've seen others use this before aswell, I'm not sure if it is considered standard.
	////
	//*Pick one, stick with it, and document your decision in code!*
	////

	glm::vec2 bToa;

	bToa = position1 - position2;
	//Check if MTV must be reversed to face Obj 1
	if(glm::dot(bToa, minimumTranslationVector) < 0.0f)
	{
		MTV *= -1.0f;
	}

	return true;
}

///
//Wraps a moving circle around the edges of the screen
//
//Parameters:
//	body: The rigidbody of the circle
//	circle: The collider of the circle
void Wrap(struct RigidBody &body)
{
	if(body.position.x < -1.0f)
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
	//Update the rigidbodies
	IntegrateLinear(dt, *rigidBody1);
	IntegrateLinear(dt, *rigidBody2);

	IntegrateAngular(dt, *rigidBody1);
	IntegrateAngular(dt, *rigidBody2);

	//Check for collision
	if(TestIntersection(convexHull1, glm::vec2(rigidBody1->position), convexHull2, glm::vec2(rigidBody2->position), minimumTranslationVector, overlap))
	{
		//Decouple and find point of collision
		DecoupleObjects(*rigidBody1, *rigidBody2, minimumTranslationVector, overlap);
		pointOfCollision = DeterminePointOfCollision(convexHull1, glm::vec2(rigidBody1->position), convexHull2, glm::vec2(rigidBody2->position), minimumTranslationVector);

		//Check if collision resolution is necessary
		//If two objects are already moving away from each other, but in contact, we let them resolves themselves.
		if(IsResolutionNeeded(*rigidBody1, *rigidBody2, minimumTranslationVector, pointOfCollision))
		{
			//Resolve collision
			ResolveCollision(*rigidBody1, *rigidBody2, minimumTranslationVector, pointOfCollision);
		}
	}

	Wrap(*rigidBody1);
	Wrap(*rigidBody2);

	//Move appearance to new position
	polygon1.center = glm::vec2(rigidBody1->position);
	polygon2.center = glm::vec2(rigidBody2->position);
	polygon1.rotation = convexHull1.rotation = glm::mat2(rigidBody1->rotation);
	polygon2.rotation = convexHull2.rotation = glm::mat2(rigidBody2->rotation);
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

	// Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 0.0);



	// code to draw the line between two points.This is using a deprecated method. This should be done in the shaders. But for simple physics implementations and debugging,
	// this is enough. 
	glUseProgram(0);
	glLineWidth(2.5f);
	glPointSize(8.0f);
	glColor3f(1.0f , 1.0f, 1.0f);

	//We must get the points of polygon 1 oriented in space
	std::vector<glm::vec2> poly1Temp;
	int size1 = polygon1.points.size();
	for (int i = 0; i < size1; i++)
	{
		poly1Temp.push_back(polygon1.center + polygon1.rotation * polygon1.points[i]);
	}
	//Same for polygon 2
	std::vector<glm::vec2> poly2Temp;
	int size2 = polygon2.points.size();
	for (int i = 0; i < size2; i++)
	{
		poly2Temp.push_back(polygon2.center + polygon2.rotation * polygon2.points[i]);
	}

	//Draw polygon 1
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < size1; i++)
	{
		glVertex3f(poly1Temp[i].x, poly1Temp[i].y, 0.0f);
	}
	glEnd();

	//Draw polygon 2
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < size2; i++)
	{
		glVertex3f(poly2Temp[i].x, poly2Temp[i].y, 0.0f);
	}
	glEnd();


}

#pragma endregion util_Functions



void main()
{
	glfwInit();

	// Create a window
	window = glfwCreateWindow(800, 800, "Resolving Collisions (Linear - 2D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();


	//Initialize first polygon
	polygon1.center.x = 0.5f;
	polygon1.points.push_back(glm::vec2(0.1f, 0.0f));
	polygon1.points.push_back(glm::vec2(0.1f, -0.1f));
	polygon1.points.push_back(glm::vec2(-0.2f, -0.1f));
	polygon1.points.push_back(glm::vec2(-0.1f, 0.1f));

	//Initialize first convex hull
	GenerateConvexHull(&convexHull1, polygon1);

	//Initialize second polygon
	polygon2.center.x = -0.5f;
	polygon2.points.push_back(glm::vec2(0.0f, 0.3f));
	polygon2.points.push_back(glm::vec2(-0.1f, 0.0f));
	polygon2.points.push_back(glm::vec2(0.0f, -0.1f));
	polygon2.points.push_back(glm::vec2(0.1f, 0.0f));

	//Initialize second convex hull
	GenerateConvexHull(&convexHull2, polygon2);


	//Generate the circle rigidbodies
	rigidBody1 = new RigidBody(
		glm::vec3(-0.75f, 0.00f, 0.0f),		//Initial position
		glm::vec3(0.2f, 0.0f, 0.0f),		//Initial velocity
		glm::vec3(0.0f),					//Zero acceleration
		glm::mat3(1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		1.0f,								//Mass
		1.0f								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		);
	rigidBody2 = new RigidBody(
		glm::vec3(0.75f, -0.2f, 0.0f), 		//Initial position
		glm::vec3(-0.2f, 0.0f, 0.0f), 		//Initial velocity
		glm::vec3(0.0f), 					//Zero acceleration
		glm::mat3(1.0f),
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		0.5f, 								//Mass
		1.0f								//Elasticity of object (1.0 is perfectly elastic-- no energy lost in collisions)
		);

	//Calculate approximate moments of inertia using an axis aligned rectangle that fits around the polygons
	rigidBody1->momentOfInertia = CalculateMomentOfInertiaOfRectangle(0.3f, 0.2f, 1.0f / rigidBody1->inverseMass);
	rigidBody2->momentOfInertia = CalculateMomentOfInertiaOfRectangle(0.2f, 0.4f, 1.0f / rigidBody2->inverseMass);

	//orient polygons
	polygon1.center = glm::vec2(rigidBody1->position);
	polygon2.center = glm::vec2(rigidBody2->position);
	polygon1.rotation = glm::mat2(rigidBody1->rotation);
	polygon2.rotation = glm::mat2(rigidBody2->rotation);

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

	delete rigidBody1;
	delete rigidBody2;

	// Frees up GLFW memory
	glfwTerminate();
}