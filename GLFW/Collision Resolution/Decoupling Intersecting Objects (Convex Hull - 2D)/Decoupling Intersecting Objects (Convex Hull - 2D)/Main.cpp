/*
Title: Minimum Translation Vector (Convex Hull 2D)
File Name: Main.cpp
Copyright © 2015
Revision Authors: Nicholas Gallagher
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
This is a demonstration of decoupling Two intersecting convex polygons in 2D.
Decoupling two shapes occurs when during a timestep two objects have intersected, and before computing a
physical reaction you will (most likely) want to pull the objects apart.

After determining the minimum translation vector we can translate the objects the correct amount
to have a contact case rather than intersection.

You can move the polygons using WASD, and rotate them using Q and E.
You can switch which polygon you are controlling using Spacebar.
The polygons will appear green when not intersecting, and red when a collision is detected. 

References:
AABB2D - Brockton Roth
*/

#include "GLIncludes.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

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


glm::vec3 lineColor(1.0f, 1.0f, 1.0f);

glm::vec2 minimumTranslationVector;		//The vector which signifies the smallest direction of translation needed to separate two colliding objects.
float overlap;							//The magnitude of overlap in the minimumTranslationVector
bool collision = false;

// Variable to set the sensitivity of the input controls.
float movrate = 0.05f;
float rotrate = 0.1f;

// Reference to the window object being created by GLFW.
GLFWwindow* window;

struct Polygon poly1, poly2;
struct Polygon* selectedPoly = &poly1;
struct ConvexHull convexHull1, convexHull2;

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
bool TestIntersection(const struct ConvexHull &hull1, const glm::vec2 &position1, const struct ConvexHull &hull2, const glm::vec2 &position2)
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
			if(i == 0 || overlap1 < overlap)
			{
				overlap = overlap1;
				minimumTranslationVector = rNormals1[i];
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
			if(overlap1 < overlap)
			{
				overlap = overlap1;
				minimumTranslationVector = rNormals2[i];
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

	bToa = poly1.center - poly2.center;
	//Check if MTV must be reversed to face Obj 1
	if(glm::dot(bToa, minimumTranslationVector) < 0.0f)
	{
		minimumTranslationVector *= -1.0f;
	}

	return true;
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

///
//Separates two intersecting polygons
//
//Parameters:
//	poly1: The first of the intersecting polygons
//	poly2: The second of the intersecting polygons
//	MTV: The minimum translation vector
//	mag: The magnitude of the overlap along the minimum translation vector
void DecoupleObjects(Polygon &poly1, Polygon &poly2, const glm::vec2 &MTV, const float mag)
{
	//The first step in decoupling a pair of objects is to figure how much you must move each one.
	//Normally, you can do this by taking the sum of the magnitudes of their velocities along the MTV
	//And performing a ratio of each individual velocities magnitude along the MTV to that sum.
	//
	//For example, if we wanted to figure out how much to move poly1:
	//	individual = fabs(glm::Dot(poly1.velocity, MTV))
	//	sum = individual + fabs(glm::Dot(Poly2.velocity, MTV));
	//	ratio = individual / sum
	//
	//From here, we can figure out the magnitude of of how much to move poly 1 along the MTV by taking the product of the ratio with the magnitude of the overlap
	//	mag1 = ratio * mag
	//
	//However, in this simulation we do not have velocities-- and it is clear which object is doing all of the movement. However, Instead of translating the
	//moving polygon, I will translate the non-moving polygon to allow for a "Pushing" effect that might be sought after in simpler games!
	float mag1, mag2;
	//The next line of code is just setting the mag corresponding to polygon that is NOT the selected polygon to 1. The other polygon is set to 0.
	//So for example, if polygon 1 is the selected polygon, mag1 = 0, and mag2 = 1, and visa versa
	mag1 = selectedPoly == &poly1 ? ((mag2 = 1.0f), 0.0f) : ((mag2 = 0.0f), 1.0f);	//Don't code like this if you aren't good at reading this kind of thing. Readability is important.
																					//Also, don't code like this if anybody else will need to maintain your code. They will hate you.
																					//I just broke that rule^^.. Sorry whoever needs to fix this one day...

	//Now, remember, the MTV always points toward object1, by the convention we established. So we want to move poly1 along the MTV, and poly2 opposite the MTV!
	poly1.center += mag1 * mag * MTV;
	poly2.center -= mag2 * mag * MTV;
}

//Checks for line intersections and adjusts color accordingly
void update()
{
	//Each update we must be sure to update the convex hulls with the current orientation of the polygons
	convexHull1.rotation = poly1.rotation;
	convexHull2.rotation = poly2.rotation;

	//test if they are intersection. If so, change the color of the line segment.
	if (TestIntersection(convexHull1, poly1.center, convexHull2, poly2.center))
	{
		//Decouple
		DecoupleObjects(poly1, poly2, minimumTranslationVector, overlap);
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
	glColor3f(lineColor.x, lineColor.y, lineColor.z);

	//We must get the points of polygon 1 oriented in space
	std::vector<glm::vec2> poly1Temp;
	int size1 = poly1.points.size();
	for (int i = 0; i < size1; i++)
	{
		poly1Temp.push_back(poly1.center + poly1.rotation * poly1.points[i]);
	}
	//Same for polygon 2
	std::vector<glm::vec2> poly2Temp;
	int size2 = poly2.points.size();
	for (int i = 0; i < size2; i++)
	{
		poly2Temp.push_back(poly2.center + poly2.rotation * poly2.points[i]);
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

// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//This control will alter the line selected
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		selectedPoly = selectedPoly == &poly1 ? &poly2 : &poly1;

	//This set of controls are used to move one point (point1) of the line (line1).
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		selectedPoly->center.y += movrate;
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		selectedPoly->center.x -= movrate;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		selectedPoly->center.y -= movrate;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		selectedPoly->center.x += movrate;

	//Set of controls to rotate the selected polygon
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		selectedPoly->rotation = glm::mat2(glm::rotate(glm::mat4(selectedPoly->rotation), rotrate, glm::vec3(0.0f, 0.0f, 1.0f)));
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		selectedPoly->rotation = glm::mat2(glm::rotate(glm::mat4(selectedPoly->rotation), -rotrate, glm::vec3(0.0f, 0.0f, 1.0f)));
}

// Initialization code
void init()
{	
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	//Initialize first polygon
	poly1.center.x = 0.5f;
	poly1.points.push_back(glm::vec2(0.1f, 0.0f));
	poly1.points.push_back(glm::vec2(0.1f, -0.1f));
	poly1.points.push_back(glm::vec2(-0.2f, -0.1f));
	poly1.points.push_back(glm::vec2(-0.1f, 0.1f));

	//Initialize first convex hull
	GenerateConvexHull(&convexHull1, poly1);

	//Initialize second polygon
	poly2.center.x = -0.5f;
	poly2.points.push_back(glm::vec2(0.0f, 0.3f));
	poly2.points.push_back(glm::vec2(-0.1f, 0.0f));
	poly2.points.push_back(glm::vec2(0.0f, -0.1f));
	poly2.points.push_back(glm::vec2(0.1f, 0.0f));

	//Initialize second convex hull
	GenerateConvexHull(&convexHull2, poly2);

	//Set options
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();
	//Create window
	window = glfwCreateWindow(800, 800, "Decoupling Intersecting Objects (Convex Hull - 2D)", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	//Print controls
	std::cout << "Controls:\nUse WASD to move the selected polygon.\nUse Q and E to rotate the selected polygon.\nPress spacebar to swap the selected polygon.\n";

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		glfwSwapBuffers(window);

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}