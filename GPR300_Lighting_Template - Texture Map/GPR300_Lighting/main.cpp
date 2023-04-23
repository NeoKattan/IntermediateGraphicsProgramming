#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
GLuint createTexture(const char* filePath);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

glm::vec3 bgColor = glm::vec3(0);

bool wireFrame = false;

struct DirLight {
	glm::vec3 color = glm::vec3(1);
	glm::vec3 direction;
	float intensity = 1;
};

DirLight dirLight;

struct PtLight {
	glm::vec3 color = glm::vec3(1);
	float intensity = 1;
	float linearAtt = 10;
};

PtLight ptLight1, ptLight2;

struct SpLight {
	glm::vec3 color = glm::vec3(1);
	glm::vec3 position = glm::vec3(0);
	glm::vec3 direction = glm::vec3(0);
	float intensity = 1;
	float linearAtt = 10;
	float minAngle = 30;
	float maxAngle = 60;
	float falloffCurve = 1;
};

SpLight spLight;

struct Material {
	glm::vec3 color = glm::vec3(1);
	float ambientK;
	float diffuseK;
	float specularK;
	float shininess;
};

Material material;

bool scrolling = false;
float scrollSpeed = 1;

const char* wrappingModes[] = { "Clamp To Edge", "Clamp To Border", "Repeat", "Mirrored Repeat" };
static const char* currentWrap = "Clamp To Edge";
int currentWrapMode = 0;

