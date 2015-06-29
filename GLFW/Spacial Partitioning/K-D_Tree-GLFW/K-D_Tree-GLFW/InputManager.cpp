#include "InputManager.h"
#include <iostream>

double InputManager::_mousePos[2];
bool InputManager::_leftMouseButton = false;
bool InputManager::_prevLeftMouseButton = false;
bool InputManager::_upKey = false;
bool InputManager::_prevUpKey = false;
bool InputManager::_downKey = false;
bool InputManager::_prevDownKey = false;
GLFWwindow* InputManager::_window;
float InputManager::_aspectRatio = 0.0f;
int InputManager::_windowSize[2];

void InputManager::Init(GLFWwindow* window)
{
	_window = window;
	glfwGetWindowSize(window, &_windowSize[0], &_windowSize[1]);
	_aspectRatio = (float)_windowSize[0] / (float)_windowSize[1];
}

void InputManager::Update()
{
	_prevLeftMouseButton = _leftMouseButton;
	_leftMouseButton = glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	_prevDownKey = _downKey;
	_downKey = glfwGetKey(_window, GLFW_KEY_DOWN) == GLFW_PRESS;
	_prevUpKey = _upKey;
	_upKey = glfwGetKey(_window, GLFW_KEY_UP) == GLFW_PRESS;
	glfwGetCursorPos(_window, &_mousePos[0], &_mousePos[1]);
}

glm::vec2 InputManager::GetMouseCoords()
{
	glm::vec2 ret = glm::vec2();
	ret.x = (((float)_mousePos[0] / (float)_windowSize[0]) * 2.0f - 1.0f) * _aspectRatio;
	ret.y = -(((float)_mousePos[1] / (float)_windowSize[1]) * 2.0f - 1.0f);
	return ret;
}
bool InputManager::leftMouseButton(bool prev) { if (prev) return _prevLeftMouseButton; else return _leftMouseButton; }
bool InputManager::upKey(bool prev) { if (prev) return _prevUpKey; else return _upKey; }
bool InputManager::downKey(bool prev) { if (prev) return _prevDownKey; else return _downKey; }
