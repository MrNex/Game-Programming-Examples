/*
Title: Fluid HydroDynamics
File Name: main.cpp
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
In this example we use the previous "Fluid Simulation (SPH)" example to
demonstrate the hydrodynamics of fluid. 
Fluid when poured onto a a container which is connected to another by a 
pipe along the bottom, the fluid flows into the second container until 
the level of fluid is the same on both containers. This is because it 
need to balance the pressure on both sides. The pressure on each side is 
independant of the surface area and dependant on the height. 
So, if external pressure is applied on any one side, the fluid level changes 
or more accurately balances out the difference in p[ressure between the two containers.

In this example, all the fluid particles are released in one container and
they gradually flow into the adjacent container until there is equal liquid in both
the containers. 

Press "SHIFT" to start simulation
Use "SPACE" to toggle gravity in x-axis, or use "W" to toggle gravity in y-axis.

Changing gravity in x-axis will cause all the fluid to flow into the left container.

References:
Nicholas Gallagher
Lagrangian Fluid Dynamics Using Smoothed Particles Hydrodynamics by Micky Kelager
Particle-Based Fluid Simulation for Interactive Applications by Matthias Muller, David Charypar and Markus Gross
*/

#include "GLIncludes.h"

#define BoundarySizeX 1.0f
#define BoundarySizeY 1.0f
#define BoundarySizeZ 0.5f
#define Number_of_particels 400
#define	Grid_Size 10
#define K 5.0f											// Gas stiffness
#define DENSITY 998.29f
#define MASS (6000.0f/Number_of_particels)
#define VISCOSITY 0.001003f
#define SIGMA  0.0728f									// Surface tension
#define DAMPENING_CONSTANT -0.3f
#define COLOR_FIELD_THRESHOLD 7.065f 
#define POINTSIZE 20.0f
#define RADIUS (POINTSIZE/600.0f)
#define H  RADIUS * 4.0f									// Kernel Radius
#define pipeLength 0.5f

#pragma region program specific Data members
glm::vec3 POC(0.0f, 0.0f, 0.0f);
glm::vec3 G(0.0f, -9.8f, 0.0f);
float radius = 0.25f;
bool start = false;
#pragma endregion

//This struct consists of the basic stuff needed for getting the shape on the screen.
struct stuff_for_drawing{
	
	//This stores the address the buffer/memory in the GPU. It acts as a handle to access the buffer memory in GPU.
	GLuint vbo;

	//This will be used to tell the GPU, how many vertices will be needed to draw during drawcall.
	int numberOfVertices;

	//This function gets the number of vertices and all the vertex values and stores them in the buffer.
	void initBuffer(int numVertices, VertexFormat* vertices)
	{
		numberOfVertices = numVertices;

		// This generates buffer object names
		// The first parameter is the number of buffer objects, and the second parameter is a pointer to an array of buffer objects (yes, before this call, vbo was an empty variable)
		glGenBuffers(1, &vbo);
		
		//// Binds a named buffer object to the specified buffer binding point. Give it a target (GL_ARRAY_BUFFER) to determine where to bind the buffer.
		//// There are several different target parameters, GL_ARRAY_BUFFER is for vertex attributes, feel free to Google the others to find out what else there is.
		//// The second paramter is the buffer object reference. If no buffer object with the given name exists, it will create one.
		//// Buffer object names are unsigned integers (like vbo). Zero is a reserved value, and there is no default buffer for each target (targets, like GL_ARRAY_BUFFER).
		//// Passing in zero as the buffer name (second parameter) will result in unbinding any buffer bound to that target, and frees up the memory.
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		//// Creates and initializes a buffer object's data.
		//// First parameter is the target, second parameter is the size of the buffer, third parameter is a pointer to the data that will copied into the buffer, and fourth parameter is the 
		//// expected usage pattern of the data. Possible usage patterns: GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, 
		//// GL_DYNAMIC_READ, or GL_DYNAMIC_COPY
		//// Stream means that the data will be modified once, and used only a few times at most. Static means that the data will be modified once, and used a lot. Dynamic means that the data 
		//// will be modified repeatedly, and used a lot. Draw means that the data is modified by the application, and used as a source for GL drawing. Read means the data is modified by 
		//// reading data from GL, and used to return that data when queried by the application. Copy means that the data is modified by reading from the GL, and used as a source for drawing.
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * numVertices, vertices, GL_STATIC_DRAW);

		//// By default, all client-side capabilities are disabled, including all generic vertex attribute arrays.
		//// When enabled, the values in a generic vertex attribute array will be accessed and used for rendering when calls are made to vertex array commands (like glDrawArrays/glDrawElements)
		//// A GL_INVALID_VALUE will be generated if the index parameter is greater than or equal to GL_MAX_VERTEX_ATTRIBS
		glEnableVertexAttribArray(0);

