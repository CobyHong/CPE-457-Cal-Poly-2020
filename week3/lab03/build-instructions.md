

Orientation
===============
CPE471 - Getting Started

*Please note many systems can have slight variations - we've tried to explain and provide pointers but you will likely need to do some web searching as you encounter errors.  We will do our best to support you.*

Introduction
Alongside CMake, there are two basic tools we need to set up in order to get started with OGL on any platform. However setup of these tools will vary by operating system. 

GLFW: A cross-platform window management system intended for use with graphics projects. Provides a unified way to interact with and draw on windows, and get input from the keyboard, mouse, or gamepads. 
GLM: The OpenGL math library provides many useful tools for doing math for computer graphics. It is modelled after GLSL to make syntax and operations more convenient and familiar. 

Cross-Platform Steps:

1. Create a Directory for Graphics Libraries
We recommend that you decide on a common location to place the libraries we’re setting up in this document. It can be wherever you like, so we’ll refer to it simply as ‘your graphics library directory’ moving forward. 

2. GLM Setup
GLM is a header only library, which fortunately makes setup convenient.
Step 1:
Go to the GLM release page on Github and download the zip file for the latest release. At the time of writing, the latest release is 0.9.9.7
https://github.com/g-truc/glm/releases  You can also use a package manager to install if desired.
Step 2: 
Extract the contents of the ‘glm’ directory out of the zip file and to your library directory. 
Step 3: 
Create an environment variable ‘GLM_INCLUDE_DIR’ and point it to the newly extracted glm directory (The directory which contains CMakeLists.txt) 


3. GLFW setup
Step 1: 
Visit https://github.com/glfw/glfw/releases and download the latest version as a zip file. This should be done through the link glfw-3.x.x.zip, not the WIN32, WIN64, or MACOS links. You can also use a package manager to install if desired.
Step 2: 
Extract the zip contents into your graphics library directory. 
Step 3: 
Create environment variable GLFW_DIR and point it to this new directory (the directory should contain CMakeLists.txt). 




Installing prerequisite software described for each system
=======================================================


Lab machines (Linux)
--------------------

The compiler, CMake, GLM, and GLFW3 should already be installed.
Skip to "Building and Running the Lab/Assignment" below.


Mac OS X
--------

You can use homebrew/macports or install these manually.

- Xcode developer tools. You'll need to log in with your Apple ID.
- CMake (<http://cmake.org/download/>)
- [GLM](http://brewformulas.org/Glm)
- [GLFW3](http://brewformulas.org/glfw)

Make sure the commands `g++` and `cmake` work from the command prompt.


Windows
-------

First, download **Visual Studio Community 2017 or you can use VS Code - just make sure you install a C++ compiler**.
Make sure to install the C++ development tools.

Download these:

- CMake (<http://cmake.org/download/>). Make sure to add CMake to the system
  path when asked to do so.
- [vcpkg](https://github.com/Microsoft/vcpkg)

Make sure the command `cmake` works from the command prompt.

Now use `vcpkg` to install `glm` and `glfw3`


Ubuntu Linux
------------
(These have not been updated recently - proceed with caution)
You'll need the following if you don't have them already.

	> sudo apt-get update
	> sudo apt-get install g++
	> sudo apt-get install cmake
	> sudo apt-get install libglfw3-dev
	> sudo apt-get install libglfw3-dev

You may or may not need these:

	> sudo apt-get install freeglut3-dev
	> sudo apt-get install libxrandr-dev
	> sudo apt-get install libxinerama-dev
	> sudo apt-get install libxcursor-dev

Building and Running the Lab/Assignment
=======================================

All platforms (except Lab windows)
----------------------------------

We'll perform an "out-of- source" build, which means that the binary files
will not be in the same directory as the source files. In the folder that
contains CMakeLists.txt, run the following.

	> mkdir build
	> cd build

Then run one of the following *from the build folder*, depending on your
choice of platform and IDE.


OSX & Linux Makefile
--------------------

	> cmake ..

This will generate a Makefile that you can use to compile your code. To
compile the code, run the generated Makefile.

	> make -j4

The `-j` argument speeds up the compilation by multithreading the compiler.
This will generate an executable, which you can run by typing

	> ./Lab3 

To change the compiler, read [this
page](http://cmake.org/Wiki/CMake_FAQ#How_do_I_use_a_different_compiler.3F).
The best way is to use environment variables before calling cmake. For
example, to use the Intel C++ compiler:

	> which icpc # copy the path
	> CXX=/path/to/icpc cmake ..


OSX 
---------
You can use XCode or not.  If you want to use XCode type:

	> cmake -G Xcode ..

This will generate e.g. `Lab3.xcodeproj` project that you can open with Xcode.

- To run, change the target to `Lab03` by going to Product -> Scheme -> Lab03.
  Then click on the play button or press Command+R to run the application.
- Edit the scheme to add command-line arguments (`../../resources`) or to run
  in release mode.


Windows Visual Studio 2015,2017,2019
--------------------------

Or on your own machine, if you would prefer to use CMake.

Use [vcpkg](https://github.com/Microsoft/vcpkg) to install `glfw3` and `glm`. Follow instructions found under "Quick Start: Windows".

	> cmake ..

By default on Windows, CMake will generate a Visual Studio solution file, e.g. `Lab00.sln`, which you can open by double-clicking.
If you get a version mismatch, you may have to specify your visual studio version, for example:

	> cmake -G "Visual Studio 14 2015" ..

Other versions of Visual Studio are listed on the CMake page
(<http://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>).

- To build and run the project, right-click on e.g. `Lab0` in the project explorer
  and then click on "Set as Startup Project." Then press F7 (Build Solution)
  and then F5 (Start Debugging).
- To add a commandline argument, right-click on e.g. `Lab0` in
  the project explorer and then click on "Properties" and then click on
  "Debugging."

