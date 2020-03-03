// OpenglTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
// vcpkg install --triplet x64-windows glew glm

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include "GLFW/glfw3.h"
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include "common/shader.hpp"

#include <iostream>

int main()
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "AudioScope", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	//GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
	//GLuint programID = LoadShaders("line.vert", "line.frag");
	//GLuint programID = LoadShaders("line.vert", "SimpleFragmentShader.fragmentshader", nullptr);
	//GLuint programID = LoadShaders("line.vert", "line.frag", nullptr);
	//GLuint programID = LoadShaders("line/line.vert", "line/line.frag", "line/line.geom");
	//GLuint programID = LoadShaders("shaders/simple/simple.vert", "shaders/simple/simple.frag", nullptr);
	//GLuint programID = LoadShaders("shaders/simple/simple.vert", "shaders/simple/simple.frag", "shaders/simple/simple.geom");
	//GLuint programID = LoadShaders("shaders/line/line.vert", "shaders/line/line.frag", nullptr);
	GLuint programID = LoadShaders("shaders/line/line.vert", "shaders/line/line.frag", "shaders/line/line.geom");

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -0.5f, 0.0f, 1.0f,
		-0.5f, 0.25f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.25f, 0.0f, 1.0f, 
		1.0f, 0.5f, 0.0f, 1.0f,
	};

	//float points[] = {
	//	-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // top-left
	//	0.5f, 0.5f, 0.0f, 1.0f, 0.0f, // top-right
	//	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
	//	-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
	//};

	//static const GLfloat g_vertex_buffer_data[259] = { 0.5f };

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//static const GLfloat g_color_buffer_data[] = {
	//	1.0f, 0.0f, 0.0f,
	//	1.0f, 0.0f, 0.0f,
	//	1.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 1.0f,
	//	0.0f, 0.0f, 1.0f,
	//	0.0f, 0.0f, 1.0f,
	//};

	//GLuint colorbuffer;
	//glGenBuffers(1, &colorbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	// Get a handle for our "n" uniform
	GLuint windowID = glGetUniformLocation(programID, "window");
	GLuint nID = glGetUniformLocation(programID, "n");
	GLuint baseHueID = glGetUniformLocation(programID, "base_hue");
	GLuint colorizeID = glGetUniformLocation(programID, "colorize");
	GLuint thicknessID = glGetUniformLocation(programID, "thickness");
	GLuint minThicknessID = glGetUniformLocation(programID, "min_thickness");
	GLuint thinningID = glGetUniformLocation(programID, "thinning");
	GLuint decayID = glGetUniformLocation(programID, "decay");
	GLuint desaturationID = glGetUniformLocation(programID, "desaturation");

	//GLuint greenColourID = glGetUniformLocation(programID, "greenColour");

	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		//glUniform2f(windowID, 1024.0f, 768.0f);
		glUniform2f(windowID, 768.0f, 1024.0f);
		glUniform1ui(nID, 5);
		glUniform1f(thicknessID, 10.0f);
		glUniform1f(minThicknessID, 1.5f);
		glUniform1f(thinningID, 0.05f);
		glUniform1f(baseHueID, 0.0f);
		glUniform1ui(colorizeID, 1);
		glUniform1f(decayID, 0.3f);
		glUniform1f(desaturationID, 0.1f);

		//glUniform1f(greenColourID, 0.5f);

		// 1rst attribute buffer : vertices
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			4,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			4 * sizeof(float),  // stride
			(void*)0            // array buffer offset
		);

		glDisableVertexAttribArray(1);
		//glEnableVertexAttribArray(1);
		//glVertexAttribPointer(
		//	1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		//	3,                  // size
		//	GL_FLOAT,           // type
		//	GL_FALSE,           // normalized?
		//	7 * sizeof(float),  // stride
		//	(void*)(4 * sizeof(float))            // array buffer offset
		//);

		// 2nd attribute buffer : colors
		//glEnableVertexAttribArray(1);
		//glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		//glVertexAttribPointer(
		//	1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		//	3,                                // size
		//	GL_FLOAT,                         // type
		//	GL_FALSE,                         // normalized?
		//	0,                                // stride
		//	(void*)0                          // array buffer offset
		//);

		// Draw the triangle !
		//glDrawArrays(GL_LINES, 0, 2*2);
		glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, 5);
		//glDrawArrays(GL_TRIANGLES, 0, 2*3); // 3 indices starting at 0 -> 1 triangle
		//glDrawArrays(GL_POINTS, 0, 4);

		//glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}