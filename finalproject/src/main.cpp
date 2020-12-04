/**
 * Base code for particle systems lab
 */

#include <iostream>
#include <algorithm>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "WindowManager.h"
#include "particleSys.h"
#include "stb_image.h" 

 // value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

unsigned int sky;

class Application : public EventCallbacks
{

	public:

	WindowManager *windowManager = nullptr;

	// Our shader program for particles
	std::shared_ptr<Program> partProg;

	// Our shader program for meshes
	std::shared_ptr<Program> meshProg;

	// Our texture program for meshes
	std::shared_ptr<Program> progTex;

	// Our skybox program for meshes
	std::shared_ptr<Program> progSky;

	//the partricle system
	particleSys *thePartSystem;
	particleSys* thePartSystem2;
	particleSys* thePartSystem3;
	particleSys* thePartSystem4;
	particleSys* thePartSystem5;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> shape;

	// Obstacle to be used (from  file).
	shared_ptr<Shape> obstacle;

	// buildings to be used (from  file).
	shared_ptr<Shape> bld1;
	shared_ptr<Shape> bld2;
	shared_ptr<Shape> bld3;
	shared_ptr<Shape> bld4;
	shared_ptr<Shape> bld5;

	// buildings lists to be used.
	vector<shared_ptr<Shape>> bld1_list;
	vector<shared_ptr<Shape>> bld2_list;
	vector<shared_ptr<Shape>> bld3_list;
	vector<shared_ptr<Shape>> bld4_list;
	vector<shared_ptr<Shape>> bld5_list;

	// Shape list to be used.
	vector<shared_ptr<Shape>> shape_list;

	// OpenGL handle to texture data used in particle
	shared_ptr<Texture> texture;
	shared_ptr<Texture> texture4;

	bool keyToggles[256] = { false };

	//some particle variables
	float t = 0.0f; //reset in init
	float h = 0.01f;

	glm::vec3 g = glm::vec3(0.0f, -0.01f, 0.0f);

	vec3 carpos = vec3(0, 0, 0);
	float carRot = 0;
	float accel = 0;
	bool driving = false;

	double cursor_x, cursor_y;
	float x_1, y_1, z_1;
	float phi, theta = 0;
	vec3 eye = vec3(0, 20, 0);
	vec3 center;
	vec3 up = vec3(0, 1, 0);
	vec3 forward;
	mat4x4 lookAt_value;

	//collison boxes.
	float obs1[6];
	float obs2[6];
	float obs3[6];
	float obs4[6];

	float obs5[6];
	float obs6[6];
	float obs7[6];
	float obs8[6];

	bool fire_hit1 = false;
	bool fire_hit2 = false;
	bool fire_hit3 = false;
	bool fire_hit4 = false;

	bool reward_status = false;
	int reward_once = 0;

	int obs_idx_mat1 = 4;
	int obs_idx_mat2 = 4;
	int obs_idx_mat3 = 4;
	int obs_idx_mat4 = 4;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		keyToggles[key] = ! keyToggles[key];

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (key == GLFW_KEY_A)
		{
			vec3 rightVec = cross(up, forward);
			eye += rightVec * 0.1f;
			center += rightVec * 0.1f;
		}
		else if (key == GLFW_KEY_D)
		{
			vec3 rightVec = cross(forward, up);
			eye += rightVec * 0.1f;
			center += rightVec * 0.1f;
		}
		else if (key == GLFW_KEY_W)
		{
			eye += forward * 0.1f;
			center += forward * 0.1f;
		}
		else if (key == GLFW_KEY_S)
		{
			eye -= forward * 0.1f;
			center -= forward * 0.1f;
		}
		else if (key == GLFW_KEY_LEFT)
		{
			if (accel != 0)
			{
				carRot += 0.1f;
			}

		}
		else if (key == GLFW_KEY_RIGHT)
		{
			if (accel != 0)
			{
				carRot -= 0.1f;
			}

		}
		else if (key == GLFW_KEY_UP)
		{
			accel -= 0.01;
			driving = true;
		}
		else if (key == GLFW_KEY_DOWN)
		{
			accel += 0.01;
			driving = true;
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		CHECKED_GL_CALL(glViewport(0, 0, width, height));
	}

	//code to set up the two shaders - a diffuse shader and texture mapping
	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		CHECKED_GL_CALL(glClearColor(.12f, .34f, .56f, 1.0f));

		// Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(24.0f));


