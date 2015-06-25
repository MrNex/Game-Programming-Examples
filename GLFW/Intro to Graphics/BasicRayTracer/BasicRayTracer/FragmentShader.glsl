/*
Title: Basic Ray Tracer
File Name: FragmentShader.glsl
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

References:
https://github.com/LWJGL/lwjgl3-wiki/wiki/2.6.1.-Ray-tracing-with-OpenGL-Compute-Shaders-(Part-I)

Description:
This program serves to demonstrate the concept of ray tracing. This
example is very basic, storing the triangles as a hardcoded constant
in the Fragment Shader itself. It draws a quad with the Vertex Shader,
and renders each pixel via tracing a ray through that pixel from the camera
position. There is no lighting.
*/

#version 400 // Identifies the version of the shader, this line must be on a separate line from the rest of the shader code

// The uniform variables, these storing the camera position and the four corner rays of the camera's view.
uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray01;
uniform vec3 ray10;
uniform vec3 ray11;

// The input textureCoord relative to the quad as given by the Vertex Shader.
in vec2 textureCoord;

// The output of the Fragment Shader, AKA the pixel color.
out vec4 color;

// Every one of our triangles contains 3 points, a normal, and a color.
struct triangle {
	vec3 a;
	vec3 b;
	vec3 c;
	vec3 normal;
	vec3 color;
};

// Create some constants
#define MAX_SCENE_BOUNDS 100.0
#define NUM_TRIANGLES 14

