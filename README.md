# CG4Games

New and Improved Sandbox!

Now by default implements advanced features and includes saving and loading of shaders and model Matrix
(also includes the glfw3.lib as well as the QR code generating tool!)
adds glm for math functionality and improved framework for working with multiple models (multiple VAO method)

Created by Eric Ameres for my Computer Graphics for Games Students May 2021
Simple OpenGL applpication for sandboxing Graphics Algorithms

I created this file as a framework for experimenting with shaders and testing primitive graphics algorithms within OpenGL

To work with pixel by pixel algorithms, a small texture is used that is written into directly, but then used as a texture

using IMGUI the program also allows for "hot swapping" the basic vertex and fragment shaders

building requires VS 2019 (likely works with earlier versions as well) and CMAKE https://cmake.org/

Find the build directory and run the setup batch file.  This will create a g4gBuild directory.  
Inside that directory you will find a VS2019 solution file named "g4gP1.sln". 
Open that with Visual Studio (Community Edition)