		//// Defines an array of generic vertex attribute data. Takes an index, a size specifying the number of components (in this case, floats)(has a max of 4)
		//// The third parameter, type, can be GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
		//// The fourth parameter specifies whether to normalize fixed-point data values, the fifth parameter is the stride which is the offset (in bytes) between generic vertex attributes
		//// The fifth parameter is a pointer to the first component of the first generic vertex attribute in the array. If a named buffer object is bound to GL_ARRAY_BUFFER (and it is, in this case) 
		//// then the pointer parameter is treated as a byte offset into the buffer object's data.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)16);
		//// You'll note sizeof(VertexFormat) is our stride, because each vertex contains data that adds up to that size.
		//// You'll also notice we offset this parameter by 16 bytes, this is because the vec3 position attribute is after the vec4 color attribute. A vec4 has 4 floats, each being 4 bytes 
		//// so we offset by 4*4=16 to make sure that our first attribute is actually the position. The reason we put position after color in the struct has to do with padding.
		//// For more info on padding, Google it.

		//// This is our color attribute, so the offset is 0, and the size is 4 since there are 4 floats for color.
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	}
};

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float mass;
	float density;
	float viscosity;
} particles[Number_of_particels];

std::vector<Particle *> grid[Grid_Size][Grid_Size][Grid_Size];
std::vector<Particle *> gridLeft[Grid_Size][Grid_Size][Grid_Size];
std::vector<Particle *> neighbors[Number_of_particels];

void setup()
{
	float divisionX = BoundarySizeX / Grid_Size, divisionY = BoundarySizeY / Grid_Size, divisionZ = BoundarySizeZ / Grid_Size;

	int i, j, k, l;
	for (i = 0; i < Number_of_particels; i++)
	{
		particles[i].position.x = (float)(i % 3)*divisionX + (0.1f );	//* (i%2)
		particles[i].position.y = (float)((i / 3)% 3)*divisionY + 0.1f;
		particles[i].position.z = (float)((i / 9) % 3)*divisionZ +0.1f;
		
		particles[i].density = DENSITY;
		particles[i].mass = MASS;
		particles[i].viscosity = VISCOSITY;
		particles[i].velocity = glm::vec3(0.0f);
		particles[i].acceleration = glm::vec3(0.0f);
	}

	for (i = 0; i < Grid_Size; i++)
	{
		for (j = 0; j < Grid_Size; j++)
		{
			for (k = 0; k < Grid_Size; k++)
			{
				grid[i][j][k].clear();
				gridLeft[i][j][k].clear();
			}
		}
	}
}

void clear_tree()
{
	// Clear the grid. ( matrix of vectors )
	int i, j, k;
	for (i = 0; i < Grid_Size; i++)
	{
		for (j = 0; j < Grid_Size; j++)
		{
			for (k = 0; k < Grid_Size; k++)
			{
				grid[i][j][k].clear();
				gridLeft[i][j][k].clear();
			}
		}
	}
}

glm::vec3 EulerIntegrator(glm::vec3 pos, float h, glm::vec3 &velocity, glm::vec3 acc)
{
	glm::vec3 P;

	velocity += h * acc;

	//Calculate the displacement in that time step with the current velocity.
	P = pos + (h * velocity);

	//return the position P
	return P;
}

void catergorizeParticles()
{
	// This function categorizes each particle into the respectivel grid using its position's first value as the index for the grid.
	clear_tree();
	Particle p;
	float divisionX = BoundarySizeX / Grid_Size, divisionY = BoundarySizeY / Grid_Size, divisionZ = BoundarySizeZ / Grid_Size;
	int x, y, z;
	for (int i = 0; i < Number_of_particels; i++)
	{
		p = particles[i];

		//If the particle is farther to left than half of the pipe length(the pipe joining the two containers)
		//then the particle should be considered as a part of the left cylinder.
		if (p.position.x >= - pipeLength/2.0f)
			x = (int)floor(p.position.x / (divisionX));
		else
			x = (int)floor((p.position.x + BoundarySizeX + pipeLength)/ (divisionX));

		y = (int)floor(p.position.y / (divisionY));
		z = (int)floor(p.position.z / (divisionZ));

		x = (x < 0) ? 0 : x;
		y = (y < 0) ? 0 : y;
		z = (z < 0) ? 0 : z;
		
		x = (x > Grid_Size -1 ) ? Grid_Size -1 : x;
		y = (y > Grid_Size -1 ) ? Grid_Size -1 : y;
		z = (z > Grid_Size -1 ) ? Grid_Size -1 : z;
		
		if (p.position.x >= -pipeLength/2.0f)
			grid[x][y][z].push_back(&particles[i]);
		else
			gridLeft[x][y][z].push_back(&particles[i]);
		
	}
}

