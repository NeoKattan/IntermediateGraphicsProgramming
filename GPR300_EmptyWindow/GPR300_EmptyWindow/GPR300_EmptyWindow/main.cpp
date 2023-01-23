#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

//void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);

//TODO: Vertex shader source code
//const char* vertexShaderSource = "...";

//TODO: Fragment shader source code
//const char* fragmentShaderSource = "...";

//TODO: Vertex data array
//const float vertexData[] = { ... };

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGLExample", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);


	//TODO: Create and compile vertex shader
	//glCreateShader(...)
	//glShaderSource(...)
	//glCompileShader(...)

	//TODO: Get vertex shader compilation status and output info log
	//glGetShaderiv(...)
	//glGetShaderInfoLog(...)
	
	//TODO: Create and compile fragment shader
	//glCreateShader(...)
	//glShaderSource(...)
	//glCompileShader(...)

	//TODO: Get fragment shader compilation status and output info log
	//glGetShaderiv(...)
	//glGetShaderInfoLog(...)

	//TODO: Create shader program
	//glCreateProgram()

	//TODO: Attach vertex and fragment shaders to shader program
	//glAttachShader(...)

	//TODO: Link shader program
	//glLinkProgram(...)

	//TODO: Check for link status and output errors
	//glGetProgramiv(...)
	//glGetProgramInfoLog(...)

	//TODO: Delete vertex + fragment shader objects
	//glDeleteShader(...)

	//TODO: Create and bind Vertex Array Object (VAO)
	//glGenVertexArrays(...)
	//glBindVertexArray(...)

	//TODO: Create and bind Vertex Buffer Object (VBO), fill with vertexData
	//glGenBuffers(...)
	//glBindBuffer(...)
	//glBufferData(...)

	//TODO: Define vertex attribute layout
	//glVertexAttribPointer(...)
	//glEnableVertexAttribArray(...)

	while (!glfwWindowShouldClose(window)) {
		glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//TODO:Use shader program
		//glUseProgram(...)
		
		//TODO: Draw triangle (3 indices!)
		//glDrawArrays(...)

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

