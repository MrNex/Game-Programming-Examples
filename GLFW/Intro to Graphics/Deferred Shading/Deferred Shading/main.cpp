/*
Title: Deferred Shading
File Name: main.cpp
Copyright © 2015
Original authors: Brockton Roth
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
This example demonstrates the implementation of deferred shading 
rendering technique. In this example we render two spheres.

In the first pass, we store the position, normal and diffuse color 
on separate textures. This is done by attaching three separate textures
to a frame buffer object. and then rendering to the frame buffer.

In the second pass, we get the position, normal and diffuse color by 
sampling the three textures, and use those to compute the final color 
of that particular pixel based on lighting calculations. 

References:
OpenGL 4 Shading language Cookbook
*/


#pragma once
#include "GLIncludes.h"
#include "BasicFunctions.h"

//Define the constants
#define PI 3.14159265
#define WindowSize 800
#define DIVISIONS 40

// Creating handles for buffers on the GPU
GLuint depthBuf;					// Depth buffer
GLuint posTex, normTex, colorTex;	// Position-texture buffer, Normal-texture buffer, DiffuseColor-texture buffer 
GLuint fboHandle;					// Framebuffer Handle. 

//Projection matrix multiplied by view matrix (its global because we have only 1 camera in the scene.
glm::mat4 PV;

// handles to uniforms in the shaders
GLuint LightPos;
GLuint LightIntensity;
GLuint LightAmbient;

// This funciton creates a buffer to store the texture in the GPU.
// We also have to specify the size of the texture along with the type of data it will store.
// This is done so that enough memory is available to store the data for every pixel in the texture.
void createBufTex(GLenum texUnit, GLenum format, GLuint &texId)
{
	glActiveTexture(texUnit);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, format, WindowSize, WindowSize, 0, GL_RGBA, GL_FLOAT, 0);
	
	//Also set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

void createGeometry()
{
	//Create the vertices for the spheres to be drawn
	std::vector<VertexFormat> vertices;

	float radius = 0.5f;
	float pitch, yaw;
	yaw = 0.0f;
	pitch = 0.0f;
	int i, j;
	float pitchDelta = 360 / DIVISIONS;
	float yawDelta = 360 / DIVISIONS;

	VertexFormat p1, p2, p3, p4;

	for (i = 0; i < DIVISIONS; i++)
	{
		for (j = 0; j < DIVISIONS; j++)
		{
			p1.position.x = radius * sin((pitch)* PI / 180.0) * cos((yaw)* PI / 180.0);
			p1.position.y = radius * sin((pitch)* PI / 180.0) * sin((yaw)* PI / 180.0);;
			p1.position.z = radius * cos((pitch)* PI / 180.0);
			p1.normal = p1.position;
			p1.color.r = 0.7f;
			p1.color.g = 0.2f;
			p1.color.b = 0;
			p1.color.a = 1;

			p2.position.x = radius * sin((pitch)* PI / 180.0) * cos((yaw + yawDelta)* PI / 180.0);
			p2.position.y = radius * sin((pitch)* PI / 180.0) * sin((yaw + yawDelta)* PI / 180.0);;
			p2.position.z = radius * cos((pitch)* PI / 180.0);
			p2.normal = p2.position;
			p2.color.r = 0.7f;
			p2.color.g = 0.2f;
			p2.color.b = 0;
			p2.color.a = 1;

			p3.position.x = radius * sin((pitch + pitchDelta)* PI / 180.0) * cos((yaw + yawDelta)* PI / 180.0);
			p3.position.y = radius * sin((pitch + pitchDelta)* PI / 180.0) * sin((yaw + yawDelta)* PI / 180.0);;
			p3.position.z = radius * cos((pitch + pitchDelta)* PI / 180.0);
			p3.normal = p3.position;
			p3.color.r = 0.7f;
			p3.color.g = 0.2f;
			p3.color.b = 0;
			p3.color.a = 1;

			p4.position.x = radius * sin((pitch + pitchDelta)* PI / 180.0) * cos((yaw)* PI / 180.0);
			p4.position.y = radius * sin((pitch + pitchDelta)* PI / 180.0) * sin((yaw)* PI / 180.0);;
			p4.position.z = radius * cos((pitch + pitchDelta)* PI / 180.0);
			p4.normal = p4.position;
			p4.color.r = 0.7f;
			p4.color.g = 0.2f;
			p4.color.b = 0;
			p4.color.a = 1;

			vertices.push_back(p1);
			vertices.push_back(p2);
			vertices.push_back(p3);
			vertices.push_back(p1);
			vertices.push_back(p3);
			vertices.push_back(p4);

			yaw = yaw + yawDelta;
		}

		pitch += pitchDelta;
	}

	sphere1.base.initBuffer(vertices.size(), &vertices[0]);
	sphere2.base.initBuffer(vertices.size(), &vertices[0]);

	sphere1.origin = glm::vec3(0.0f);
	sphere2.origin = glm::vec3(-1.0f, 0.0f, -2.0f);
	sphere1.radius = radius;
	sphere2.radius = radius;
}

