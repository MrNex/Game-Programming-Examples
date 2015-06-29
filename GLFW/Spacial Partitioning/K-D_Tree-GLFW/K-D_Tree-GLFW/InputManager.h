#pragma once
#include <glew\glew.h>
#include <glfw\glfw3.h>
#include <glm\glm.hpp>

class InputManager
{
public:
	static void Init(GLFWwindow* window);

	static void Update();

	static glm::vec2 GetMouseCoords();
	static bool leftMouseButton(bool prev = false);
	static bool downKey(bool prev = false);
	static bool upKey(bool prev = false);

private:

	static double _mousePos[2];
	static bool _leftMouseButton;
	static bool _prevLeftMouseButton;
	static bool _upKey;
	static bool _prevUpKey;
	static bool _downKey;
	static bool _prevDownKey;
	static GLFWwindow* _window;
	static float _aspectRatio;
	static int _windowSize[2];
};
