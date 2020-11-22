/* 
Using a framebuffer as a basis to implement Gaussian blur in GLSL
Currently will make 2 FBOs and textures (only uses one in base code)
and writes out frame as a .png (Texture_output.png)

Winter 2017 - ZJW (Piddington texture write)
Look for "TODO" in this file and write new shaders
*/
#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "GLTextureWriter.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

void initQuad();

using namespace std;
using namespace glm;


class Application : public EventCallbacks
{

public:
	WindowManager * windowManager = nullptr;

	shared_ptr<Program> prog, tex_prog, tex_prog2, blur_prog, blur_prog2;
	shared_ptr<Shape> bunny;

	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;
	shared_ptr<Texture> texture2;

	int DEBUG_LIGHT = 0;
	int GEOM_DEBUG = 1;
	int g_GiboLen;
	int g_width, g_height;
    int gMat = 1;
	int FirstTime = 1;

    bool mouseDown = false;

    //global reference to texture FBO
    GLuint frameBuf[2];
    GLuint texBuf[2];
    GLuint depthBuf;

	float g_Camtrans = -2.5;
	vec3 g_light = vec3(1, 1, 1);
	float updateDir = 0.5;
	double g_phi, g_theta;

	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(0, 1, 0);
	vec3 g_lookAt = vec3(0, 1, -4);

	//global data for ground plane
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	static void error_callback(int error, const char *description) {
		cerr << description << endl;
	}