		// Initialize the GLSL program.
		partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(
			resourceDirectory + "/lab10_vert.glsl",
			resourceDirectory + "/lab10_frag.glsl");
		if (! partProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");
		partProg->addAttribute("pColor");


		// Initialize the GLSL program.
		meshProg = make_shared<Program>();
		meshProg->setVerbose(true);
		meshProg->setShaderNames(
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (! meshProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		meshProg->addUniform("P");
		meshProg->addUniform("M");
		meshProg->addUniform("V");
		meshProg->addAttribute("vertPos");
		meshProg->addAttribute("vertNor");
		meshProg->addAttribute("vertTex");

		meshProg->addUniform("MatAmb");
		meshProg->addUniform("MatDif");
		meshProg->addUniform("MatSpec");
		meshProg->addUniform("shine");
		meshProg->addUniform("light_source");
		meshProg->addUniform("eye");


		// Initialize the GLSL program.
		progTex = make_shared<Program>();
		progTex->setVerbose(true);
		progTex->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		progTex->init();
		progTex->addUniform("P");
		progTex->addUniform("V");
		progTex->addUniform("M");
		progTex->addAttribute("vertPos");
		progTex->addAttribute("vertNor");
		progTex->addAttribute("vertTex");

		//material properties.
		progTex->addUniform("MatAmb");
		progTex->addUniform("MatDif");
		progTex->addUniform("MatSpec");
		progTex->addUniform("shine");

		//lighting.
		progTex->addUniform("light_source");

		//texturing.
		progTex->addUniform("Texture0");
		progTex->addUniform("Invert");

		//camera.
		progTex->addUniform("eye");


		// Initialize the GLSL program.
		progSky = make_shared<Program>();
		progSky->setVerbose(true);
		progSky->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		progSky->init();
		progSky->addUniform("P");
		progSky->addUniform("V");
		progSky->addUniform("M");
		progSky->addAttribute("vertPos");
		progSky->addAttribute("vertNor");

		//skybox.
		progSky->addUniform("skybox");


		//initialize particle system.
		thePartSystem = new particleSys(vec3(0, 0, 0));
		thePartSystem->gpuSetup();

		thePartSystem2 = new particleSys(vec3(-10, 0, 10));
		thePartSystem2->gpuSetup();

		thePartSystem3 = new particleSys(vec3(10, 0, 10));
		thePartSystem3->gpuSetup();

		thePartSystem4 = new particleSys(vec3(10, 0, -10));
		thePartSystem4->gpuSetup();

		thePartSystem5 = new particleSys(vec3(-10, 0, -10));
		thePartSystem5->gpuSetup();
	}

	void initGeom(const std::string& resourceDirectory){

		//EXAMPLE new set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/AE86.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				shape = make_shared<Shape>();
				shape->createShape(TOshapes[i]);
				shape->measure();
				shape->init();

				shape_list.push_back(shape);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			obstacle = make_shared<Shape>();
			obstacle->createShape(TOshapes[0]);
			obstacle->measure();
			obstacle->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/house.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				bld1 = make_shared<Shape>();
				bld1->createShape(TOshapes[i]);
				bld1->measure();
				bld1->init();

				bld1_list.push_back(bld1);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/bld1.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				bld2 = make_shared<Shape>();
				bld2->createShape(TOshapes[i]);
				bld2->measure();
				bld2->init();

				bld2_list.push_back(bld2);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/bld3.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				bld3 = make_shared<Shape>();
				bld3->createShape(TOshapes[i]);
				bld3->measure();
				bld3->init();

				bld3_list.push_back(bld3);
			}
		}
	}

