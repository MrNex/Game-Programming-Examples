#include "RenderShape.h"

RenderShape::RenderShape(GLint vao, GLsizei count, GLenum mode, Shader shader, const glm::vec4 &color)
{
	_vao = vao;
	_count = count;
	_mode = mode;
	_shader = shader;
	_color = color;
	_currentColor = color;

	_transform = Transform();
}
RenderShape::~RenderShape()
{

}

void RenderShape::Update(float dt)
{
	// Update values
	_transform.position += _transform.linearVelocity * dt;
	_transform.rotation = glm::slerp(_transform.rotation, _transform.rotation * _transform.angularVelocity, dt);

	_currentColor = _color;
}
void RenderShape::Draw(const glm::mat4& viewProjMat)
{
	if (_active)
	{
		// Apply transforms
		glm::mat4 translateMat = glm::translate(glm::mat4(), _transform.position);

		glm::mat4 rotateOriginMat = glm::translate(glm::mat4(), _transform.rotationOrigin);
		glm::mat4 rotateMat = rotateOriginMat * glm::mat4_cast(_transform.rotation) * glm::inverse(rotateOriginMat);

		glm::mat4 scaleOriginMat = glm::translate(glm::mat4(), _transform.scaleOrigin);
		glm::mat4 scaleMat = scaleOriginMat * glm::scale(glm::mat4(), _transform.scale) * glm::inverse(scaleOriginMat);

		glm::mat4 *parentModelMat = _transform.parent ? &_transform.parent->modelMat : &glm::mat4();

		_transform.modelMat = (*parentModelMat) * (translateMat * scaleMat* rotateMat);

		glm::mat4 transformMat = viewProjMat * _transform.modelMat;

		glBindVertexArray(_vao);

		glUniformMatrix4fv(_shader.uTransform, 1, GL_FALSE, glm::value_ptr(transformMat));
		glUniform4fv(_shader.uColor, 1, glm::value_ptr(_currentColor));

		//Make draw call
		glDrawElements(_mode, _count, GL_UNSIGNED_INT, 0);
	}
}

const glm::vec4& RenderShape::color()
{
	return _color;
}
glm::vec4& RenderShape::currentColor()
{
	return _currentColor;
}
Transform& RenderShape::transform()
{
	return _transform;
}
GLint RenderShape::vao()
{
	return _vao;
}
GLsizei RenderShape::count()
{
	return _count;
}
GLenum RenderShape::mode()
{
	return _mode;
}
Shader RenderShape::shader()
{
	return _shader;
}

bool& RenderShape::active()
{
	return _active;
}