    /* helper functions leave them be */
  void SetProjectionMatrix(shared_ptr<Program> curShade) {
    int width, height;
    glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
    float aspect = width/(float)height;
    mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 100.0f);
    glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
  }

  /* camera controls - do not change */
  void SetView(shared_ptr<Program> curShade) {
    mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
    glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
  }

  /* model transforms */
  void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
    mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
    mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
    mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
    mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
    mat4 ctm = Trans*RotX*RotY*ScaleS;
    glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  }

  /* heler to draw the geometry */
  void drawScene(shared_ptr<Program> shader) {
      //draw the mesh  
      SetMaterial((gMat + 1) % 4);
      SetModel(vec3(0.5, 0, -3), 0, 0, 1.5 / (bunny->max.x - bunny->min.x), shader);
      bunny->draw(shader);

      SetMaterial(0);
      //draw the ground plane
      SetModel(vec3(0, -1, 0), 0, 0, 1, shader);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(2);
      glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

      // actual draw!
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
      glDisableVertexAttribArray(2);
  }

  void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  float speed = 0.2;

  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    g_eye -= speed*strafe;
    g_lookAt -= speed*strafe;
  }
  if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    g_eye += speed*strafe;
    g_lookAt += speed*strafe;
  }
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    g_eye -= speed*view;
    g_lookAt -= speed*view;
  }
  if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    g_eye += speed*view;
    g_lookAt += speed*view;
  }
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    g_light.x += 0.25;
  if (key == GLFW_KEY_E && action == GLFW_PRESS)
    g_light.x -= 0.25;
  if (key == GLFW_KEY_M && action == GLFW_PRESS)
    g_Camtrans += 0.25;
  if (key == GLFW_KEY_N && action == GLFW_PRESS)
    g_Camtrans -= 0.25;
  if (key == GLFW_KEY_L && action == GLFW_PRESS)
    DEBUG_LIGHT = !DEBUG_LIGHT;

   if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
  }

  void scroll_callback(GLFWwindow* window, double deltaX, double deltaY) {
   vec3 diff, newV;
   //cout << "xDel + yDel " << deltaX << " " << deltaY << endl;
   g_theta += deltaX;
   g_phi += deltaY;
   newV.x = cosf(g_phi*(3.14/180.0))*cosf(g_theta*(3.14/180.0));
   newV.y = -1.0*sinf(g_phi*(3.14/180.0));
   newV.z = 1.0*cosf(g_phi*(3.14/180.0))*cosf((90.0-g_theta)*(3.14/180.0));
   diff.x = (g_lookAt.x - g_eye.x) - newV.x;
   diff.y = (g_lookAt.y - g_eye.y) - newV.y;
   diff.z = (g_lookAt.z - g_eye.z) - newV.z;
   g_lookAt.x = g_lookAt.x - diff.x;
   g_lookAt.y = g_lookAt.y - diff.y;
   g_lookAt.z = g_lookAt.z - diff.z;
   view = g_eye - g_lookAt;
   strafe = cross(vec3(0, 1,0), view);
  }

  void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
      double posX, posY;

      if (action == GLFW_PRESS)
      {
          mouseDown = true;
          glfwGetCursorPos(window, &posX, &posY);
          cout << "Pos X " << posX << " Pos Y " << posY << endl;
      }

      if (action == GLFW_RELEASE)
      {
          mouseDown = false;
      }
  }

  void resizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
  }

  void init(const std::string& resourceDirectory) {
      GLSL::checkVersion();

      int width, height;
      glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

      g_phi = 0;
      g_theta = -90;

      // Set background color.
      glClearColor(.12f, .34f, .56f, 1.0f);
      // Enable z-buffer test.
      glEnable(GL_DEPTH_TEST);

      // Initialize the GLSL program to render the obj
      prog = make_shared<Program>();
      prog->setVerbose(true);
      prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
      prog->init();
      prog->addUniform("P");
      prog->addUniform("V");
      prog->addUniform("M");
      prog->addUniform("MatAmb");
      prog->addUniform("MatDif");
      prog->addAttribute("vertPos");
      prog->addAttribute("vertNor");

      //set up the shaders to blur the FBO decomposed just a placeholder pass thru now
      //TODO - modify and possibly add other shaders to complete blur
      tex_prog = make_shared<Program>();
      tex_prog->setVerbose(true);
      tex_prog->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/shift_frag1.glsl");
      tex_prog->init();
      tex_prog->addUniform("texBuf");
      tex_prog->addAttribute("vertPos");

      tex_prog2 = make_shared<Program>();
      tex_prog2->setVerbose(true);
      tex_prog2->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/shift_frag2.glsl");
      tex_prog2->init();
      tex_prog2->addUniform("texBuf");
      tex_prog2->addAttribute("vertPos");

      blur_prog = make_shared<Program>();
      blur_prog->setVerbose(true);
      blur_prog->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/tex_frag_blur1.glsl");
      blur_prog->init();
      blur_prog->addUniform("texBuf");
      blur_prog->addAttribute("vertPos");

      blur_prog2 = make_shared<Program>();
      blur_prog2->setVerbose(true);
      blur_prog2->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/tex_frag_blur2.glsl");
      blur_prog2->init();
      blur_prog2->addUniform("texBuf");
      blur_prog2->addAttribute("vertPos");

      //create two frame buffer objects to toggle between
      //make two FBOs and two textures
      glGenFramebuffers(2, frameBuf);
      glGenTextures(2, texBuf);
      glGenRenderbuffers(1, &depthBuf);

      //create one FBO
      createFBO(frameBuf[0], texBuf[0]);

      //set up depth necessary since we are rendering a mesh that needs depth test
      glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

      //more FBO set up
      GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
      glDrawBuffers(1, DrawBuffers);

      //create another FBO so we can swap back and forth
      createFBO(frameBuf[1], texBuf[1]);
      //this one doesn't need depth - its just an image to process into
  }

  /* code to read in meshes and define the ground plane */
  void initGeom(const std::string& resourceDirectory) {

      vector<tinyobj::shape_t> TOshapes;
      vector<tinyobj::material_t> objMaterials;
      string errStr;
      //load in the mesh and make the shape(s)
      bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/bunny.obj").c_str());
      if (!rc) {
          cerr << errStr << endl;
      }
      else {
          bunny = make_shared<Shape>();
          bunny->createShape(TOshapes[0]);
          bunny->measure();
          bunny->init();
      }

      float g_groundSize = 20;
      float g_groundY = -1.5;

      // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
      float GrndPos[] = {
       -g_groundSize, g_groundY, -g_groundSize,
       -g_groundSize, g_groundY,  g_groundSize,
       g_groundSize, g_groundY,  g_groundSize,
       g_groundSize, g_groundY, -g_groundSize
      };

      float GrndNorm[] = {
       0, 1, 0,
       0, 1, 0,
       0, 1, 0,
       0, 1, 0,
       0, 1, 0,
       0, 1, 0
      };

      static GLfloat GrndTex[] = {
         0, 0, // back
         0, 1,
         1, 1,
         1, 0 };

      unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };

      GLuint VertexArrayID;
      //generate the VAO
      glGenVertexArrays(1, &VertexArrayID);
      glBindVertexArray(VertexArrayID);

      g_GiboLen = 6;
      glGenBuffers(1, &GrndBuffObj);
      glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      glGenBuffers(1, &GrndNorBuffObj);
      glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      glGenBuffers(1, &GrndTexBuffObj);
      glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      glGenBuffers(1, &GIndxBuffObj);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

      initQuad();
  }

  /**** geometry set up for a quad *****/
  void initQuad() {
      //now set up a simple quad for rendering FBO
      glGenVertexArrays(1, &quad_VertexArrayID);
      glBindVertexArray(quad_VertexArrayID);

      static const GLfloat g_quad_vertex_buffer_data[] =
      {
          -1.0f, -1.0f, 0.0f,
           1.0f, -1.0f, 0.0f,
          -1.0f,  1.0f, 0.0f,
          -1.0f,  1.0f, 0.0f,
           1.0f, -1.0f, 0.0f,
           1.0f,  1.0f, 0.0f,
      };

      glGenBuffers(1, &quad_vertexbuffer);
      glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

  }

  /*
Helper function to create the framebuffer object and associated texture to write to
*/
  void createFBO(GLuint& fb, GLuint& tex) {
      //initialize FBO (global memory)
      int width, height;
      glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

      //set up framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, fb);
      //set up texture
      glBindTexture(GL_TEXTURE_2D, tex);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
          cout << "Error setting up frame buffer - exiting" << endl;
          exit(0);
      }
  }

  /* Process Texture on the specificed texture  - could vary what it does based on
      shader  - works on inTex - runs shaders and output to textured quad */
      /* TODO: fill in with call to appropriate shader(s) to complete the texture processding */
  void ProcessDrawTex(GLuint inTex, std::shared_ptr<Program> tex_prog) {

      //set up inTex as my input texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, inTex);
      //example applying of 'drawing' the FBO texture
      //this shader just draws right now
      tex_prog->bind();
      glUniform1i(tex_prog->getUniform("texBuf"), 0);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glDisableVertexAttribArray(0);
      tex_prog->unbind();
  }

  /* let's draw */
  void render() {

      // Get current frame buffer size.
      int width, height;
      glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
      glViewport(0, 0, width, height);

      glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);

      // Clear framebuffer.
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      prog->bind();
      //render scene
      SetProjectionMatrix(prog);
      SetView(prog);
      SetMaterial((gMat + 1) % 4);
      drawScene(prog);
      prog->unbind();


      if (mouseDown)
      {
          for (int i = 0; i < 200; i++)
          {
              glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[(i + 1) % 2]);
              glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
              // process the 2D rendering of the original 3D scene through your first frame buffer (vertical shift)
              ProcessDrawTex(texBuf[i % 2], blur_prog);
          }

          //regardless NOW set up to render to the screen = 0
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          // Draw the processed texture with the 2nd FBO with horizontal shift
          ProcessDrawTex(texBuf[1], blur_prog2);
      }
      else
      {
          glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[1]);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          // process the 2D rendering of the original 3D scene through your first frame buffer (vertical shift)
          ProcessDrawTex(texBuf[0], tex_prog);

          //regardless NOW set up to render to the screen = 0
          glBindFramebuffer(GL_FRAMEBUFFER, 0);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          // Draw the processed texture with the 2nd FBO with horizontal shift
          ProcessDrawTex(texBuf[1], tex_prog2);
      }


      /* code to write out the FBO (texture) just once  - this is for debugging*/
      /* Note that texBuf[0] corresponds to frameBuf[0] */
      if (FirstTime) {
          assert(GLTextureWriter::WriteImage(texBuf[0], "Texture_output.png"));
          FirstTime = 0;
      }

  }

  //helper function to set materials for shading
  void SetMaterial(int i) {
      switch (i) {
      case 0: //shiny blue plastic
          glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
          glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
          break;
      case 1: // flat grey
          glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
          glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
          break;
      case 2: //brass
          glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
          glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
          break;
      case 3: //copper
          glUniform3f(prog->getUniform("MatAmb"), 0.1913, 0.0735, 0.0225);
          glUniform3f(prog->getUniform("MatDif"), 0.7038, 0.27048, 0.0828);
          break;
      }
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
	while (! glfwWindowShouldClose(windowManager->getHandle())) {
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