	// Code to load in the texture
	void initTex(const std::string& resourceDirectory)
	{
		texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + "/alpha.bmp");
		texture->init();
		texture->setUnit(0);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture4 = make_shared<Texture>();
		texture4->setFilename(resourceDirectory + "/moon.jpg");
		texture4->init();
		texture4->setUnit(2);
		texture4->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void SetPhongUniforms(int i, std::shared_ptr<Program> prog) {
		switch (i) {
		case 0: //midnight-blue.
			glUniform3f(prog->getUniform("MatAmb"), 0.1334, 0.3113, 0.34);
			glUniform3f(prog->getUniform("MatDif"), 0.234, 0.435, 0.6786);
			glUniform3f(prog->getUniform("MatSpec"), 0.3243, 0.1233, 0.2324);
			glUniform1f(prog->getUniform("shine"), 10.345);

			glUniform3f(prog->getUniform("eye"), 0.0, 0.0, 0.0);
			glUniform3f(prog->getUniform("light_source"), 2.0, 2.0, 2.0);
			break;
		case 1: //pale-yellow.
			glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
			glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
			glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
			glUniform1f(prog->getUniform("shine"), 27.9);

			glUniform3f(prog->getUniform("eye"), 0.0, 0.0, 0.0);
			glUniform3f(prog->getUniform("light_source"), 2.0, 2.0, 2.0);
			break;
		case 2: //glaze-light-red.
			glUniform3f(prog->getUniform("MatAmb"), 0.6294, 0.2235, 0.2745);
			glUniform3f(prog->getUniform("MatDif"), 0.1123, 0.432, 0.34232);
			glUniform3f(prog->getUniform("MatSpec"), 0.238, 0.678, 0.2348);
			glUniform1f(prog->getUniform("shine"), 34.7234);

			glUniform3f(prog->getUniform("eye"), 0.0, 0.0, 0.0);
			glUniform3f(prog->getUniform("light_source"), 2.0, 2.0, 2.0);
			break;
		case 3: //metal.
			glUniform3f(prog->getUniform("MatAmb"), 0.03, 0.03, 0.03);
			glUniform3f(prog->getUniform("MatDif"), 0.16, 0.16, 0.16);
			glUniform3f(prog->getUniform("MatSpec"), 0.8, 0.8, 0.8);
			glUniform1f(prog->getUniform("shine"), 100);

			glUniform3f(prog->getUniform("eye"), 0.0, 0.0, 0.0);
			glUniform3f(prog->getUniform("light_source"), 2.0, 2.0, 2.0);
			break;
		case 4: //hot-pink-purple.
			glUniform3f(prog->getUniform("MatAmb"), 0.013, 0.103, 0.203);
			glUniform3f(prog->getUniform("MatDif"), 0.53416, 0.16, 0.33216);
			glUniform3f(prog->getUniform("MatSpec"), 0.7, 0.5628, 0.3248);
			glUniform1f(prog->getUniform("shine"), 23.32);

			glUniform3f(prog->getUniform("eye"), 0.0, 0.0, 0.0);
			glUniform3f(prog->getUniform("light_source"), 2.0, 2.0, 2.0);
			break;
		}
	}

	float getRand()
	{
		return ((double)rand() / (RAND_MAX));
	}

	bool collision(float A[], float B[])
	{
		bool hit = (
			A[3] > B[0] &&
			A[0] < B[3] &&
			A[4] > B[1] &&
			A[1] < B[4] &&
			A[5] > B[2] &&
			A[2] < B[5]);
		return hit;
	}