void setup()
{
	glEnable(GL_TEXTURE_2D);

	//generate and bind fbo
	glGenFramebuffers(1, &fboHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

	//generate the depth buffer
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowSize, WindowSize);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

	//Create positin, normal and color buffers
	createBufTex(GL_TEXTURE0, GL_RGBA16F, posTex);
	createBufTex(GL_TEXTURE1, GL_RGBA16F, normTex);
	createBufTex(GL_TEXTURE2, GL_RGBA8, colorTex);

	//Attach the texture to fbo 
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, posTex, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normTex, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, colorTex, 0);

	//This sets the render target for the given FBO
	GLenum drawbuf[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(3, drawbuf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Frame buffer not created. \n" << glCheckFramebufferStatus(GL_FRAMEBUFFER);

	//unbind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	createGeometry();

	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	PV = proj * view;

	LightPos = glGetUniformLocation(renderProgram, "pointLight.position");
	LightIntensity = glGetUniformLocation(renderProgram, "pointLight.Intensity");
	LightAmbient = glGetUniformLocation(renderProgram, "pointLight.Ambient");


	renderPlane.initBuffer();
}

// Functions called between every frame. game logic
#pragma region util_functions

// This runs once every physics timestep.
void update()
{
	
}

// This function runs every frame
void renderScene()
{
	glm::mat4 MVP;

	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to white
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	//First Render Call
	glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
	glViewport(0, 0, WindowSize, WindowSize);
	{
		//Sphere1
		// Set the uniform matrix in our shader to our MVP matrix for the first object.
		MVP = PV * glm::translate(glm::mat4(1), sphere1.origin);
		glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
		// Draw the Gameobjects
		glBindVertexArray(sphere1.base.vao);
		glBindBuffer(GL_ARRAY_BUFFER, sphere1.base.vbo);
		glDrawArrays(GL_TRIANGLES, 0, sphere1.base.numberOfVertices);

		//Sphere2
		// Set the uniform matrix in our shader to our MVP matrix for the first object.
		MVP = PV * glm::translate(glm::mat4(1), sphere2.origin);
		glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(MVP));
		// Draw the Gameobjects
		glBindVertexArray(sphere2.base.vao);
		glBindBuffer(GL_ARRAY_BUFFER, sphere2.base.vbo);
		glDrawArrays(GL_TRIANGLES, 0, sphere2.base.numberOfVertices);
	}
	
	// You have to clear the depth buffer before you render the screen filling quad, as that will cause it to cull the objects behind,
	// Thereby clearing the textures.
	glClear(GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	//Second render call
	glUseProgram(renderProgram);
	//Set the uniform variables
	glUniform3f(LightPos, -1.0f, 1.0f, 1.0f);
	glUniform3f(LightIntensity, 1.0f, 1.0f, 1.0f);
	glUniform3f(LightAmbient, 0.4f, 0.4f, 0.4f);
	glViewport(0, 0, WindowSize, WindowSize);
	{
		glBindVertexArray(renderPlane.vao);
		
		//Set the textures 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, posTex);
	
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normTex);
	
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, colorTex);

		//Render the quad.
		glDrawArrays(GL_TRIANGLES, 0, renderPlane.numberOfVertices);
	}
}

#pragma endregion Helper_functions


void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(WindowSize, WindowSize, "SDeferred Shading", nullptr, nullptr);

	std::cout << "This example demonstrates the implementation of deferred shading.";

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	setup();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
		update();

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


	// Frees up GLFW memory
	glfwTerminate();
}