std::vector<Particle *> getNeighborsforPoint(int x, int y, int z, Particle r, std::vector<Particle *> (&Localgrid)[Grid_Size][Grid_Size][Grid_Size])
{
	// Get neighbors for a particle in a specific grid. 

	std::vector<Particle *> p;
	bool xFlagL = false, yFlagL = false , zFlagL = false;
	bool xFlagM = false, yFlagM = false, zFlagM = false;
	std::vector<Particle *>::iterator it;

	p.clear();

	for (it = (Localgrid)[x][y][z].begin(); it != (Localgrid)[x][y][z].end(); it++)
	{
		p.push_back((*it));
	}

	// We have to get the particles within the support raius. So, we take the number of grids which encompass 1 support radius. (H/boundarySize)
	for (int i = 1; i <= (H*Grid_Size) / BoundarySizeX; i++)
	{
		xFlagL = false; yFlagL = false; zFlagL = false;
		xFlagM = false; yFlagM = false; zFlagM = false;
		// X-axis
		if (x - i >= 0)
		{
			for (it = (Localgrid)[x - i][y][z].begin(); it != (Localgrid)[x - i][y][z].end(); it++)
			{
				p.push_back((*it));
			}
			xFlagL = true;
		}
		if (x + i < Grid_Size)
		{
			for (it = (Localgrid)[x + i][y][z].begin(); it != (Localgrid)[x + i][y][z].end(); it++)
			{
				p.push_back((*it));
			}
			xFlagM = true;
		}

		//Y axis
		if (y - i >= 0)
		{
			for (it = (Localgrid)[x][y - i][z].begin(); it != (Localgrid)[x][y - i][z].end(); it++)
			{
				p.push_back((*it));
			}
			yFlagL = true;

		}
		if (y + i < Grid_Size)
		{
			for (it = (Localgrid)[x][y + i][z].begin(); it != (Localgrid)[x][y + i][z].end(); it++)
			{
				p.push_back((*it));
			}
			yFlagM = true;
		}

		// Z-axis
		if (z - i >= 0)
		{
			for (it = (Localgrid)[x][y][z - i].begin(); it != (Localgrid)[x][y][z - i].end(); it++)
			{
				p.push_back((*it));
			}
			zFlagL = true;
		}
		if (z + i < Grid_Size)
		{
			for (it = (Localgrid)[x][y][z + i].begin(); it != (Localgrid)[x][y][z + i].end(); it++)
			{
				p.push_back((*it));
			}
			zFlagM = true;
		}


		if (xFlagL && yFlagL)
		{
			for (it = (Localgrid)[x - i][y - i][z].begin(); it != (Localgrid)[x - i][y - i][z].end(); it++)
			{
				p.push_back((*it));
			}

			if (zFlagL)
			{
				for (it = (Localgrid)[x - i][y - i][z - i].begin(); it != (Localgrid)[x - i][y - i][z - i].end(); it++)
				{
					p.push_back((*it));
				}
			}
			if (zFlagM)
			{
				for (it = (Localgrid)[x - i][y - i][z + i].begin(); it != (Localgrid)[x - i][y - i][z + i].end(); it++)
				{
					p.push_back((*it));
				}
			}
		}

		if (xFlagM && yFlagM)
		{
			for (it = (Localgrid)[x + i][y + i][z].begin(); it != (Localgrid)[x + i][y + i][z].end(); it++)
			{
				p.push_back((*it));
			}

			if (zFlagL)
			{
				for (it = (Localgrid)[x + i][y + i][z - i].begin(); it != (Localgrid)[x + i][y + i][z - i].end(); it++)
				{
					p.push_back((*it));
				}
			}
			if (zFlagM)
			{
				for (it = (Localgrid)[x + i][y + i][z + i].begin(); it != (Localgrid)[x + i][y + i][z + i].end(); it++)
				{
					p.push_back((*it));
				}
			}
		}

		if (xFlagL && yFlagM)
		{
			for (it = (Localgrid)[x - i][y + i][z].begin(); it != (Localgrid)[x - i][y + i][z].end(); it++)
			{
				p.push_back((*it));
			}

			if (zFlagL)
			{
				for (it = (Localgrid)[x - i][y + i][z - i].begin(); it != (Localgrid)[x - i][y + i][z - i].end(); it++)
				{
					p.push_back((*it));
				}
			}
			if (zFlagM)
			{
				for (it = (Localgrid)[x - i][y + i][z + i].begin(); it != (Localgrid)[x - i][y + i][z + i].end(); it++)
				{
					p.push_back((*it));
				}
			}
		}

		if (xFlagM && yFlagL)
		{
			for (it = (Localgrid)[x + i][y - i][z].begin(); it != (Localgrid)[x + i][y - i][z].end(); it++)
			{
				p.push_back((*it));
			}

			if (zFlagL)
			{
				for (it = (Localgrid)[x + i][y - i][z - i].begin(); it != (Localgrid)[x + i][y - i][z - i].end(); it++)
				{
					p.push_back((*it));
				}
			}
			if (zFlagM)
			{
				for (it = (Localgrid)[x + i][y - i][z + i].begin(); it != (Localgrid)[x + i][y - i][z + i].end(); it++)
				{
					p.push_back((*it));
				}
			}
		}
	}
	return p;
}

//get all the neighbors for respective particle
void getNeighbors()
{
	Particle p;
	float divisionX = BoundarySizeX / Grid_Size, divisionY = BoundarySizeY / Grid_Size, divisionZ = BoundarySizeZ / Grid_Size;
	int x, y, z;

	for (int i = 0; i < Number_of_particels; i++)
	{
		p = particles[i];
		if (p.position.x >= -pipeLength/2.0f)
			x = (int)floor(p.position.x / (divisionX));
		else
			x = (int)floor((p.position.x + BoundarySizeX + pipeLength)/ (divisionX));
		
		//x = (int)floor(p.position.x / divisionX);
		y = (int)floor(p.position.y / divisionY);
		z = (int)floor(p.position.z / divisionZ);

		x = (x < 0) ? 0 : x;
		y = (y < 0) ? 0 : y;
		z = (z < 0) ? 0 : z;

		x = (x > Grid_Size - 1) ? Grid_Size - 1 : x;
		y = (y > Grid_Size - 1) ? Grid_Size - 1 : y;
		z = (z > Grid_Size - 1) ? Grid_Size - 1 : z;
		
		if (p.position.x >= -pipeLength/2.0f)
			neighbors[i] = getNeighborsforPoint(x, y, z, particles[i], grid);
		else
			neighbors[i] = getNeighborsforPoint(x, y, z, particles[i], gridLeft);
	}
}

