#pragma once
#include "RenderShape.h"

struct Collider
{
	float width;
	float height;
	float x;
	float y;

	Collider()
	{
		width = 0.0f;
		height = 0.0f;
		x = 0.0f;
		y = 0.0f;
	}
};

class InteractiveShape : public RenderShape
{
public:
	InteractiveShape(Collider collider, GLint vao = 0, GLsizei count = 0, GLenum mode = 0, Shader shader = Shader(), glm::vec4 &color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	~InteractiveShape();

	void Update(float dt);

	void Draw(const glm::mat4& viewProjMat);

	const Collider& collider();
	bool mouseOver();
	bool mouseOut();
	bool moved();

private:

	bool _selected;
	bool _mouseOver;
	bool _mouseOut;
	bool _moved;
	Collider _collider;
};
