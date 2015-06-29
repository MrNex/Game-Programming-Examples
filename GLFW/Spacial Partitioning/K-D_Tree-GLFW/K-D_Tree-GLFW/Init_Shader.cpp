#include "init_shader.h"
#include <fstream>
#include <iostream>

static char* textFileRead(char* fn)
{
	FILE* fp;
	char* content = NULL;
	errno_t err;

	int count = 0;

	if (fn != NULL)
	{
		err = fopen_s(&fp, fn, "rt");

		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0)
			{
				content = new char[count + 1];
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			err = fclose(fp);
		}
	}
	return content;
}

GLuint initShaders(char** shaders, GLenum* types, int numShaders)
{
	GLuint program = glCreateProgram();
	for (int i = 0; i < numShaders; ++i)
	{
		// Compile the vertex shader
		const char* source = textFileRead(shaders[i]);
		GLuint shader = glCreateShader(types[i]);
		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		glAttachShader(program, shader);
	}

	glBindFragDataLocation(program, 0, "outColor");

	glLinkProgram(program);

	glUseProgram(program);

	return program;
}