#pragma region 
//===============================================================
//						DENSITY
//===============================================================
//The smoothing kernel for density calculation
float smoothKernelPoly6(glm::vec3 r)
{
	//This smoothing kernel is used to compute the density of the particle. 
	//This kernel forms a sort of a bell-curve, which is what we want for density. 
	//We need the density to be MAX value and not INFINITY and decrease as the distance increases from 0.
	// For more information and graphs, please refer to the papers listed above.
	/*
		W(r,h) = 315 * (h^2 - |r|^2)/64 * pi * h^9				 When 0<= |r| <= h
		W(r,h) = 0												 When |r| > h

		h = support radius
	*/
	float R = glm::length(r);
	float result = (315.0f * powf((H * H) - (R*R), 3)) / (64.0f * PI * powf(H, 9));

	return result;
}

//Calculate the change in density
float densityChange(Particle &r, Particle &p)
{
	return p.mass * smoothKernelPoly6(r.position - p.position);
}

// Update the density of the a single particle
void updateParticleDensity(Particle &p, int counterValue)
{
	std::vector<Particle *> neighbor;
	
	//Get the neighbouring particles of the selected particle.
	neighbor = neighbors[counterValue];

	float density = 0.0f;

	// Update density of the current particle
	for (int i = 0; i < neighbor.size(); i++)
	{
		//For each particle in the selected particle's vicinity, computeteh effect on density
		if (glm::length(p.position - neighbor[i]->position) < H)
			density += densityChange(p, *neighbor[i]);
	}

	p.density = density;
	
	//Since a lot forces are inversly proportional to density, we set to a value slightly higher than 0, 
	//if it is 0. This prevent divide by 0 errors. 
	if (density == 0.0f)
	{
		p.density = FLT_EPSILON;
	}
}

// Update the densities of all the particles
void updateDensities()
{
	//Call the function for each particle.
	for (int i = 0; i < Number_of_particels; i++)
	{
		updateParticleDensity(particles[i], i);
	}
}
//----------------------------------------------------------------
#pragma endregion Density

#pragma region
//This function computes the gradient of the smooth Kernel
glm::vec3 smoothKernelPoly6Gradient(glm::vec3 r)
{
	/*
	gradient W(r,h) = -945 * r * (h^2 - |r|^2)/ 32 * pi * h 
	*/
	float R = glm::length(r);
	glm::vec3 result = r * (-954.0f * powf((H*H) - (R*R), 2));

	result /= (32 * PI * powf(H, 9));

	return result;
}

//This function computes the laplacian of the smooth Kernel
float smoothKernelPoly6Laplacian(glm::vec3 r)
{
	/*
	Laplacian W(r,h) = -945 * (h^2 - |r|^2) * (3h^2 - 7|r|^2) / 32 * pi * h^9
	*/
	float R = glm::length(r);
	float L = -945 / (32 * PI * powf(H, 9));

	L *= ((H * H) - (R * R)) * ((3.0f * H * H) - (7.0f * R * R));

	return L;
}

float smoothColorField(Particle &r, Particle &p)
{
	return p.mass * smoothKernelPoly6(r.position - p.position) / p.density;
}
#pragma endregion Surface Tension

#pragma region
//================================================================
//						PRESSURE
//================================================================
//spike kernel for calculating the pressure force
glm::vec3 spikeKernelPoly6Gradient(glm::vec3 r)
{
	/*
	We are using spike kernel to smooth pressure. We are using the spike 
	kernel because we need the pressure to increase alsmot exponentially as 
	the distance between the two positons decreases.  
	*/
	glm::vec3 grad(0.0f);;
	float R = glm::length(r);
	grad = r * (H - R) * (H - R) * (-45.0f);
	grad /= (PI * powf(H, 6) * fmax(R,FLT_EPSILON));

	return grad;
}

//Calculate the force experienced by a particle due to another particle
glm::vec3 pressureForcePerParticle(Particle &r, Particle &p)
{
	/*
		PV = nRT
		
		n = mass/MolarMAss = 1000g /18 = 55.55555
		R = 0.0083144621(75) amu (km/s)2 K−1
		T= 293.15 K

		V = mass/density
		P = nRT * mass / density
	*/

	float P1 = K * 13.533444f  * r.density / (r.mass), P2 = K * 13.533444f * p.density / (p.mass);		// Calculating the pressure.
	
	/*
	Here we compute the force caused due to pressure difference between the particles. 
	It is done by calculating the pressure difference between the two positions, 
	and use the spike kernel to calculatethe force.
	*/
	glm::vec3 fp = (P1 + P2) * p.mass * spikeKernelPoly6Gradient(r.position - p.position) / (2.0f * p.density);
	
	return fp;
}
//-----------------------------------------------------------------