	unsigned int createSky(string dir, vector<string> faces)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for (GLuint i = 0; i < faces.size(); i++) {
			unsigned char* data =
				stbi_load((dir + faces[i]).c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else {
				cout << "failed to load: " << (dir + faces[i]).c_str() << endl;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cout << "creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}

	void render(GLFWwindow* window)
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width / (float)height;

		// Create the matrix stacks
		auto P = make_shared<MatrixStack>();
		auto V = make_shared<MatrixStack>();
		auto M = make_shared<MatrixStack>();

		// Apply perspective projection.
		P->pushMatrix();
		P->perspective(45.0f, aspect, 0.01f, 100.0f);

		//==================================================================
		//CAMERA.
		glfwGetCursorPos(window, &cursor_x, &cursor_y);
		glfwSetCursorPos(window, width / 2, height / 2);
		ShowCursor(false);
		float radius = 6;

		if ((cursor_x >= 0 && cursor_x <= width) && (cursor_y >= 0 && cursor_y <= height))
		{
			phi += (height / 2 - cursor_y) * 0.001;
			theta -= (width / 2 - cursor_x) * 0.001;
		}

		if (phi > 1.5) { phi = 1.5; }
		if (phi < -1.5) { phi = -1.5; }

		x_1 = radius * cos(phi) * cos(theta);
		y_1 = radius * sin(phi);
		z_1 = radius * cos(phi) * cos((3.14 / 2.0) - theta);

		forward = center - eye;
		center = eye + vec3(x_1, y_1, z_1);
		lookAt_value = lookAt(eye, center, up);
		//==================================================================
		// Basic Car physics.
		//move base on car direction.
		carpos.z += accel * cos(carRot);
		carpos.x += accel * sin(carRot);
		//basic acceleration principles.
		if (accel > 0) { accel -= 0.001f; }
		if (accel < 0) { accel += 0.001f; }
		if (accel <= 0.008f && accel >= -0.008f)
		{ 
			accel = 0;
			driving = false;
		}
		//==================================================================
		// Basic Bounding Box collision detection.
		float car_obj[6] =
		{
			shape_list[9]->min.x + carpos.x,
			shape_list[9]->min.y + carpos.y,
			shape_list[9]->min.z + carpos.z,
			shape_list[9]->max.x + carpos.x,
			shape_list[9]->max.y + carpos.y,
			shape_list[9]->max.z + carpos.z
		};

		//object translated at (0,0,-20), with transforms (40,2,4).
		float obs_1[6] =
		{
			obs1[0] = (obstacle->min.x * 40),
			obs1[1] = (obstacle->min.y * 2),
			obs1[2] = (obstacle->min.z * 4) - 20,
			obs1[3] = (obstacle->max.x * 40),
			obs1[4] = (obstacle->max.y * 2),
			obs1[5] = (obstacle->max.z * 4) - 20
		};
		//object translated at (0,0, 20), with transforms (40,2,4).
		float obs_2[6] =
		{
			obs2[0] = (obstacle->min.x * 40),
			obs2[1] = (obstacle->min.y * 2),
			obs2[2] = (obstacle->min.z * 4) + 20,
			obs2[3] = (obstacle->max.x * 40),
			obs2[4] = (obstacle->max.y * 2),
			obs2[5] = (obstacle->max.z * 4) + 20
		};
		//object translated at (-20,0, 0), with transforms (4,2,40).
		float obs_3[6] =
		{
			obs3[0] = (obstacle->min.x * 4) - 20,
			obs3[1] = (obstacle->min.y * 2),
			obs3[2] = (obstacle->min.z * 40),
			obs3[3] = (obstacle->max.x * 4) - 20,
			obs3[4] = (obstacle->max.y * 2),
			obs3[5] = (obstacle->max.z * 40)
		};
		//object translated at (20,0, 0), with transforms (4,2,40).
		float obs_4[6] =
		{
			obs4[0] = (obstacle->min.x * 4) + 20,
			obs4[1] = (obstacle->min.y * 2),
			obs4[2] = (obstacle->min.z * 40),
			obs4[3] = (obstacle->max.x * 4) + 20,
			obs4[4] = (obstacle->max.y * 2),
			obs4[5] = (obstacle->max.z * 40)
		};

		//Firework boxes!
		float obs_5[6] =
		{
			obs5[0] = (obstacle->min.x * 4) - 10,
			obs5[1] = (obstacle->min.y * 8),
			obs5[2] = (obstacle->min.z * 4) + 10,
			obs5[3] = (obstacle->max.x * 4) - 10,
			obs5[4] = (obstacle->max.y * 8),
			obs5[5] = (obstacle->max.z * 4) + 10
		};

		float obs_6[6] =
		{
			obs5[0] = (obstacle->min.x * 4) + 10,
			obs5[1] = (obstacle->min.y * 8),
			obs5[2] = (obstacle->min.z * 4) + 10,
			obs5[3] = (obstacle->max.x * 4) + 10,
			obs5[4] = (obstacle->max.y * 8),
			obs5[5] = (obstacle->max.z * 4) + 10
		};

		float obs_7[6] =
		{
			obs5[0] = (obstacle->min.x * 8) + 10,
			obs5[1] = (obstacle->min.y * 4),
			obs5[2] = (obstacle->min.z * 4) - 10,
			obs5[3] = (obstacle->max.x * 8) + 10,
			obs5[4] = (obstacle->max.y * 4),
			obs5[5] = (obstacle->max.z * 4) - 10
		};

		float obs_8[6] =
		{
			obs5[0] = (obstacle->min.x * 4) - 10,
			obs5[1] = (obstacle->min.y * 4),
			obs5[2] = (obstacle->min.z * 4) - 10,
			obs5[3] = (obstacle->max.x * 4) - 10,
			obs5[4] = (obstacle->max.y * 4),
			obs5[5] = (obstacle->max.z * 4) - 10
		};
		//==================================================================
		if (fire_hit1 == true && fire_hit2 == true && fire_hit3 == true && fire_hit4 == true)
		{
			reward_once += 1;
			if (reward_once == 1) { cout << "You are a terrible driver!" << endl; }
			reward_status = true;
		}
		//==================================================================
		// Meshes.
		progTex->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		// draw mesh 
		M->pushMatrix();
			M->loadIdentity();
			M->translate(carpos);

			M->pushMatrix();
				M->scale(vec3(0.5, 0.5, 0.5));
				M->rotate(carRot + 1.56, vec3(0, 1, 0));
				glUniformMatrix4fv(progTex->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));

				SetPhongUniforms(3, progTex);
				glUniform1f(progTex->getUniform("Invert"), 1);
				glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
				glUniform3f(progTex->getUniform("light_source"), 2.0, 2.0, 2.0);
				texture4->bind(progTex->getUniform("Texture0"));

				for (int i = 0; i < shape_list.size(); i++) { shape_list[i]->draw(progTex); }
			M->popMatrix();

		M->popMatrix();
		progTex->unbind();
		//==================================================================
		// Particles 1 / Car smoke.
		partProg->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		M->pushMatrix();
			M->loadIdentity();
			M->translate(vec3(carpos.x, carpos.y-1, carpos.z + 3));
			M->scale(vec3(0.5, 0.5, 0.5));

			M->pushMatrix();
				M->rotate(carRot + 1.56, vec3(0, 1, 0));
				thePartSystem->setCamera(lookAt_value);
				texture->bind(partProg->getUniform("alphaTexture"));
				glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
				if (driving == true) { thePartSystem->drawMe(partProg); }
				thePartSystem->update();
			M->popMatrix();

		M->popMatrix();
		partProg->unbind();
		//==================================================================
		// Wall 1 - left.
		meshProg->bind();
		M->pushMatrix();
			M->loadIdentity();
			M->translate(vec3(0, 0, -20));
			M->scale(vec3(40, 2, 4));
			CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
			CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
			glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			SetPhongUniforms(3, meshProg);
			if (collision(car_obj, obs_1)) { accel = -1 * accel; }
			obstacle->draw(meshProg);
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// Wall 2 - right.
		meshProg->bind();
		M->pushMatrix();
			M->loadIdentity();
			M->translate(vec3(0, 0, 20));
			M->scale(vec3(40, 2, 4));
			CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
			CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
			glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			SetPhongUniforms(3, meshProg);
			if (collision(car_obj, obs_2)) { accel = -1 * accel; }
			obstacle->draw(meshProg);
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// Wall 3 - top.
		meshProg->bind();
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(-20, 0, 0));
		M->scale(vec3(4, 2, 40));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		SetPhongUniforms(3, meshProg);
		if (collision(car_obj, obs_3)) { accel = -1 * accel; }
		obstacle->draw(meshProg);
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// Wall 4 - bottom.
		meshProg->bind();
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(20, 0, 0));
		M->scale(vec3(4, 2, 40));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		SetPhongUniforms(3, meshProg);
		if (collision(car_obj, obs_4)) { accel = -1 * accel; }
		obstacle->draw(meshProg);
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// Ground.
		progTex->bind();
		M->pushMatrix();
			M->loadIdentity();
			M->scale(vec3(40, 1, 40));
			M->translate(vec3(0, -0.5, 0));
			CHECKED_GL_CALL(glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
			CHECKED_GL_CALL(glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
			glUniform1f(progTex->getUniform("Invert"), 1);
			glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
			glUniform3f(progTex->getUniform("light_source"), 2.0, 2.0, 2.0);
			glUniformMatrix4fv(progTex->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
			SetPhongUniforms(4, progTex);
			texture4->bind(progTex->getUniform("Texture0"));
			obstacle->draw(progTex);
		M->popMatrix();
		progTex->unbind();
		//==================================================================
		// obstacle 1 - top right.
		meshProg->bind();
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(-10, 0, 10));
		M->scale(vec3(4, 4, 4));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		SetPhongUniforms(obs_idx_mat1, meshProg);
		if (collision(car_obj, obs_5))
		{
			accel = -1 * accel * 1.1;
			obs_idx_mat1 = 2;
			fire_hit1 = true;
		}
		bld1_list[0]->draw(meshProg);
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// obstacle 2 - top left.
		meshProg->bind();
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(10, 0, 10));
		M->scale(vec3(4, 8, 4));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		SetPhongUniforms(obs_idx_mat2, meshProg);
		if (collision(car_obj, obs_6))
		{
			accel = -1 * accel * 1.1;
			obs_idx_mat2 = 2;
			fire_hit2 = true;
		}
		bld1_list[0]->draw(meshProg);
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// obstacle 3 - bottom left.
		meshProg->bind();
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(10, 0, -10));
		M->scale(vec3(0.5, 0.5, 0.5));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		SetPhongUniforms(obs_idx_mat3, meshProg);
		if (collision(car_obj, obs_7))
		{
			accel = -1 * accel * 1.1;
			obs_idx_mat3 = 2;
			fire_hit3 = true;
		}
		for (int i = 0; i < bld2_list.size(); i++) { bld2_list[i]->draw(meshProg); }
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// obstacle 4 - bottom left.
		meshProg->bind();
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(-10, 4.5, -10));
		M->scale(vec3(0.005, 0.005, 0.005));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(meshProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		glUniformMatrix4fv(meshProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		SetPhongUniforms(obs_idx_mat4, meshProg);
		if (collision(car_obj, obs_8))
		{
			accel = -1 * accel * 1.1;
			obs_idx_mat4 = 2;
			fire_hit4 = true;
		}
		for (int i = 0; i < bld3_list.size(); i++) { bld3_list[i]->draw(meshProg); }
		M->popMatrix();
		meshProg->unbind();
		//==================================================================
		// Particles obstacle 1 - top right.
		partProg->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(-10, 2, 13));

		M->pushMatrix();
		thePartSystem2->setCamera(lookAt_value);
		texture->bind(partProg->getUniform("alphaTexture"));
		glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		if (fire_hit1) { thePartSystem2->drawMe(partProg); }
		thePartSystem2->update();
		M->popMatrix();

		M->popMatrix();
		partProg->unbind();
		//==================================================================
		// Particles obstacle 2 - top left.
		partProg->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(10, 6, 13));

		M->pushMatrix();
		thePartSystem3->setCamera(lookAt_value);
		texture->bind(partProg->getUniform("alphaTexture"));
		glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		if (fire_hit2) { thePartSystem3->drawMe(partProg); }
		thePartSystem3->update();
		M->popMatrix();

		M->popMatrix();
		partProg->unbind();
		//==================================================================
		// Particles obstacle 3 - bottom left.
		partProg->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(10, 30, -5));

		M->pushMatrix();
		thePartSystem4->setCamera(lookAt_value);
		texture->bind(partProg->getUniform("alphaTexture"));
		glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		if (fire_hit3) { thePartSystem4->drawMe(partProg); }
		thePartSystem4->update();
		M->popMatrix();

		M->popMatrix();
		partProg->unbind();
		//==================================================================
		// Particles obstacle 4 - bottom right.
		partProg->bind();
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value)));
		M->pushMatrix();
		M->loadIdentity();
		M->translate(vec3(-10, 10, -10));

		M->pushMatrix();
		thePartSystem5->setCamera(lookAt_value);
		texture->bind(partProg->getUniform("alphaTexture"));
		glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		if (fire_hit4) { thePartSystem5->drawMe(partProg); }
		thePartSystem5->update();
		M->popMatrix();

		M->popMatrix();
		partProg->unbind();
		//==================================================================
		//SKYBOX.
		auto Model2 = make_shared<MatrixStack>();
		//to draw the sky box bind the right shader
		progSky->bind();
		glUniformMatrix4fv(progSky->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
		glDepthFunc(GL_LEQUAL);
		glUniformMatrix4fv(progSky->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

		Model2->pushMatrix();
		Model2->loadIdentity();
		Model2->scale(vec3(100, 100, 100));

		glUniformMatrix4fv(progSky->getUniform("M"), 1, GL_FALSE, value_ptr(Model2->topMatrix()));
		glBindTexture(GL_TEXTURE_CUBE_MAP, sky);
		obstacle->draw(progSky);
		glDepthFunc(GL_LESS);

		Model2->popMatrix();

		progSky->unbind();
		//==================================================================

		P->popMatrix();
	}

};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(1000, 500);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initGeom(resourceDir);

	vector<std::string> space_sky
	{
		"corona_rt.png",
		"corona_lf.png",
		"corona_up.png",
		"corona_dn.png",
		"corona_ft.png",
		"corona_bk.png"
	};
	//sky = application->createSky("../resources/cracks/", faces);
	sky = application->createSky("../resources/skybox/", space_sky);
	cout << "createsky: " << sky << endl;

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render(windowManager->getHandle());

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
