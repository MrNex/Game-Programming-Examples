/*
Title: Advanced Ray Tracer
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
builds off a previous Intermediate Ray Tracer, adding in reflections. 
There are four point lights, specular and diffuse lighting, and shadows. 
It is important to note that the light positions and triangles being 
rendered are all hardcoded in the shader itself. Usually, you would 
pass those values into the Fragment Shader via a Uniform Buffer.

WARNING: Framerate may suffer depending on your hardware. This is a normal 
problem with Ray Tracing. If it runs too slowly, try removing the second 
cube from the triangles array in the Fragment Shader (and also adjusting 
NUM_TRIANGLES accordingly). There are many optimization techniques out 
there, but ultimately Ray Tracing is not typically used for Real-Time 
rendering.
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
#define NUM_TRIANGLES 26

// Here we are setting the triangles up as constants in the shader code itself.
// Obviously, these could be passed in via uniform buffers as well. The key thing to realize here is that these values aren't being sent to the Vertex Shader.
const triangle triangles[] = triangle[26](
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
	triangle(vec3(-0.5, 1.0, 0.5), vec3(0.5, 1.0, -0.5), vec3(-0.5, 1.0, -0.5), vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0)),

	/* Cube Box 2 */
	/* Back face triangles */
	triangle(vec3(2.5, 3.5, 2.5), vec3(3.5, 3.5, 2.5), vec3(2.5, 4.5, 2.5), vec3(0.0, 0.0, -1.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(3.5, 3.5, 2.5), vec3(3.5, 4.5, 2.5), vec3(2.5, 4.5, 2.5), vec3(0.0, 0.0, -1.0), vec3(1.0, 0.0, 0.0)),
	/* Front face triangles*/
	triangle(vec3(2.5, 3.5, 1.5), vec3(2.5, 4.5, 1.5), vec3(3.5, 4.5, 1.5), vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(2.5, 3.5, 1.5), vec3(3.5, 4.5, 1.5), vec3(3.5, 3.5, 1.5), vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0)),
	/* Right face triangles */
	triangle(vec3(3.5, 3.5, 1.5), vec3(3.5, 4.5, 1.5), vec3(3.5, 4.5, 2.5), vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(3.5, 3.5, 1.5), vec3(3.5, 4.5, 2.5), vec3(3.5, 3.5, 2.5), vec3(1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	/* Left face triangles */
	triangle(vec3(2.5, 3.5, 2.5), vec3(2.5, 4.5, 2.5), vec3(2.5, 4.5, 1.5), vec3(-1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(2.5, 3.5, 2.5), vec3(2.5, 4.5, 1.5), vec3(2.5, 3.5, 1.5), vec3(-1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)),
	/* Top face triangles */
	triangle(vec3(2.5, 4.5, 1.5), vec3(2.5, 4.5, 2.5), vec3(3.5, 4.5, 2.5), vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(2.5, 4.5, 1.5), vec3(3.5, 4.5, 2.5), vec3(3.5, 4.5, 1.5), vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	/* Bottom face triangles */
	triangle(vec3(2.5, 3.5, 1.5), vec3(3.5, 3.5, 1.5), vec3(3.5, 3.5, 2.5), vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0)),
	triangle(vec3(2.5, 3.5, 1.5), vec3(3.5, 3.5, 2.5), vec3(2.5, 3.5, 2.5), vec3(0.0, -1.0, 0.0), vec3(1.0, 0.0, 0.0))
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

// Takes the position of a light, a vector from the point of collision toward the light, a vector direction from the origin toward the point of collision,
// a hitinfo object containing data in regards to the ray-triangle collision, and a float determining the brightness of a light.
// This will also factor in a single reflection off of the initial collided surface.
vec3 addToPixColor(vec3 lightPos, vec3 pointToLight, vec3 dir, hitinfo eyeHitPoint, float lightIntensity)
{
	// Get the distance from point on surface to light
	float dist = length(pointToLight);

	// Create a hitinfo object to store the collision.
	hitinfo lightRender;

	// Now we render things from the light point of view, so we call intersectTriangles passing in the light position as the origin.
	// The direction vector is from the light position toward the point on the triangle that we're trying to render.
	if(intersectTriangles(lightPos, normalize(eyeHitPoint.point - lightPos), lightRender))
	{
		// If the distance from the point to the light is farther than the distance from the light to the first surface it hits
		if(dist - length(lightPos - lightRender.point) > 0.1)
		{
			// Then this is in shadow, since the light is hitting another object first.
			return vec3(0.0, 0.0, 0.0);
		}
	}

	// Normalize our pointToLight.
	vec3 normalPTL = pointToLight / dist;

	// Get a reflection vector bouncing the light ray off the surface of the triangle.
	vec3 r = normalize((2 * dot(triangles[eyeHitPoint.index].normal, normalPTL) * triangles[eyeHitPoint.index].normal) - normalPTL);

	// The reflection Level variable determines how much of the original surface you see versus the reflection.
	float reflectionLevel = 0.5;
	// The reflection power variable determines the strength of the reflected light.
	float reflectionPower = 0.35;

	// Establish a pixColor variable at zero.
	vec3 pixColor = vec3(0.0, 0.0, 0.0);

	// Gets a vector in the direction of the reflected ray.
	vec3 reflectedEyeToPoint = normalize(dir - (2 * dot(dir, triangles[eyeHitPoint.index].normal) * triangles[eyeHitPoint.index].normal));
	
	// We're doing another collision test here to get the reflection.
	// You can see how this starts to get intensive and can slow down your framerate, since every one of these intersectTriangles calls tests a ray against 
	// every triangle in the scene. There are ways to optimize this (spatial partitioning), but ultimately Ray Tracing is not a technique for real-time rendering.
	hitinfo reflectHit;

	// If the reflected vector hits a triangle.
	if(intersectTriangles(eyeHitPoint.point, reflectedEyeToPoint, reflectHit))
	{
		// Get a vector from this reflected point of collision to the light.
		vec3 reflectPointToLight = lightPos - reflectHit.point;
		
		// Essentially, we're calculating things the same way here only we're factoring in the reflection.
		// So we calculate the reflected point of collision's surface color based on the dot product of a vector from it toward our light and the surface normal.
		// We divide by distance squared, giving less light the further away a point is.
		float diffuse = max(0, dot(triangles[reflectHit.index].normal, reflectPointToLight)) / pow(length(reflectPointToLight), 2);

		// Then we add the reflected color to our pixColor.
		pixColor += triangles[reflectHit.index].color * diffuse * lightIntensity * reflectionPower;
	}

	// Modify our pixColor by our reflection level (so if it's at .5, then half of reflected color will show through in this pixel)
	pixColor = pixColor * reflectionLevel;

	// Calculate specular and diffuse lighting normally.
	float specular = max(0, pow(dot(r,-dir), 4)) / pow(dist, 2);
	float diffuse = max(0, dot(triangles[eyeHitPoint.index].normal, normalPTL)) / pow(dist, 2);

	// Add in our diffuse light and specular (we do white light, for specula) and factor in the reflectionLevel and lightIntensity.
	pixColor += ((triangles[eyeHitPoint.index].color * diffuse * lightIntensity) + (lightIntensity * specular * vec3(1.0, 1.0, 1.0))) * (1 - reflectionLevel);
	
	// Then return the final color.
	return pixColor;
}

// Trace a ray from an origin point in a given direction and calculate/return the color value of the point that ray hits.
vec4 trace(vec3 origin, vec3 dir)
{
	// Create object to get our hitinfo back out of the intersectTriangles function.
	hitinfo i;

	// If this ray intersects any of the triangles in the scene.
	if (intersectTriangles(origin, dir, i))
	{
		// We now must factor in lights. In this case, we've created 4 lights.
		// This establishes positions for the lights, hardcoded into the shader.
		// Like the triangles above, these could be passed into a uniform object.
		vec3 light[4];
		light[0] = vec3(5.0, 3.0, 0.0);
		light[1] = vec3(-8.0, 5.0, 5.0);
		light[2] = vec3(5.0, 8.0, -5.0);
		light[3] = vec3(-5.0, 5.0, -5.0);
		
		// This gets us a vector from the point of collision toward the light.
		vec3 l[4];
		l[0] = light[0] - i.point;
		l[1] = light[1] - i.point;
		l[2] = light[2] - i.point;
		l[3] = light[3] - i.point;

		// This lightIntensity variable is used to multiply the brightness given off by this light. Higher value means more light.
		float lightIntensity = 6.0;

		// Create a pixColor variable, which will determine the output color of this pixel. Start with some ambient light.
		vec3 pixColor = triangles[i.index].color * 0.1;

		// For each light.
		for(int j = 0; j < 4; j++)
		{
			// Call our addToPixColor function to calculate the color given off by this light.
			// Essentially, we're rendering the scene from the light's point of view for each light to get this pixel color.
			pixColor += addToPixColor(light[j], l[j], dir, i, lightIntensity);
		}
		
		// Return the final pixel color.		
		return vec4(pixColor.rgb, 1.0);
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