#pragma endregion Pressure

#pragma region
//=================================================================
//						VISCOSITY
//=================================================================

//Calculate the force experienced by a particle due to another another particle's viscosity
glm::vec3 viscosityForcePerParticle(Particle &r, Particle &p)
{
	glm::vec3 fv;

	fv = (p.velocity - r.velocity) * p.mass * smoothKernelPoly6Laplacian(r.position - p.position) / p.density;

	return fv;
}
//------------------------------------------------------------------

#pragma endregion Viscosity

#pragma region
void resolveCollision(Particle &A, Particle &B)
{
	//This function resolves the collision between two particles.
	//There are two ways to resolve collision, using impulse based systems
	//or using the momentum and energy equations to calculate the final velocities.

	//glm::vec3 n = (B.velocity - A.velocity);
	glm::vec3 n = B.position - A.position;
	
	if (glm::length(n) > FLT_EPSILON)
	{
		n = glm::normalize(n);

		//This section of commented code decouples the two particles
		// We have commented out this code, as it makes the demo more jittery
		//A.position += m / 2.0f * n;
		//B.position -= m / 2.0f * n;

		glm::vec3 An = glm::dot(A.velocity, n) * n;
		glm::vec3 Bn = glm::dot(B.velocity, n) * n;

		float nMagA = glm::dot(A.velocity, n);
		float nMagB = glm::dot(B.velocity, n);

		// Apn and Bpn store those component of velocity which will not be affected in this collision
		glm::vec3 Apn = A.velocity - An;
		glm::vec3 Bpn = B.velocity - Bn;

		float relVel = nMagA - nMagB;

		float num;
		num = -2.0f * relVel;
		float denom;
		denom = A.mass + B.mass; //1.0f/A.mass + 1.0f/B.mass;

		// Now that we have the velocities in that one dimension, solve them and add the components to the respective
		// resultant to get the final velocities.
		glm::vec3 u1, u2;
		u1 = An;
		u2 = Bn;

		float j = num / denom;
		//A.velocity += j * n / A.mass;
		//B.velocity -= j * n / B.mass;

		A.velocity = ((((A.mass - B.mass)*u1) + 2 * B.mass*u2) / denom) + Apn;
		B.velocity = ((2 * A.mass*u1 + ((B.mass - A.mass)*u2)) / denom) + Bpn;
	}

}

bool detectCollision(Particle &A, Particle &B)
{
	if (glm::length(A.position - B.position) < RADIUS * 2)
		return true;

	return false;
}

void findAndResolveCollisions()
{
	std::vector<Particle *>::iterator it,xy;

	for (int i = 0; i < Grid_Size; i++)
	{
		for (int j = 0; j < Grid_Size; j++)
		{
			for (int k = 0; k < Grid_Size; k++)
			{
				for (it = grid[i][j][k].begin(); it != grid[i][j][k].end(); it++)
				{
					xy = it;
					xy++;
					for (; xy != grid[i][j][k].end(); xy++)
					{
						if (detectCollision(**it, **xy))
							resolveCollision(**it, **xy);
					}
				}

				for (it = gridLeft[i][j][k].begin(); it != gridLeft[i][j][k].end(); it++)
				{
					xy = it;
					xy++;
					for (; xy != gridLeft[i][j][k].end(); xy++)
					{
						if (detectCollision(**it, **xy))
							resolveCollision(**it, **xy);
					}
				}
			}
		}
	}
	
}
#pragma endregion Collision

