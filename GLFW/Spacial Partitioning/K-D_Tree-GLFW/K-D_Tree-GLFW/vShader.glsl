#version 150

in vec2 position;
uniform mat4 transform;
uniform vec4 color;

out vec4 Color;

void main()
{
	Color = color;
	gl_Position = transform * vec4(position, 0.0, 1.0);
}