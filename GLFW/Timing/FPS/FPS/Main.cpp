/*
Title: FPS
File Name: Main.cpp
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
Contains code to calculate an average FPS every second and display it in the window title.
*/

#include "glfw\glfw3.h"
#include <iostream>
#include <string>

// Variables you will need to calculate FPS.
int frame;
double time;
double timebase;
int fps;

// A pointer to the window we will be creating.
GLFWwindow* window;

// This function runs every frame before the render function
void update()
{
	time = glfwGetTime(); // Gets the elapsed time, in seconds, since glfwInit() was called.

	// If there has been more than 1 second since the last time we calculated the FPS
	if (time - timebase > 1)
	{
		// Calculate FPS: Take the number of frames (frame) since the last time we calculated FPS, and divide by the amount of time that has passed since the 
		// last time we calculated FPS (time - timebase).
		fps = frame / (time - timebase);
		timebase = time;	// Now we set timebase = time, so that we have a reference for when we calculated the FPS
		frame = 0;			// And we reset our frame counter to 0, to mark that 0 frames have passed since we calculated FPS (since we literally just did it)

		std::string s = "FPS: " + std::to_string(fps); // This just creates a string that looks like "FPS: 60" or however much.

		glfwSetWindowTitle(window, s.c_str()); // This will set the window title to that string, displaying the FPS as the window title.
	}
}

// This function runs every frame
void renderScene(void)
{
	// Clear the color buffer and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear the screen to red
	glClearColor(1.0, 0.0, 0.0, 1.0);
}

int main(int argc, char **argv)
{
	frame = 0;
	time = 0.0;
	timebase = 0.0;
	fps = 0;

	// Initializes the GLFW library
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 600, "FPS", nullptr, nullptr);

	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS (otherwise we'd be consistently getting 60FPS or lower, since it would match our FPS 
	// to the screen refresh rate.
	glfwSwapInterval(0);

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call the update function
		update();

		// Call the render function.
		renderScene();

		// Swaps the back buffer to the front buffer
		// Remember, you're rendering to the back buffer, then once rendering is complete, you're moving the back buffer to the front so it can be displayed.
		glfwSwapBuffers(window);

		// Now that we've finished the renderScene function and swapped the buffers, we can say that a frame has completed, and thus we add one to our frame counter.
		frame++;

		// Checks to see if any events are pending and then processes them.
		glfwPollEvents();
	}

	// Frees up GLFW memory
	glfwTerminate();

	return 0;
}