#pragma once

class Chapter0 {
public:
	const char* name = "Graphics for Games - Chapter 0";
	virtual void start() {}
	virtual void update(double) {}
	virtual void end() {}
	virtual void callback(GLFWwindow* window, int width, int height) {}
};

class Chapter1 : Chapter0 {
public:
	Chapter1() { name = "G4G:Chapter 1"; }
	void start();
	void update(double);
	void end();
	void callback(GLFWwindow* window, int width, int height);
}; 

class Chapter1a : Chapter0 { // cubeQuad
public:
	Chapter1a() { name = "G4G:Chapter 1a - Cube of Quads"; }
	void start();
	void update(double);
	void end();
	void callback(GLFWwindow* window, int width, int height);
};

class Chapter2 : Chapter0 {
public:
	Chapter2() { name = "G4G:Chapter 2 - Everything!"; }
	void start();
	void update(double);
	void end();
	void callback(GLFWwindow* window, int width, int height);
};