//Quincy Code CellShading
bool cellShadingEnabled = false;
int toon_color_levels = 8;
bool floorFuncEnabled = false;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	material.ambientK = 0.25;
	material.diffuseK = 0.5;
	material.specularK = 0.5;
	material.shininess = 100;

	dirLight.color = glm::vec3(0, 1, 0);
	dirLight.direction = glm::vec3(1, -1, 0);

	ptLight1.color = glm::vec3(1, 1, 1);
	//ptLight2.color = glm::vec3(0, 0, 1);

	//spLight.color = glm::vec3(0, 1, 1);
	//spLight.position = glm::vec3(3, 3, 0);
	//spLight.direction = glm::vec3(-1, -1, 0);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform1;
	ew::Transform lightTransform2;

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	lightTransform1.scale = glm::vec3(0.5f);
	lightTransform1.position = glm::vec3(0.0f, 5.0f, 0.0f);

	//lightTransform2.scale = glm::vec3(0.5f);
	//lightTransform2.position = glm::vec3(-1.0f, 5.0f, -1.0f);

	glActiveTexture(GL_TEXTURE0);
	GLuint bamboo = createTexture("../../Resources/Bamboo/Bamboo001A_4K_Color.jpg");
	
	glActiveTexture(GL_TEXTURE1);
	GLuint fabric = createTexture("../../Resources/Fabric/Fabric061_4K_Color.jpg");

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//UPDATE
		cubeTransform.rotation.x += deltaTime;

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());

		//Set some lighting uniforms
		 
		//Quincy Code Cell Shading
		//**************************
		litShader.setVec3("_DirLight[0].color", dirLight.color);
		litShader.setVec3("_DirLight[0].direction", normalize(dirLight.direction));
		litShader.setFloat("_DirLight[0].intensity", dirLight.intensity);
		litShader.setInt("CellShadingEnabled", cellShadingEnabled);
		litShader.setInt("toon_color_levels", toon_color_levels);
		litShader.setInt("floorFuncEnabled", floorFuncEnabled);
		//*******************************

		litShader.setVec3("_PtLight[0].position", lightTransform1.position);
		litShader.setVec3("_PtLight[0].color", ptLight1.color);
		litShader.setFloat("_PtLight[0].intensity", ptLight1.intensity);
		litShader.setFloat("_PtLight[0].linearAtt", ptLight1.linearAtt);

		//litShader.setVec3("_PtLight[1].position", lightTransform2.position);
		//litShader.setVec3("_PtLight[1].color", ptLight2.color);
		//litShader.setFloat("_PtLight[1].intensity", ptLight2.intensity);
		//litShader.setFloat("_PtLight[1].linearAtt", ptLight2.linearAtt);

		//litShader.setVec3("_SpLight[0].color", spLight.color);
		//litShader.setVec3("_SpLight[0].position", spLight.position);
		//litShader.setVec3("_SpLight[0].direction", normalize(spLight.direction));
		//litShader.setFloat("_SpLight[0].intensity", spLight.intensity);
		//litShader.setFloat("_SpLight[0].linearAtt", spLight.linearAtt);
		//litShader.setFloat("_SpLight[0].minAngle", cos(spLight.minAngle / 180 * 3.14159));
		//litShader.setFloat("_SpLight[0].maxAngle", cos(spLight.maxAngle / 180 * 3.14159));
		//litShader.setFloat("_SpLight[0].falloffCurve", spLight.falloffCurve);

		litShader.setInt("numDirLights", 1);
		litShader.setInt("numPtLights", 1);
		//litShader.setInt("numSpLights", 1);

		litShader.setVec3("_CameraPos", camera.getPosition());

		//Set some material uniforms
		litShader.setVec3("_Material.color", material.color);
		litShader.setInt("Scrolling", scrolling);
		litShader.setFloat("Time", (float)glfwGetTime() * scrollSpeed);
		litShader.setFloat("_Material.ambientK", material.ambientK);
		litShader.setFloat("_Material.diffuseK", material.diffuseK);
		litShader.setFloat("_Material.specularK", material.specularK);
		litShader.setFloat("_Material.shininess", material.shininess);

		litShader.setInt("first", 0);
		litShader.setInt("second", 1);

		//Draw cube
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//Draw plane
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());
		unlitShader.setMat4("_Model", lightTransform1.getModelMatrix());
		unlitShader.setVec3("_Color", ptLight1.color);
		sphereMesh.draw();
		//unlitShader.setMat4("_Model", lightTransform2.getModelMatrix());
		//unlitShader.setVec3("_Color", ptLight2.color);
		//sphereMesh.draw();

		//Draw UI
		ImGui::Begin("Material");
		//ImGui::ColorEdit3("Material Color", &material.color.r);
		ImGui::Checkbox("Scrolling", &scrolling);
		ImGui::SliderFloat("Scroll Speed", &scrollSpeed, 0, 1);

		if (ImGui::BeginCombo("Wrapping Mode", currentWrap)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(wrappingModes); n++)
			{
				bool is_selected = (currentWrap == wrappingModes[n]); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(wrappingModes[n], is_selected))
				{
					currentWrap = wrappingModes[n];
					currentWrapMode = n;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
			ImGui::EndCombo();
		}

		ImGui::SliderFloat("Material Ambient K", &material.ambientK, 0, 1);
		ImGui::SliderFloat("Material Diffuse K", &material.diffuseK, 0, 1);
		ImGui::SliderFloat("Material Specular K", &material.specularK, 0, 1);
		ImGui::SliderFloat("Material Shininess", &material.shininess, 1, 512);
		ImGui::End();

		//Quincy Code
		//********************************
		ImGui::Begin("Directional Light");
		ImGui::ColorEdit3("Color", &dirLight.color.r);
		ImGui::DragFloat3("Direction", &dirLight.direction.r, 1.0f, -5.0f, 5.0f);
		ImGui::SliderFloat("Intensity", &dirLight.intensity, 0, 1);
		ImGui::SliderInt("Cell Levels", &toon_color_levels, 1, 10);
		ImGui::Checkbox("Enable Cell Shading", &cellShadingEnabled);
		ImGui::Checkbox("Enable Floor Function", &floorFuncEnabled);
		ImGui::End();
		//*************************************

		ImGui::Begin("Point Lights");
		ImGui::ColorEdit3("Color 1", &ptLight1.color.r);
		ImGui::DragFloat3("Position 1", &lightTransform1.position.r, 1, -1, 1);
		ImGui::SliderFloat("Intensity 1", &ptLight1.intensity, 0, 1);
		ImGui::SliderFloat("Linear Attenuation 1", &ptLight1.linearAtt, 0, 10);
		//ImGui::ColorEdit3("Color 2", &ptLight2.color.r);
		//ImGui::DragFloat3("Position 2", &lightTransform2.position.r, 1, -1, 1);
		//ImGui::SliderFloat("Intensity 2", &ptLight2.intensity, 0, 1);
		//ImGui::SliderFloat("Linear Attenuation 2", &ptLight2.linearAtt, 0, 10);
		ImGui::End();

		//ImGui::Begin("Spot Light");
		//ImGui::ColorEdit3("Color", &spLight.color.r);
		//ImGui::DragFloat3("Position", &spLight.position.r, 1, -1, 1);
		//ImGui::DragFloat3("Direction", &spLight.direction.r, 1, -1, 1);
		//ImGui::SliderFloat("Intensity", &spLight.intensity, 0, 1);
		//ImGui::SliderFloat("Linear Attenuation", &spLight.linearAtt, 0, 10);
		//ImGui::SliderFloat("Min Angle", &spLight.minAngle, 0, spLight.maxAngle);
		//ImGui::SliderFloat("Max Angle", &spLight.maxAngle, spLight.minAngle, 180);
		//ImGui::SliderFloat("Falloff Curve", &spLight.falloffCurve, 0, 1);
		//ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}

//Create a function that loads a tecture from a file and returns a handle to it.
GLuint createTexture(const char* filePath) {

	//Generate a texture name
	GLuint texture;
	glGenTextures(1, &texture);

	//Bind our name to GL_TEXTURE_2D to make it a 2D texture.
	glBindTexture(GL_TEXTURE_2D, texture);

	//Load texture data as file
	int width, height, numComponents;
	unsigned char* textureData = stbi_load(filePath, &width, &height, &numComponents, 0);
	if (textureData == NULL) {
		printf("Texture data did not work");
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

	//Wrapping
	switch (currentWrapMode) {
		case 0:
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case 1:
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			break;
		case 2:
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case 3:
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			break;
	}

	//When magnififying, use nearest neighbor sampling
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//When minifying, use bilinear sampling
	glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	
	return texture;
}
