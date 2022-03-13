//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include "Windows.h"
#include "MMSystem.h"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

double oldX, oldY;
double totalPitch;

glm::mat4 moonMat(1.0f);
glm::mat4 armMat(1.0f);
glm::mat4 rocketMat(1.0f);

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightPos;
GLuint lightPosLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;

glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 lightColor2;
GLuint lightColorLoc2;

gps::Camera myCamera(
				glm::vec3(0.0f, 0.0f, 2.5f), 
				glm::vec3(0.0f, 0.0f, -10.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.2f;

bool pressedKeys[1024];
float angleY = 0.0f;

std::vector<const GLchar*> faces;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;


std::vector<glm::mat4> generatedMatrices;
int objNum = 0;

gps::Model3D asteroidModel;
gps::Model3D myModel;
gps::Model3D moonModel;
gps::Model3D armModel;
gps::Model3D rocketModel;
gps::Shader myCustomShader;


GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (!(oldX == 0.0f && oldY == 0.0f)) {
		totalPitch += (oldY - ypos) * 0.002;
		if (abs(totalPitch) < 1.55f) {
			myCamera.rotate((oldY - ypos) * 0.002, (oldX - xpos) * 0.002);
			//update view matrix
			view = myCamera.getViewMatrix();
			myCustomShader.useShaderProgram();
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			// compute normal matrix for teapot
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		}
		else {
			totalPitch -= (oldY - ypos) * 0.002;
		}
	}
	oldX = xpos;
	oldY = ypos;
}


void processMovement()
{
	
	if (pressedKeys[GLFW_KEY_Q]) {
		myCustomShader.useShaderProgram();
		angleY -= 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	if (pressedKeys[GLFW_KEY_E]) {
		myCustomShader.useShaderProgram();
		angleY += 1.0f;
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCustomShader.useShaderProgram();
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		skyboxShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
			glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCustomShader.useShaderProgram();
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		skyboxShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
			glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCustomShader.useShaderProgram();
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		skyboxShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
			glm::value_ptr(view));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCustomShader.useShaderProgram();
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		skyboxShader.useShaderProgram();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
			glm::value_ptr(view));
	}
	if (pressedKeys[GLFW_KEY_M]) {
		moonMat = glm::rotate(moonMat, 0.01f, glm::vec3(0.1f, 0.0f, 0.0f));
	}
	if (pressedKeys[GLFW_KEY_N]) {
		moonMat = glm::rotate(moonMat, -0.01f, glm::vec3(0.1f, 0.0f, 0.0f));
	}
	if (pressedKeys[GLFW_KEY_K]) {
		armMat = glm::translate(armMat,glm::vec3(-21.438f, 3.3587f, 0.17045));
		armMat = glm::rotate(armMat, 0.03f, glm::vec3(0.1f, 0.0f, 0.0f));
		armMat = glm::translate(armMat, glm::vec3(21.438f, -3.3587f, -0.17045));
	}
	if (pressedKeys[GLFW_KEY_J]) {
		armMat = glm::translate(armMat, glm::vec3(-21.438f, 3.3587f, 0.17045));
		armMat = glm::rotate(armMat, -0.03f, glm::vec3(0.1f, 0.0f, 0.0f));
		armMat = glm::translate(armMat, glm::vec3(21.438f, -3.3587f, -0.17045));
	}
	if (pressedKeys[GLFW_KEY_I]) {
		rocketMat = glm::translate(rocketMat, glm::vec3(0.0f, 0.2f, 0.0f));
	}
	if (pressedKeys[GLFW_KEY_O]) {
		rocketMat = glm::translate(rocketMat,  glm::vec3(0.0f, -0.2f, 0.0f));
	}
	if (pressedKeys[GLFW_KEY_Z]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_C]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_F]) {
		PlaySound(TEXT("SpaceLoop.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	}

	if (pressedKeys[GLFW_KEY_G]) {
		glm::vec3 direction = myCamera.getCameraDirection();
		glm::vec3 position = myCamera.getCameraPosition();
		float alpha = -1.0f * position.y / direction.y;
		generatedMatrices.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(position.x + alpha * direction.x, 0.0f, position.z + alpha * direction.z)));
		objNum++;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initFBO() {

	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initObjects() {
	myModel.LoadModel("objects/Scene/scene.obj", "objects/Scene/");
	moonModel.LoadModel("objects/Planet/planet.obj", "objects/Planet/");
	armModel.LoadModel("objects/Arm/arm.obj", "objects/Arm/");
	rocketModel.LoadModel("objects/Rocket/rocket.obj", "objects/Rocket/");
	asteroidModel.LoadModel("objects/Asteroid/asteroid.obj", "objects/Asteroid/");
	mySkyBox.Load(faces);
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {

	myCustomShader.useShaderProgram();
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightPos = glm::vec3(21.438f, -3.3587f, -0.17045);
	lightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	//set light color
	lightColor = glm::vec3(0.047f, 0.615f, 0.152f); //green light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set the light direction
	lightDir = glm::vec3(-1.0f, 1.0f, 0.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor2 = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor2");
	glUniform3fv(lightColorLoc2, 1, glm::value_ptr(lightColor2));

	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));
}

glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(glm::vec3(-5.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = -30.0f, far_plane = 30.0f;
	glm::mat4 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void renderScene() {

	moonMat = glm::rotate(moonMat, 0.01f, glm::vec3(0.005f, 0.0f, 0.0f));
	armMat = glm::translate(armMat, glm::vec3(-21.438f, 3.3587f, 0.17045));
	armMat = glm::rotate(armMat, 0.03f, glm::vec3(0.1f, 0.0f, 0.0f));
	armMat = glm::translate(armMat, glm::vec3(21.438f, -3.3587f, -0.17045));

	// depth maps creation pass

	//render the scene to the depth buffer
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	depthMapShader.useShaderProgram();

	myModel.Draw(depthMapShader);

	model = moonMat;
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	moonModel.Draw(depthMapShader);

	model = armMat;
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	armModel.Draw(depthMapShader);

	model = rocketMat;
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	rocketModel.Draw(depthMapShader);

	for (int i = 0; i < objNum; i++) {
		model = generatedMatrices.at(i);
		modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		asteroidModel.Draw(depthMapShader);
	}
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);




	glViewport(0, 0, retina_width, retina_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	myCustomShader.useShaderProgram();

	//bind the shadow map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	myModel.Draw(myCustomShader);

	model = moonMat;
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	moonModel.Draw(myCustomShader);

	model = armMat;
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	armModel.Draw(myCustomShader);

	model = rocketMat;
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	rocketModel.Draw(myCustomShader);

	for (int i = 0; i < objNum; i++) {
		model = generatedMatrices.at(i);
		modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		asteroidModel.Draw(myCustomShader);
	}

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glfwDestroyWindow(glWindow);
	glDeleteFramebuffers(1, &shadowMapFBO);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	

	faces.push_back("textures/skybox/bkg3_right1.png");
	faces.push_back("textures/skybox/bkg3_left2.png");
	faces.push_back("textures/skybox/bkg3_top3.png");
	faces.push_back("textures/skybox/bkg3_bottom4.png");
	faces.push_back("textures/skybox/bkg3_front5.png");
	faces.push_back("textures/skybox/bkg3_back6.png");

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
