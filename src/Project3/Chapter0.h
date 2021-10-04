#pragma once

class Chapter0 {
public:
	virtual void start() {}
	virtual void update(double) {}
	virtual void end() {}
	virtual void callback(GLFWwindow* window, int width, int height) {}
};

class Chapter1 : Chapter0 {
public:
	void start();
	void update(double);
	void end();
	void callback(GLFWwindow* window, int width, int height);
};

class Chapter2 : Chapter0 {
public:
	void start();
	void update(double);
	void end();
	void callback(GLFWwindow* window, int width, int height);
};