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

This example now runs just as well on MacOS as it does on windows (On 9/7/21 at least!)

On Windows, building requires VS 2019 (likely works with earlier versions as well) and CMAKE https://cmake.org/
On Mac (Only verified on Catalina 10.15) requires XCode 12.4 (IDE and Command Line Tools) and CMAKE https://cmake.org/

On Windows:
Find the build directory and run the setup batch file.  This will create a g4gBuild directory.  
Inside that directory you will find a VS2019 solution file named "g4gP1.sln". 
Open that with Visual Studio (Community Edition)

On MacOS:
Find the build directory and run the command file (a script). This will create a g4gBuild directory.
Inside that directory you will find an XCode project named g4gp1, open that in XCode 12.4

It has been reported that the sandbox works in Big Sur with newer XCode, but I don't know the specific versions
