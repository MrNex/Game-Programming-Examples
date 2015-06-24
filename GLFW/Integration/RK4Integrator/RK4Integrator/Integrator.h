/*
Title: Runge-Kutta 4 integration method
File Name: integrator.cpp
Copyright © 2015
Original authors: Srinivasan Thiagarajan
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
This is a program that shows the difference in integration mechanisms Runge kutta and
Euler integration method. The runge kutta method is of fourth order. Euler integrator,
takes the value of the function F(x,y(x)) and integrates it over the time step T.
Unlike Euler integrator, RK integrator, divides the entire time step into 3 parts and 
averages the slope of the velocity It uses that value of the y(x) to integrate over
the entire time step T. This results in having a reduced margin of error.

You can see the error margin between the two techniques. The red line uses the RK method,
while the blue line uses euler integrator. The red line is closer to its precise implementation.

use "space" to move one time Step.

References:
Nicholas Gallagher
Book : physics based animation by Kenny Erleben,Jon Sporring, Knud Henriksen and Henrik ;
Wikipedia : https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods ;
https://en.wikipedia.org/wiki/Midpoint_method

*/

#pragma once
#include "GLIncludes.h"

glm::vec2 AcceleratedVel(glm::vec2 Acc, glm::vec2 velocity, float h)
{
	// Calculate the change in velocity ina given time h, for an acceleration of Acc
	return velocity + (Acc*h);
}


glm::vec2 EulerIntegrator(glm::vec2 pos, float h, glm::vec2 &velocity, glm::vec2 Acceleration)
{
	glm::vec2 P;

	//Calculate the displacement in that time step with the current velocity.
	P = pos + (h * velocity);

	//calculate the velocity at the end of the timestep.
	velocity = AcceleratedVel(Acceleration, velocity, h);

	//return the position P
	return P;
}

glm::vec2 RK2Intergrator(glm::vec2 pos, float h, glm::vec2 &velocity, glm::vec2 Acceleration)
{
	/*
	K1 is the increment based on the slope at the beginning of the interval, uing y (euler's method)
	k2 is the increment based on the slope at the midpoint of the interval, using y + (h/2)k1
	k3 is the increment based on the slope at the midpoint of the interval, using y + (h/2)k2
	k4 is the increment based on the slope at the end of the interval, using y + h*k3

	k1-------------k2-----------------k3----------------k4
	|<--------------------- T -------------------------->|
	*/

	// Se t current position as pos
	glm::vec2 P = pos;

	glm::vec2 k1, k2, k3, k4, vel;
	glm::vec2 k;
	vel = velocity;

	k = EulerIntegrator(pos, 0.0f, vel, Acceleration);

	k1 = vel;
	
	//vel = velocity;

	k = EulerIntegrator(pos + (h * k1 / 2.0f), h / 2.0f, vel, Acceleration);
	k2 = vel;
	
	k = EulerIntegrator(pos + (h * k2 / 2.0f), h / 2.0f, vel, Acceleration);
	k3 = vel;

	k = EulerIntegrator(pos + (h * k3), h, vel, Acceleration);
	k4 = vel;

	//Use the velocity at the mid point to compute the displacement during the timestep h
	P += h * (k1 + (2.0f * k2) + (2.0f * k3) + k4) / 6.0f;

	//Change the velocity to the value at the end of the timestep.
	EulerIntegrator(pos, h, velocity, Acceleration);// AcceleratedVel(Acceleration, velocity, h / 2);

	//Return the poisiotn P
	return P;
}