void boundVelocities(std::vector<Particle *>(&Localgrid)[Grid_Size][Grid_Size][Grid_Size], bool left)
{
	std::vector<Particle *>::iterator it;

	// This function goes through all the edge grids.
	// For all the particles there, it checks if they leave the bounding area. 
	// If they are outside the bounding area, then check if they continue to 
	// move outwards of the bounding volume, then change the component of velocity
	// which is along the surface normal.
	
	float horizontalBoundaryMin,horizontalBoundaryMax;

	if (!left)
	{
		horizontalBoundaryMin = 0.0f;
		horizontalBoundaryMax = BoundarySizeX;
	}
	else
	{
		horizontalBoundaryMax = 0.0f -pipeLength;
		horizontalBoundaryMin = horizontalBoundaryMax - BoundarySizeX;
	}

	for (int i = 0; i < Grid_Size; i++)
	{
		for (int j = 0; j < Grid_Size; j++)
		{
			//The part constituting the pipe :: this is done to remove restrictions along the pipe (create an opening for the water to flow out of)
			// This should be done of the both the containers as the liquid can flow in and out of both the containers.
			if (i != 0 || (j != Grid_Size / 2 && j != (Grid_Size / 2) + 1) || left)
			{
				for (it = Localgrid[0][i][j].begin(); it != Localgrid[0][i][j].end(); it++)
				{
					if ((*it)->position.x < horizontalBoundaryMin && ((*it)->velocity.x < 0 /*|| (*it)->acceleration.x < 0*/))
					{
						(*it)->velocity.x *= DAMPENING_CONSTANT;
						(*it)->position.x = horizontalBoundaryMin;
					}
				}
			}

			//X-axis
			if (i != 0 || (j != Grid_Size / 2 && j != (Grid_Size / 2) + 1) || !left)
			{
				for (it = Localgrid[Grid_Size - 1][i][j].begin(); it != Localgrid[Grid_Size - 1][i][j].end(); it++)
				{
					if ((*it)->position.x > horizontalBoundaryMax && ((*it)->velocity.x > 0 /*|| (*it)->acceleration.x > 0 */))
					{
						(*it)->velocity.x *= DAMPENING_CONSTANT;
						(*it)->position.x = horizontalBoundaryMax;
					}
				}
			}

			//Y-Axis

			for (it = Localgrid[i][0][j].begin(); it != Localgrid[i][0][j].end(); it++)
			{
				if ((*it)->position.y < 0 && ((*it)->velocity.y < 0 || (*it)->acceleration.y < 0))
				{
					(*it)->velocity.y *= -0.1f;// DAMPENING_CONSTANT;
					(*it)->position.y = 0.0f;
				}
			}

			for (it = Localgrid[i][Grid_Size - 1][j].begin(); it != Localgrid[i][Grid_Size - 1][j].end(); it++)
			{
				if ((*it)->position.y > BoundarySizeY && ((*it)->velocity.y > 0 || (*it)->acceleration.y > 0))
				{
					(*it)->velocity.y *= DAMPENING_CONSTANT;
					(*it)->position.y = BoundarySizeY;
				}
			}

			//Z-axis
			for (it = Localgrid[i][j][0].begin(); it != Localgrid[i][j][0].end(); it++)
			{
				if ((*it)->position.z < 0 && ((*it)->velocity.z < 0 || (*it)->acceleration.z < 0))
				{
					(*it)->velocity.z *= DAMPENING_CONSTANT;
					(*it)->position.z = 0.0f;
				}
			}
			for (it = Localgrid[i][j][Grid_Size - 1].begin(); it != Localgrid[i][j][Grid_Size - 1].end(); it++)
			{
				if ((*it)->position.z > BoundarySizeZ && ((*it)->velocity.z > 0 || (*it)->acceleration.z > 0))
				{
					(*it)->velocity.z *= DAMPENING_CONSTANT;
					(*it)->position.z = BoundarySizeZ;
				}
			}
		
		}
	}
}

void updateVelocities()
{
	/*
	This function updates the acceleration of each particle.
	*/

	std::vector<Particle *>::iterator it;
	float k;													// Smoothed Color
	glm::vec3 Fpressure, Fviscosity, n, Fexternal, Fsurface, Finternal, Ftotal;
	float l;
	for (int i = 0; i < Number_of_particels; i++)
	{
		//Reset the values in forces
		Fpressure = glm::vec3(0.0f);
		Fviscosity = glm::vec3(0.0f);
		Fexternal = glm::vec3(0.0f);
		Finternal = glm::vec3(0.0f);
		Fsurface = glm::vec3(0.0f);
		k = 0;
		n = Fpressure;

		for (it = neighbors[i].begin(); it != neighbors[i].end(); it++)
		{ 
			l = glm::length(particles[i].position - (*it)->position);

			if ( l <= H && l > 0.0f)
			{
				Fpressure += pressureForcePerParticle(particles[i], *(*it));
				Fviscosity += viscosityForcePerParticle(particles[i], *(*it));
				// n is the direction of surface tension force.
				// For particles which are not in the outside surface n will sum upto 0.
				// For all the particles in the surface, the value will be non zero.
				n += ((*it)->mass * smoothKernelPoly6Gradient(particles[i].position - (*it)->position)) / (*it)->density;
				k += ((*it)->mass * smoothKernelPoly6Laplacian(particles[i].position - (*it)->position)) / (*it)->density;
			}
		}
		
		Fpressure *= -1.0f;
		Fviscosity *= particles[i].viscosity;
		
		float CFNLength = glm::length(n);
		//Calculate the surface tension force
		if (CFNLength > COLOR_FIELD_THRESHOLD && CFNLength != 0.0f)
		{
			Fsurface = (-SIGMA) * k * (n / CFNLength);
		}
		
		Finternal = Fviscosity + Fpressure;

		Fexternal = (G * particles[i].density) + Fsurface;

		Ftotal = Finternal + Fexternal;
		
		particles[i].acceleration = Ftotal / particles[i].density;
	}

	boundVelocities(grid,false);
	boundVelocities(gridLeft, true);
}

void integrate(float dt)
{
	//std::cout << "\n ";
	for (int i = 0; i < Number_of_particels; i++)
	{
		particles[i].velocity.z = 0.0f;
		particles[i].position = EulerIntegrator(particles[i].position, dt, particles[i].velocity, particles[i].acceleration);
		//std::cout << "\n particle : " << particles[i].velocity.z;
	}
}

#pragma region Global Data member
// Global data members
// This is your reference to your shader program.
// This will be assigned with glCreateProgram().
// This program will run on your GPU.
GLuint program;

// These are your references to your actual compiled shaders
GLuint vertex_shader;
GLuint fragment_shader;

// This is a reference to your uniform MVP matrix in your vertex shader
GLuint uniMVP;
GLuint color;

glm::mat4 view;
glm::mat4 proj;
glm::mat4 PV;
glm::mat4 MVP;