// Here we are setting the triangles up as constants in the shader code itself.
// Obviously, these could be passed in via uniform buffers as well. The key thing to realize here is that these values aren't being sent to the Vertex Shader.
const triangle triangles[] = triangle[14](
	/* Flat Box*/
	/* Top face triangles */
	triangle(vec3(-5.0, 0.0, 5.0), vec3(-5.0, 0.0, -5.0), vec3(5.0, 0.0, -5.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)),
	triangle(vec3(-5.0, 0.0, 5.0), vec3(5.0, 0.0, -5.0), vec3(5.0, 0.0, 5.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)),

	/* Cube Box */
	/* Back face triangles */
	triangle(vec3(-0.5, 1.0, -0.5), vec3(0.5, 1.0, -0.5), vec3(-0.5, 2.0, -0.5), vec3(0.0, 0.0, -1.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(0.5, 1.0, -0.5), vec3(0.5, 2.0, -0.5), vec3(-0.5, 2.0, -0.5), vec3(0.0, 0.0, -1.0), vec3(1.0, 0.0, 0.0)),
	/* Front face triangles*/
	triangle(vec3(-0.5, 1.0, 0.5), vec3(-0.5, 2.0, 0.5), vec3(0.5, 2.0, 0.5), vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(-0.5, 1.0, 0.5), vec3(0.5, 2.0, 0.5), vec3(0.5, 1.0, 0.5), vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0)),
	/* Right face triangles */
	triangle(vec3(0.5, 1.0, 0.5), vec3(0.5, 2.0, 0.5), vec3(0.5, 2.0, -0.5), vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(0.5, 1.0, 0.5), vec3(0.5, 2.0, -0.5), vec3(0.5, 1.0, -0.5), vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	/* Left face triangles */
	triangle(vec3(-0.5, 1.0, -0.5), vec3(-0.5, 2.0, -0.5), vec3(-0.5, 2.0, 0.5), vec3(-1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(-0.5, 1.0, -0.5), vec3(-0.5, 2.0, 0.5), vec3(-0.5, 1.0, 0.5), vec3(-1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	/* Top face triangles */
	triangle(vec3(-0.5, 2.0, 0.5), vec3(-0.5, 2.0, -0.5), vec3(0.5, 2.0, -0.5), vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(-0.5, 2.0, 0.5), vec3(0.5, 2.0, -0.5), vec3(0.5, 2.0, 0.5), vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	/* Bottom face triangles */
	triangle(vec3(-0.5, 1.0, 0.5), vec3(0.5, 1.0, 0.5), vec3(0.5, 1.0, -0.5), vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(-0.5, 1.0, 0.5), vec3(0.5, 1.0, -0.5), vec3(-0.5, 1.0, -0.5), vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0))
);

struct hitinfo
{
	vec3 point;
	int index;
};

// Determines whether or not a ray in a given direction hits a given triangle.
// Returns -1.0 if it does not; otherwise returns the value t at which the ray hits the triangle, which can be used to determine the point of collision.
// p is point on ray, d is ray direction, v0, v1, and v2 are points of the triangle.
float rayIntersectsTriangle(vec3 p, vec3 d, vec3 v0, vec3 v1, vec3 v2)
{
	vec3 e1,e2,h,s,q;
	float a,f,u,v, t;

	// Get two edges of triangle
	e1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
	e2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
	
	// Cross ray direction with triangle edge
	h = cross(d, e2);
	
	// Dot the other triangle edge with the above cross product
	a = dot(e1, h);

	// If a is zero or realy close to zero, then there's no collision.
	if (a > -0.00001 && a < 0.00001)
	{
		return -1.0;
	}

	// Take the inverse of a.
	f = 1/a;
	
	// Get vector from first triangle vertex toward cameraPos (or in the scope of this function, the vec3 p that is a point on the ray direction)
	s = vec3(p.x - v0.x, p.y - v0.y, p.z - v0.z);
	
	// Dot your s value with your h value from earlier (cross(d, e2)), then multiply by the inverse of a.
	u = f * dot(s, h);

	// If this value is not between 0 and 1, then there's no collision.
	if (u < 0.0 || u > 1.0)
	{
		return -1.0;
	}

	// Cross your s value with edge 1 (e1).
	q = cross(s, e1);

	// Dot the ray direction with this new q value, and then multiply by the inverse of a.
	v = f * dot(d, q);

	// If v is less than 0, or u + v are greater than 1, then there's no collision.
	if (v < 0.0 || u + v > 1.0)
	{
		return -1.0;
	}

	// At this stage we can compute t to find out where the intersection point is on the line
	t = f * dot(e2, q);

	// If t is greater than zero
	if (t > 0.00001)
	{
		// The ray does intersect the triangle, and we return the t value.
		return t;
	}
	
	// Otherwise, there is a line intersection, but not a ray intersection, so we return -1.0.
	return -1.0;
}

// Given an origin point, a direction, and a variable to pass information back out to, this will test a ray against every triangle in the scene.
// It will then return true or false, based on whether or not the ray collided with anything.
// If it did, then the hitinfo object will be filled with a point of collision and an index referring to which triangle it intersects with first.
bool intersectTriangles(vec3 origin, vec3 dir, out hitinfo info)
{
	// Start our variables for determining the closest triangle.
	// Smallest will be the smallest distance between the origin point and the point of collision.
	// Found just determines whether or not there was a collision at all.
	float smallest = MAX_SCENE_BOUNDS;
	bool found = false;

	// For each triangle.
	for(int i = 0; i < NUM_TRIANGLES; i++)
	{
		// Compute distance t using above function to determine how far along the ray the triangle collides.
		float t = rayIntersectsTriangle(origin, dir, triangles[i].a, triangles[i].b, triangles[i].c);

		// If t = -1.0 then there was no intersection, we also ignore it if t is not < smallest, as that would mean we already found a triangle that 
		// was closer (and thus collides first).
		if(t != -1.0 && t < smallest)
		{
			// This t becomes the new smallest.
			smallest = t;

			// color can be found via index as can the normal
			// Thus, we just pass out a point of collision using t and the triangle index.
			info.point = origin + (dir * t);
			info.index = i;

			// Make sure we set found to true, signifying that the ray collided with something.
			found = true;
		}
	}

	return found;
}

// Trace a ray from an origin point in a given direction and calculate/return the color value of the point that ray hits.
vec4 trace(vec3 origin, vec3 dir)
{
	// Create object to get our hitinfo back out of the intersectTriangles function.
	hitinfo i;

	// If this ray intersects any of the triangles in the scene.
	if (intersectTriangles(origin, dir, i))
	{
		// Return the final pixel color based on the triangle's color.
		return vec4(triangles[i.index].color.rgb, 1.0);
	}

	// If the ray doesn't hit any triangles, then this ray sees nothing and thus:
	return vec4(0.0, 0.0, 0.0, 1.0);
}

void main(void)
{
	// This is easy. Using the textureCoord, you interpolate between the four corner rays to get a ray (dir) that goes through a point in the screen.
	// For your mental image, imagine this shader runes once for every single pixel on your screen.
	// Every time it runs, dir is the ray that goes from the camera's position, through the pixel that it is rendering. Thus, we are tracing a ray through every pixel 
	// on the screen to determine what to render.
	vec2 pos = textureCoord;
	vec3 dir = normalize(mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x));
	color = trace(eye, dir);
}