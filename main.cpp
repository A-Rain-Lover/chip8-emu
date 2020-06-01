/*
	A Chip8 emulator made by A-rain-lover (Yuun)
*/
#include <iostream>
#include <fstream>

#include <string>
#include <map>

#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"

#include "src/c8.h"

using namespace chip8;

#define HEIGHT	200
#define WIDTH	400

/* To normalized device coordinates */
glm::vec2 coords(float x, float y) {
	float newX = (x / WIDTH * 2) - 1;
	float newY = -(y / HEIGHT * 2) + 1;
	return glm::vec2(newX, newY);
}
glm::vec2 dimens(float w, float h) {
	float newW = (w / WIDTH * 2);
	float newH = (h / HEIGHT * 2);
	return glm::vec2(newW, newH);
}
#define c(a,b) coords(a,b).x,coords(a,b).y
#define d(a,b) dimens(a,b).x,dimens(a,b).y


Chip8 c;
std::map<int, short> keyBindings;

void keyDown(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/* Update the keys register */
	c.setKey(keyBindings[key], action == GLFW_PRESS);
}

extern void color(Color);
int main(int argc, char** argv) {
	if (argc <= 1) {
		color(ERR);
		std::cout << "[ ERROR ] No ROM specified ! ";
		color(NORMAL);
		return 1;
	}
	c = Chip8();

	/* Usage :
		chip8 rom_file [--log] [--log-regs]
	*/
	c.m_log = argc > 2 && !strcmp(argv[2], "--log");
	c.m_log_registers = argc > 3 && !strcmp(argv[3], "--log-regs");

	// TODO: add -h, --help argument

	/* Key bindings */
	keyBindings[GLFW_KEY_5] = 0x1;
	keyBindings[GLFW_KEY_6] = 0x2;
	keyBindings[GLFW_KEY_7] = 0x3;
	keyBindings[GLFW_KEY_8] = 0xC;
	
	keyBindings[GLFW_KEY_T] = 0x4;
	keyBindings[GLFW_KEY_Y] = 0x5;
	keyBindings[GLFW_KEY_U] = 0x6;
	keyBindings[GLFW_KEY_I] = 0xD;

	keyBindings[GLFW_KEY_G] = 0x7;
	keyBindings[GLFW_KEY_H] = 0x8;
	keyBindings[GLFW_KEY_J] = 0x9;
	keyBindings[GLFW_KEY_K] = 0xE;

	keyBindings[GLFW_KEY_B] = 0xA;
	keyBindings[GLFW_KEY_N] = 0x0;
	keyBindings[GLFW_KEY_M] = 0xB;
	keyBindings[GLFW_KEY_PERIOD] = 0xF;
	// TODO: delete that and make a keybindings config file


	c.load_ROM(argv[1]);

	if (!glfwInit()) {
		color(ERR);
		std::cout << "[ ERROR ] Couldn't initialize GLFW !";
		color(NORMAL);
		return 1;
	}

	if(!glewInit()) {
		color(ERR);
		std::cout << "[ ERROR ] Couldn't initialize GLEW !";
		color(NORMAL);
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Chip8 emulator",0,0);
	if (!window) {
		color(ERR);
		std::cout << "[ ERROR ] Couldn't create window !";
		color(NORMAL);
		return 1;
	}

	glfwMakeContextCurrent(window);
	glClearColor(0.05, 0.0, 0.1, 1.0);
	glfwSetKeyCallback(window, keyDown);

	short frames = 0;
	while (!glfwWindowShouldClose(window)) {
		if (frames++ % 20)
			continue;

		c.cycle();

		if (c.shouldDraw()) {
			glClear(GL_COLOR_BUFFER_BIT);
			glBegin(GL_QUADS);
			for (short i = 0; i < 32 * 64; i++) {
				if (!c.getScreen()[i]) continue;
				float x = i % 64;
				float y = i / 64;
				float w = WIDTH / 64;
				float h = HEIGHT / 32;

				glVertex2f(c(x * w, y * h));
				glVertex2f(c(x * w + w, y * h));
				glVertex2f(c(x * w + w, y * h + h));
				glVertex2f(c(x * w, y * h + h));
			}
			glEnd();

		}
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}
	color(NORMAL);
	return 0;
}