double time = 0.0;
double timebase = 0.0;
double accumulator = 0.0;
double physicsStep = 0.012; // This is the number of milliseconds we intend for the physics to update.


// Reference to the window object being created by GLFW.
GLFWwindow* window;
#pragma endregion

#pragma region Helper_functions
// Functions called only once every time the program is executed.
std::string readShader(std::string fileName)
{
	std::string shaderCode;
	std::string line;

	// We choose ifstream and std::ios::in because we are opening the file for input into our program.
	// If we were writing to the file, we would use ofstream and std::ios::out.
	std::ifstream file(fileName, std::ios::in);

	// This checks to make sure that we didn't encounter any errors when getting the file.
	if (!file.good())
	{
		std::cout << "Can't read file: " << fileName.data() << std::endl;

		// Return so we don't error out.
		return "";
	}

	// ifstream keeps an internal "get" position determining the location of the element to be read next
	// seekg allows you to modify this location, and tellg allows you to get this location
	// This location is stored as a streampos member type, and the parameters passed in must be of this type as well
	// seekg parameters are (offset, direction) or you can just use an absolute (position).
	// The offset parameter is of the type streamoff, and the direction is of the type seekdir (an enum which can be ios::beg, ios::cur, or ios::end referring to the beginning, 
	// current position, or end of the stream).
	file.seekg(0, std::ios::end);					// Moves the "get" position to the end of the file.
	shaderCode.resize((unsigned int)file.tellg());	// Resizes the shaderCode string to the size of the file being read, given that tellg will give the current "get" which is at the end of the file.
	file.seekg(0, std::ios::beg);					// Moves the "get" position to the start of the file.

	// File streams contain two member functions for reading and writing binary data (read, write). The read function belongs to ifstream, and the write function belongs to ofstream.
	// The parameters are (memoryBlock, size) where memoryBlock is of type char* and represents the address of an array of bytes are to be read from/written to.
	// The size parameter is an integer that determines the number of characters to be read/written from/to the memory block.
	file.read(&shaderCode[0], shaderCode.size());	// Reads from the file (starting at the "get" position which is currently at the start of the file) and writes that data to the beginning
	// of the shaderCode variable, up until the full size of shaderCode. This is done with binary data, which is why we must ensure that the sizes are all correct.

	file.close(); // Now that we're done, close the file and return the shaderCode.

	return shaderCode;
}

// This method will consolidate some of the shader code we've written to return a GLuint to the compiled shader.
// It only requires the shader source code and the shader type.
GLuint createShader(std::string sourceCode, GLenum shaderType)
{
	// glCreateShader, creates a shader given a type (such as GL_VERTEX_SHADER) and returns a GLuint reference to that shader.
	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = sourceCode.c_str(); // We establish a pointer to our shader code string
	const int shader_code_size = sourceCode.size();   // And we get the size of that string.

	// glShaderSource replaces the source code in a shader object
	// It takes the reference to the shader (a GLuint), a count of the number of elements in the string array (in case you're passing in multiple strings), a pointer to the string array 
	// that contains your source code, and a size variable determining the length of the array.
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader); // This just compiles the shader, given the source code.

	GLint isCompiled = 0;

	// Check the compile status to see if the shader compiled correctly.
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		char infolog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, infolog);

		// Print the compile error.
		std::cout << "The shader failed to compile with the error:" << std::endl << infolog << std::endl;

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		// NOTE: I almost always put a break point here, so that instead of the program continuing with a deleted/failed shader, it stops and gives me a chance to look at what may 
		// have gone wrong. You can check the console output to see what the error was, and usually that will point you in the right direction.
	}

	return shader;
}

