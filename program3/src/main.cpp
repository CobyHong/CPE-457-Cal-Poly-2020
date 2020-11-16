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

using namespace std;
using namespace glm;

int x_key = 0;

//lighting movement.
float mov_light = 0;
vec3 light_pos = vec3(-2, 2, -2);

//colors.
int obj_color = 0;
int obj_color2 = 1;
int obj_color3 = 2;
int obj_color4 = 3;
int obj_color5 = 4;

class Application : public EventCallbacks {

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Shape to be used (from  file) - modify to support multiple
	vector<shared_ptr<Shape>> mesh_list;
	vector<shared_ptr<Shape>> mesh_list2;
	vector<shared_ptr<Shape>> mesh_list3;
	vector<shared_ptr<Shape>> mesh_list4;
	vector<shared_ptr<Shape>> mesh_list5;
	vector<shared_ptr<Shape>> mesh_list6;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;
	shared_ptr<Shape> mesh2;
	shared_ptr<Shape> mesh3;
	shared_ptr<Shape> mesh4;
	shared_ptr<Shape> mesh5;
	shared_ptr<Shape> mesh6;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;

	//animation data
	float sTheta = 0;
	float sTheta2 = 0;
	float gTrans = 0;
	float gTrans2 = 0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			obj_color += 1;
			obj_color2 += 1;
			obj_color3 += 1;
			obj_color4 += 1;
			obj_color5 += 1;
		}
		if (key == GLFW_KEY_Q) {
			mov_light -= 0.5;
		}
		if (key == GLFW_KEY_E) {
			mov_light += 0.5;
		}
		if (key == GLFW_KEY_A) {
			gTrans -= 0.1;
		}
		if (key == GLFW_KEY_D) {
			gTrans += 0.1;
		}
		if (key == GLFW_KEY_W) {
			gTrans2 -= 0.1;
		}
		if (key == GLFW_KEY_S) {
			gTrans2 += 0.1;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS) {
			if (x_key == 0)
			{
				x_key = 1;
			}
			else if (x_key == 1)
			{
				x_key = 0;
			}
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

		//material properties.
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");

		//lighting.
		prog->addUniform("light_source");
	}

	void initGeom(const std::string& resourceDirectory)
	{

		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/city.obj").c_str());
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

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
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

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/AE86.obj").c_str());
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

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/icoNoNormals.obj").c_str());
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

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/lamp.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				mesh5 = make_shared<Shape>();
				mesh5->createShape(TOshapes[i]);
				mesh5->measure();
				mesh5->init();

				mesh_list5.push_back(mesh5);
			}
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/bunnyNoNorm.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			for (int i = 0; i < TOshapes.size(); i++)
			{
				mesh6 = make_shared<Shape>();
				mesh6->createShape(TOshapes[i]);
				mesh6->measure();
				mesh6->init();

				mesh_list6.push_back(mesh6);
			}
		}

	}

	void m_toggle(int *color)
	{
		if (*color == 5)
		{
			*color = 0;
		}
		SetMaterial(*color);
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
		glUniform3f(prog->getUniform("light_source"), light_pos.x + mov_light, light_pos.y, light_pos.z);
   }


	void SetMaterial(int i) {
		switch (i) {
		case 0: //midnight-blue.
			glUniform3f(prog->getUniform("MatAmb"), 0.1334, 0.3113, 0.34);
			glUniform3f(prog->getUniform("MatDif"), 0.234,0.435,0.6786);
			glUniform3f(prog->getUniform("MatSpec"), 0.3243, 0.1233, 0.2324);
			glUniform1f(prog->getUniform("shine"), 10.345);
			break;
		case 1: //pale-yellow.
			glUniform3f(prog->getUniform("MatAmb"), 0.423, 0.513, 0.314);
			glUniform3f(prog->getUniform("MatDif"), 0.23423,0.534132,0.4234);
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

	void render() {
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

		// View is global translation along negative z for now
		View->pushMatrix();
			View->loadIdentity();
			View->translate(vec3(0, 0, 0));

		// Draw a stack of cubes with indiviudal transforms
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));


		//=======================================================


		// draw mesh 
		Model->pushMatrix();

			Model->loadIdentity();
			Model->translate(vec3(0, -0.5, -5));
			Model->rotate(gTrans, vec3(0, 1, 0));
			//Model->rotate(gTrans2, vec3(1, 0, 0));

			// City.
			Model->pushMatrix();
			Model->translate(vec3(0, -0.425, -8));
			Model->scale(vec3(0.25, 0.25, 0.25));
			setModel(prog, Model);
			//SetMaterial(2);
			m_toggle(&obj_color5);
			//mesh2->draw(prog);
			for (int i = 0; i < mesh_list.size(); i++)
			{
				mesh_list[i]->draw(prog);
			}
			Model->popMatrix();

			// Ground.
			Model->pushMatrix();
				Model->translate(vec3(0, -0.5, 0));
				Model->scale(vec3(4, 0.1, 4));
				setModel(prog, Model);
				//SetMaterial(2);
				m_toggle(&obj_color);
				//mesh2->draw(prog);
				for (int i = 0; i < mesh_list2.size(); i++)
				{
					mesh_list2[i]->draw(prog);
				}
			Model->popMatrix();

			// Car.
			Model->pushMatrix();
				Model->translate(vec3(0, -0.45, 0));
				Model->scale(vec3(0.5, 0.5, 0.5));
				Model->rotate(-45, vec3(0, 1, 0));
				setModel(prog, Model);
				//SetMaterial(3);
				//m_toggle(&obj_color3);
				//mesh3->draw(prog);
				for (int i = 0; i < mesh_list3.size(); i++)
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
						m_toggle(&obj_color4);
						mesh_list3[i]->draw(prog);
					}
					else
					{
						m_toggle(&obj_color3);
						mesh_list3[i]->draw(prog);
					}
				}
			Model->popMatrix();

			// lamp-left.
			Model->pushMatrix();
				Model->translate(vec3(-1.6, 0.4, 0));
				Model->scale(vec3(0.002, 0.002, 0.002));
				setModel(prog, Model);
				//SetMaterial(3);
				m_toggle(&obj_color4);
				//mesh5->draw(prog);
				for (int i = 0; i < mesh_list5.size(); i++)
				{
					mesh_list5[i]->draw(prog);
				}
			Model->popMatrix();

			// lamp-right.
			Model->pushMatrix();
				Model->translate(vec3(1.5, 0.4, 0));
				Model->scale(vec3(0.002, 0.002, 0.002));
				Model->rotate(-3, vec3(0, 1, 0));
				setModel(prog, Model);
				//SetMaterial(3);
				m_toggle(&obj_color4);
				//mesh5->draw(prog);
				for (int i = 0; i < mesh_list5.size(); i++)
				{
					mesh_list5[i]->draw(prog);
				}
			Model->popMatrix();

			// icoNoNormals.
			Model->pushMatrix();
				Model->translate(vec3(0, 2.25, 0));
				Model->scale(vec3(0.2, 0.2, 0.2));
				setModel(prog, Model);
				//SetMaterial(3);
				m_toggle(&obj_color2);
				//mesh3->draw(prog);
				for (int i = 0; i < mesh_list4.size(); i++)
				{
					mesh_list4[i]->draw(prog);
				}
			Model->popMatrix();

			// bunnyNoNorm.
			Model->pushMatrix();
				Model->translate(vec3(-1, -0.625, 0.5));
				Model->scale(vec3(5, 5, 5));
				setModel(prog, Model);
				//SetMaterial(3);
				m_toggle(&obj_color2);
				//mesh3->draw(prog);
				for (int i = 0; i < mesh_list6.size(); i++)
				{
					mesh_list6[i]->draw(prog);
				}
			Model->popMatrix();

		Model->popMatrix();

		prog->unbind();


		//=======================================================

	
		//animation update example
		if (x_key == 1)
		{
			sTheta = sin(glfwGetTime());
			sTheta2 = cos(glfwGetTime());
		}

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
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
