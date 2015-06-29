#include "InteractiveShape.h"
#include "InputManager.h"

InteractiveShape::InteractiveShape(Collider collider, GLint vao, GLsizei count, GLenum mode, Shader shader, glm::vec4 &color)
{
	this->RenderShape::RenderShape(vao, count, mode, shader, color);
	_collider = collider;
	_mouseOver = false;
	_selected = false;
	_moved = false;
}

InteractiveShape::~InteractiveShape(){}

void InteractiveShape::Update(float dt)
{
	RenderShape::Update(dt);

	// Constrain position to the screen
	_transform.position.x = _transform.position.x > 1.337f ? 1.337f : _transform.position.x;
	_transform.position.x = _transform.position.x < -1.337f ? -1.337f : _transform.position.x;
	_transform.position.y = _transform.position.y > 1.0f ? 1.0f : _transform.position.y;
	_transform.position.y = _transform.position.y < -1.0f ? -1.0f : _transform.position.y;
	if (_transform.position.x == 1.337f || _transform.position.x == -1.337f)
	{
		_transform.linearVelocity.x *= -1.0f;
	}
	if (_transform.position.y == 1.0f || _transform.position.y == -1.0f)
	{
		_transform.linearVelocity.y *= -1.0f;
	}

	_mouseOut = false;
	_moved = false;
	// Check the mouse
	glm::vec2 mousePos = InputManager::GetMouseCoords();

	//Check against the collider
	float colTop = _transform.position.y + _collider.height / 2 + _collider.y;
	float colBot = _transform.position.y - _collider.height / 2 + _collider.y;
	float colRight = _transform.position.x + _collider.width / 2 + _collider.x;
	float colLeft = _transform.position.x - _collider.width / 2 + _collider.x;

	bool colliding = mousePos.x > colLeft && mousePos.x < colRight && mousePos.y > colBot && mousePos.y < colTop;
	if (colliding)
	{
		if (!_selected)
		{
			// If this is the first time that the mouse is down, this shape is now selected
			if (InputManager::leftMouseButton() && !InputManager::leftMouseButton(true))
			{
				_selected = true;
			}
		}
		_mouseOver = true;
	}
	else
	{
		if (_selected)
		{
			if (InputManager::leftMouseButton())
			{
				_transform.position.x = mousePos.x;
				_transform.position.y = mousePos.y;
			}
			else
			{
				_selected = false;
				_mouseOut = _mouseOver;
				_mouseOver = false;
				_moved = true;
			}
		}
		else
		{
			_mouseOut = _mouseOver;
			_mouseOver = false;
		}
	}
}

void InteractiveShape::Draw(const glm::mat4& viewProjMat)
{
	RenderShape::Draw(viewProjMat);
}

const Collider& InteractiveShape::collider()
{
	Collider ret = _collider;
	ret.x += _transform.position.x;
	ret.y += _transform.position.y;
	return ret;
}

bool InteractiveShape::mouseOver() { return _mouseOver; }
bool InteractiveShape::mouseOut() { return _mouseOut; }
bool InteractiveShape::moved() { return _moved; }