// Initialization code
void init()
{
	// Initializes the glew library
	glewInit();

	// Enables the depth test, which you will want in most cases. You can disable this in the render loop if you need to.
	glEnable(GL_DEPTH_TEST);

	// Read in the shader code from a file.
	std::string vertShader = readShader("VertexShader.glsl");
	std::string fragShader = readShader("FragmentShader.glsl");

	// createShader consolidates all of the shader compilation code
	vertex_shader = createShader(vertShader, GL_VERTEX_SHADER);
	fragment_shader = createShader(fragShader, GL_FRAGMENT_SHADER);

	// A shader is a program that runs on your GPU instead of your CPU. In this sense, OpenGL refers to your groups of shaders as "programs".
	// Using glCreateProgram creates a shader program and returns a GLuint reference to it.
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);		// This attaches our vertex shader to our program.
	glAttachShader(program, fragment_shader);	// This attaches our fragment shader to our program.

	// This links the program, using the vertex and fragment shaders to create executables to run on the GPU.
	glLinkProgram(program);
	// End of shader and program creation

	// Creates the view matrix using glm::lookAt.
	// First parameter is camera position, second parameter is point to be centered on-screen, and the third paramter is the up axis.
	view = glm::lookAt(glm::vec3(-pipeLength / 2.0f, 0.5f, 4.0f), glm::vec3(-pipeLength / 2.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	// Creates a projection matrix using glm::perspective.
	// First parameter is the vertical FoV (Field of View), second paramter is the aspect ratio, 3rd parameter is the near clipping plane, 4th parameter is the far clipping plane.
	proj = glm::perspective(45.0f, 800.0f / 800.0f, 0.1f, 100.0f);

	PV = proj * view;

	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	MVP = PV* translation;
	
	// This gets us a reference to the uniform variable in the vertex shader, which is called "color".
	// We're using this variable to change color during runtime, without changing the buffer values.
	// Only 2 parameters required: A reference to the shader program and the name of the uniform variable within the shader code.
	uniMVP = glGetUniformLocation(program, "MVP");
	
	// This is not necessary, but I prefer to handle my vertices in the clockwise order. glFrontFace defines which face of the triangles you're drawing is the front.
	// Essentially, if you draw your vertices in counter-clockwise order, by default (in OpenGL) the front face will be facing you/the screen. If you draw them clockwise, the front face 
	// will face away from you. By passing in GL_CW to this function, we are saying the opposite, and now the front face will face you if you draw in the clockwise order.
	// If you don't use this, just reverse the order of the vertices in your array when you define them so that you draw the points in a counter-clockwise order.
	glFrontFace(GL_CCW);

	// This is also not necessary, but more efficient and is generally good practice. By default, OpenGL will render both sides of a triangle that you draw. By enabling GL_CULL_FACE, 
	// we are telling OpenGL to only render the front face. This means that if you rotated the triangle over the X-axis, you wouldn't see the other side of the triangle as it rotated.
	glEnable(GL_CULL_FACE);
	//We are disabling hte cull face, because we wish to see both the front and back of the objects in wireframe mode for better understanding the depth.

	// Determines the interpretation of polygons for rasterization. The first parameter, face, determines which polygons the mode applies to.
	// The face can be either GL_FRONT, GL_BACK, or GL_FRONT_AND_BACK
	// The mode determines how the polygons will be rasterized. GL_POINT will draw points at each vertex, GL_LINE will draw lines between the vertices, and 
	// GL_FILL will fill the area inside those lines.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

#pragma endregion

#pragma region util_functions
// Functions called between every frame. game logic

// This runs once every physics timestep.
void update(float t)
{
	//Catergorize the particles into their respective grids.
	catergorizeParticles();

	//Each particles collects info on the particles surrounding them
	getNeighbors();

	//Update the densities at each particle location
	updateDensities();
	
	//update the acceleration of each particle
	if (start)
		updateVelocities();
	
	//Resolve collisions
	findAndResolveCollisions();

	//Integrate the particle (update the 
	integrate(t);
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
	glClearColor(0.2f, 0.2f, 0.2f, 1.0);

	// Tell OpenGL to use the shader program you've created.
	glUseProgram(program);

	glLineWidth(1.0f);
	
	glm::vec3 p;
	glm::mat4 m;
	glEnable(GL_POINT_SMOOTH);
	glUniformMatrix4fv(uniMVP, 1, GL_FALSE, glm::value_ptr(PV));
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(POINTSIZE);
	
	glBegin(GL_POINTS);
	
	for (int i = 0; i < Number_of_particels; i++)
	{
		glVertex3fv((float*)&particles[i].position);
	}
	glEnd();
}

// This function is used to handle key inputs.
// It is a callback funciton. i.e. glfw takes the pointer to this function (via function pointer) and calls this function every time a key is pressed in the during event polling.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS))
	{
		if (G.x >= 0)
		{
			G.x = -14.8f;
		}
		else
			G.x = 0;
	}

	
	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if (G.y == 0)
		{
			G.y = -9.8f;
		}
		else
			G.y = 0.0f;
	}
	
	if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		start = true;
	}

	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		particles[0].velocity += glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		particles[0].velocity += glm::vec3(0.1f, 0.0f, 0.0f);
	}

}
#pragma endregion

void main()
{
	glfwInit();

	// Creates a window given (width, height, title, monitorPtr, windowPtr).
	// Don't worry about the last two, as they have to do with controlling which monitor to display on and having a reference to other windows. Leaving them as nullptr is fine.
	window = glfwCreateWindow(800, 800, "Fluid (SPH)", nullptr, nullptr);

	std::cout << "\n This program demonstrates implementation of angular friction \n\n\n\n\n\n\n\n\n\n";
	std::cout << "\n Press \"SHIFT\" to start simulation.";
	std::cout << "\n Use \"SPACE\" to toggle gravity in x - axis.";
	std::cout << "\n use \"W\" to toggle gravity in y - axis.";
	
	// Makes the OpenGL context current for the created window.
	glfwMakeContextCurrent(window);

	// Sets the number of screen updates to wait before swapping the buffers.
	// Setting this to zero will disable VSync, which allows us to actually get a read on our FPS. Otherwise we'd be consistently getting 60FPS or lower, 
	// since it would match our FPS to the screen refresh rate.
	// Set to 1 to enable VSync.
	glfwSwapInterval(0);

	// Initializes most things needed before the main loop
	init();

	// Sends the funtion as a funtion pointer along with the window to which it should be applied to.
	glfwSetKeyCallback(window, key_callback);

	setup();

	// Enter the main loop.
	while (!glfwWindowShouldClose(window))
	{
		// Call to update() which will update the gameobjects.
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


	// Frees up GLFW memory
	glfwTerminate();
}