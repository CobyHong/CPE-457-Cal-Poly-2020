/*
 * Lab 5 base code (could also be used for Program 2)
 * includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// for texturing
#include "Texture.h"

// for skybox
#include "stb_image.h" 

using namespace std;
using namespace glm;

//lighting movement.
float mov_light = 0;
vec3 light_pos = vec3(0, 200, 0);

//sky.
unsigned int sky;

//car animation.
float theta2 = 0;
float theta3 = 0;

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> progTex;
	std::shared_ptr<Program> progSky;

	// Shape to be used (from  file) - modify to support multiple
	vector<shared_ptr<Shape>> mesh_list;
	vector<shared_ptr<Shape>> mesh_list2;
	vector<shared_ptr<Shape>> mesh_list3;
	vector<shared_ptr<Shape>> mesh_list4;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;
	shared_ptr<Shape> mesh2;
	shared_ptr<Shape> mesh3;
	shared_ptr<Shape> mesh4;
	shared_ptr<Shape> cube_mesh;

	// Texture pointers similar to mesh.
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> texture3;
	shared_ptr<Texture> texture4;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//camera.
	vec3 eye = vec3(0, 20, 0);
	vec3 center;
	vec3 up = vec3(0, 1, 0);
	vec3 forward;
	mat4x4 lookAt_value;

	float moveSpeed = 0.1;
	float phi, theta = 0;
	double cursor_x, cursor_y;
	float x_1, y_1, z_1;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Q) {
			mov_light -= 0.5;
		}
		if (key == GLFW_KEY_E) {
			mov_light += 0.5;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		else if (key == GLFW_KEY_A)
		{
			vec3 rightVec = cross(up, forward);
			eye += rightVec * moveSpeed;
			center += rightVec * moveSpeed;
		}
		else if (key == GLFW_KEY_D)
		{
			vec3 rightVec = cross(forward, up);
			eye += rightVec * moveSpeed;
			center += rightVec * moveSpeed;
		}
		else if (key == GLFW_KEY_W)
		{
			eye += forward * moveSpeed;
			center += forward * moveSpeed;
		}
		else if (key == GLFW_KEY_S)
		{
			eye -= forward * moveSpeed;
			center -= forward * moveSpeed;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .54f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//==================================================================

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		//material properties.
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");

		//lighting.
		prog->addUniform("light_source");

		//camera.
		prog->addUniform("eye");

		//==================================================================

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

		//==================================================================

		// Initialize the GLSL program.
		progSky = make_shared<Program>();
		progSky->setVerbose(true);
		progSky->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		progSky->init();
		progSky->addUniform("P");
		progSky->addUniform("V");
		progSky->addUniform("M");
		progSky->addAttribute("vertPos");

		//skybox.
		progSky->addUniform("skybox");
	}

	void initTex(const std::string& resourceDirectory) {
		texture0 = make_shared<Texture>();
		texture0->setFilename(resourceDirectory + "/world.jpg");
		texture0->init();
		texture0->setUnit(0);
		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture1 = make_shared<Texture>();
		texture1->setFilename(resourceDirectory + "/water.jpg");
		texture1->init();
		texture1->setUnit(1);
		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture2 = make_shared<Texture>();
		texture2->setFilename(resourceDirectory + "/grass.jpg");
		texture2->init();
		texture2->setUnit(2);
		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture3 = make_shared<Texture>();
		texture3->setFilename(resourceDirectory + "/car.jpg");
		texture3->init();
		texture3->setUnit(2);
		texture3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture4 = make_shared<Texture>();
		texture4->setFilename(resourceDirectory + "/moon.jpg");
		texture4->init();
		texture4->setUnit(2);
		texture4->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void initGeom(const std::string& resourceDirectory)
	{
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;

 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/setting.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				mesh = make_shared<Shape>();
				mesh->createShape(TOshapes[i]);
				mesh->measure();
				mesh->init();

				mesh_list.push_back(mesh);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/AE86.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				mesh2 = make_shared<Shape>();
				mesh2->createShape(TOshapes[i]);
				mesh2->measure();
				mesh2->init();

				mesh_list2.push_back(mesh2);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphere.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				mesh3 = make_shared<Shape>();
				mesh3->createShape(TOshapes[i]);
				mesh3->measure();
				mesh3->init();

				mesh_list3.push_back(mesh3);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/S15.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				mesh4 = make_shared<Shape>();
				mesh4->createShape(TOshapes[i]);
				mesh4->measure();
				mesh4->init();

				mesh_list4.push_back(mesh4);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			cube_mesh = make_shared<Shape>();
			cube_mesh->createShape(TOshapes[0]);
			cube_mesh->measure();
			cube_mesh->init();
		}

	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M)
	{
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		glUniform3f(prog->getUniform("light_source"), light_pos.x + mov_light, light_pos.y, light_pos.z);
	}

	float clamp(float oldval, float oldmin, float oldmax, float newmin, float newmax)
	{ 
		float newvalue;
		float newrange = (newmax - newmin);
		float oldrange = (oldmax - oldmin);

		if (oldrange == 0)
		{ 
			newvalue = newmin;
		}
		else
		{
			newvalue = (((oldval - oldmin) * newrange) / oldrange) + newmin;
		}
		return newvalue;
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
		cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}

	void SetMaterial(int i, std::shared_ptr<Program> prog) {
		switch (i) {
		case 0: //midnight-blue.
			glUniform3f(prog->getUniform("MatAmb"), 0.1334, 0.3113, 0.34);
			glUniform3f(prog->getUniform("MatDif"), 0.234, 0.435, 0.6786);
			glUniform3f(prog->getUniform("MatSpec"), 0.3243, 0.1233, 0.2324);
			glUniform1f(prog->getUniform("shine"), 10.345);
			break;
		case 1: //pale-yellow.
			glUniform3f(prog->getUniform("MatAmb"), 0.423, 0.513, 0.314);
			glUniform3f(prog->getUniform("MatDif"), 0.23423, 0.534132, 0.4234);
			glUniform3f(prog->getUniform("MatSpec"), 0.5344, 0.3232, 0.3534);
			glUniform1f(prog->getUniform("shine"), 20.234);
			break;
		case 2: //glaze-light-red.
			glUniform3f(prog->getUniform("MatAmb"), 0.6294, 0.2235, 0.2745);
			glUniform3f(prog->getUniform("MatDif"), 0.1123, 0.432, 0.34232);
			glUniform3f(prog->getUniform("MatSpec"), 0.238, 0.678, 0.2348);
			glUniform1f(prog->getUniform("shine"), 34.7234);
			break;
		case 3: //metal.
			glUniform3f(prog->getUniform("MatAmb"), 0.03, 0.03, 0.03);
			glUniform3f(prog->getUniform("MatDif"), 0.16, 0.16, 0.16);
			glUniform3f(prog->getUniform("MatSpec"), 0.8, 0.8, 0.8);
			glUniform1f(prog->getUniform("shine"), 100);
			break;
		case 4: //hot-pink-purple.
			glUniform3f(prog->getUniform("MatAmb"), 0.013, 0.103, 0.203);
			glUniform3f(prog->getUniform("MatDif"), 0.53416, 0.16, 0.33216);
			glUniform3f(prog->getUniform("MatSpec"), 0.7, 0.5628, 0.3248);
			glUniform1f(prog->getUniform("shine"), 23.32);
			break;
		}
	}

	void render(GLFWwindow* window) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

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

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
			View->translate(vec3(0, 0, 0));

		//==================================================================
		progTex->bind();
			glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

			// draw mesh 
			Model->pushMatrix();
				float tx, tz;
				Model->loadIdentity();
				Model->translate(vec3(0, 0, 0));

				// AE86
				Model->pushMatrix();
					tx = (4.f) * sin(theta2);
					tz = (4.f) * cos(theta2);

					//Model->translate(vec3(10, -1, 0));
					Model->scale(vec3(1, 1, 1));
					Model->translate(vec3(tx, -1, tz));
					Model->rotate(3.14f + theta2, vec3(0, 1, 0));
					theta2 -= 0.05;

					setModel(progTex, Model);
					SetMaterial(3, progTex);
					glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
					glUniform1f(progTex->getUniform("Invert"), 1);
					for (int i = 0; i < mesh_list2.size(); i++)
					{
						if (i == 1 ||
						i == 2 ||
						i == 3 ||
						i == 4 ||
						i == 5 ||
						i == 6 ||
						i == 7 ||
						i == 8)
						{
							//SetMaterial(4);
							texture2->bind(progTex->getUniform("Texture0"));
							mesh_list2[i]->draw(progTex);
						}
						else
						{
							//SetMaterial(1);
							texture3->bind(progTex->getUniform("Texture0"));
							mesh_list2[i]->draw(progTex);
						}
					}
				Model->popMatrix();

			Model->popMatrix();

		progTex->unbind();

		//=======================================================
		progTex->bind();
			glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

			// draw mesh 
			Model->pushMatrix();
				float tx2, tz2;
				Model->loadIdentity();
				Model->translate(vec3(10, 0, 0));

				// AE86
				Model->pushMatrix();
					tx2 = (4.f) * sin(theta2);
					tz2 = (4.f) * cos(theta2);

					//Model->translate(vec3(10, -1, 0));
					Model->scale(vec3(1, 1, 1));
					Model->translate(vec3(tx2, -1, tz2));
					Model->rotate(3.14f + theta3, vec3(0, 1, 0));
					theta3 -= 0.05;

					setModel(progTex, Model);
					SetMaterial(3, progTex);
					glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
					glUniform1f(progTex->getUniform("Invert"), 1);
					for (int i = 0; i < mesh_list2.size(); i++)
					{
						if (i == 1 ||
							i == 2 ||
							i == 3 ||
							i == 4 ||
							i == 5 ||
							i == 6 ||
							i == 7 ||
							i == 8)
						{
							texture3->bind(progTex->getUniform("Texture0"));
							mesh_list2[i]->draw(progTex);
						}
						else
						{
							texture0->bind(progTex->getUniform("Texture0"));
							mesh_list2[i]->draw(progTex);
						}
					}
				Model->popMatrix();

			Model->popMatrix();

		progTex->unbind();

		//=======================================================
		progTex->bind();
			glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

			// draw mesh 
			Model->pushMatrix();

				Model->loadIdentity();
				Model->translate(vec3(0, 0, 0));

				// setting.
				Model->pushMatrix();
					Model->translate(vec3(0, -1, 0));
					Model->scale(vec3(1, 1, 1));

					SetMaterial(3, progTex);
					setModel(progTex, Model);
					glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
					glUniform1f(progTex->getUniform("Invert"), 1);
					texture3->bind(progTex->getUniform("Texture0"));
					for (int i = 0; i < mesh_list.size(); i++)
					{
						mesh_list[i]->draw(progTex);
					}
				Model->popMatrix();

			Model->popMatrix();

		progTex->unbind();

		//==================================================================
		progTex->bind();
			glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

			// draw mesh 
			Model->pushMatrix();

				Model->loadIdentity();
				Model->translate(vec3(0, 0, 0));

				// sphere.
				Model->pushMatrix();
					Model->translate(vec3(40, 20, 0));
					Model->rotate(90, vec3(0, 1, 0));
					Model->scale(vec3(5, 5, 5));

					setModel(progTex, Model);
					SetMaterial(3, progTex);
					glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
					glUniform1f(progTex->getUniform("Invert"), 1);
					texture0->bind(progTex->getUniform("Texture0"));
					for (int i = 0; i < mesh_list3.size(); i++)
					{
						mesh_list3[i]->draw(progTex);
					}
				Model->popMatrix();

			Model->popMatrix();

		progTex->unbind();

		//==================================================================
		progTex->bind();
			glUniformMatrix4fv(progTex->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glUniformMatrix4fv(progTex->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

			// draw mesh 
			Model->pushMatrix();

				Model->loadIdentity();
				Model->translate(vec3(0, 0, 0));

				// sphere.
				Model->pushMatrix();
					Model->translate(vec3(0, 20, 40));
					Model->scale(vec3(5, 5, 5));

					setModel(progTex, Model);
					SetMaterial(4, progTex);
					glUniform3f(progTex->getUniform("eye"), eye.x, eye.y, eye.z);
					glUniform1f(progTex->getUniform("Invert"), 1);
					texture4->bind(progTex->getUniform("Texture0"));
					for (int i = 0; i < mesh_list3.size(); i++)
					{
						mesh_list3[i]->draw(progTex);
					}
				Model->popMatrix();

			Model->popMatrix();

		progTex->unbind();

		//==================================================================
		//SKYBOX.
		auto Model2 = make_shared<MatrixStack>();
		//to draw the sky box bind the right shader
		progSky->bind();
			glUniformMatrix4fv(progSky->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
			glDepthFunc(GL_LEQUAL);
			glUniformMatrix4fv(progSky->getUniform("V"), 1, GL_FALSE, value_ptr(lookAt_value));

			Model2->pushMatrix();
				Model2->loadIdentity();
				Model2->scale(vec3(100, 100, 100));

				glUniformMatrix4fv(progSky->getUniform("M"), 1, GL_FALSE, value_ptr(Model2->topMatrix()));
				glBindTexture(GL_TEXTURE_CUBE_MAP, sky);
				cube_mesh->draw(progSky);
				glDepthFunc(GL_LESS);

			Model2->popMatrix();

		progSky->unbind();

		//=======================================================

		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char *argv[])
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
	windowManager->init(900, 600);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initTex(resourceDir);

	//==================================================================
	//SKYBOX.

	vector<std::string> faces
	{
		"vc_rt.tga",
		"vc_lf.tga",
		"vc_up.tga",
		"vc_dn.tga",
		"vc_ft.tga",
		"vc_bk.tga"
	};

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

	//==================================================================

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
