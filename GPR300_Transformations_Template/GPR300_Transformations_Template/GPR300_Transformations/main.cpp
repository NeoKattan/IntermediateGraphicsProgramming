#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/ShapeGen.h"

#include <cmath>

//using namespace glm;

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

float NEAR_PLANE = 0.01;
float FAR_PLANE = 100;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;

glm::vec3 bgColor = glm::vec3(0);
float exampleSliderFloat = 0.0f;

float orbitRadius = 10;
float orbitSpeed = 0;
float fieldOfView = 1;
float orthographicHeight = 10;
bool orthographicToggle = false;

class Transform {
public:
	Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s);
	~Transform();
	glm::mat4 GetModelMatrix();

private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

Transform::Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s) {
	position = p;
	rotation = r;
	scale = s;
}

Transform::~Transform() {

}

glm::mat4 Transform::GetModelMatrix() {
	glm::mat4  translationMat4 = { {1, 0, 0, 0},
									{0, 1, 0, 0},
									{0, 0, 1, 0},
									{position.x, position.y, position.z, 1} };

	glm::mat4 rX = {	{1, 0, 0, 0},
						{0, cos(rotation.x), sin(rotation.x), 0},
						{0, -sin(rotation.x), cos(rotation.x), 0},
						{0, 0, 0, 1} };

	glm::mat4 rY = {	{cos(rotation.y), 0, -sin(rotation.y), 0},
						{0, 1, 0, 0},
						{sin(rotation.y), 0, cos(rotation.y), 0},
						{0, 0, 0, 1} };

	glm::mat4 rZ = {	{cos(rotation.z), sin(rotation.z), 0, 0},
						{-sin(rotation.z), cos(rotation.z), 0, 0},
						{0, 0, 1, 0},
						{0, 0, 0, 1} };

	glm::mat4 rotationMat4 = rX * rY * rZ;

	glm::mat4 scaleMat4 = { {scale.x, 0, 0, 0},
							{0, scale.y, 0, 0},
							{0, 0, scale.z, 0},
							{0, 0, 0, 1} };

	glm::mat4 modelMatrix = translationMat4 * rotationMat4 * scaleMat4;

	//return glm::mat4(1);

	return modelMatrix; 
}

class Camera {
public:
	Camera();
	~Camera();
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
	glm::mat4 Ortho(float height, float aspectRatio, float nearPlane, float farPlane);
	glm::mat4 Perspective(float fov, float aspectRatio, float nearPlane, float farPlane);
	void Update();
private:
	glm::vec3 position;
	glm::vec3 target;
	float fov;
	float orthographicSize;
	bool orthographic;
};

Camera::Camera() {
	position = glm::vec3(0,0,orbitRadius);
	target = glm::vec3(0);
	fov = fieldOfView;
	orthographicSize = orthographicHeight;
	orthographic = orthographicToggle;
}

Camera::~Camera() {

}

glm::mat4 Camera::GetViewMatrix() {
	glm::vec3 f = glm::normalize(target - position);
	glm::vec3 r = glm::normalize(cross(f, glm::vec3(0, 1, 0)));
	glm::vec3 u = glm::normalize(cross(r, f));
	f = -f;

	glm::mat4 rCamInv = {	{r.x, u.x, f.x, 0},
							{r.y, u.y, f.y, 0},
							{r.z, u.z, f.z, 0},
							{0, 0, 0, 1} };

	glm::mat4 tCamInv = {	{1,0,0,0},
							{0,1,0,0},
							{0,0,1,0},
							{-position.x, -position.y, -position.z, 1} };

	glm::mat4 viewMatrix = rCamInv * tCamInv;

	//return glm::lookAt(position, target, glm::vec3(0, 1, 0));

	return viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() {
	if (orthographic) {
		return Ortho(orthographicHeight, (float)SCREEN_WIDTH / SCREEN_HEIGHT, NEAR_PLANE, FAR_PLANE);
	}
	else {
		return Perspective(fov, (float)SCREEN_WIDTH / SCREEN_HEIGHT, NEAR_PLANE, FAR_PLANE);
	}
}

glm::mat4 Camera::Ortho(float height, float aspectRatio, float nearPlane, float farPlane) {
	float r = height * aspectRatio / 2;
	float t = height / 2;
	float l = -r;
	float b = -t;

	//return glm::ortho(l, r, b, t);
	
	glm::mat4 orthoMatrix = {	{2 / (r - l), 0, 0, 0},
								{0, 2 / (t - b), 0, 0},
								{0, 0, -2 / (farPlane - nearPlane), 0},
								{-(r + l) / (r - l), -(t + b) / (t - b), -(farPlane + nearPlane) / (farPlane - nearPlane), 1} };

	return orthoMatrix;
}

glm::mat4 Camera::Perspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
	float c = tan(fov / 2);

	//return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	
	glm::mat4 perspectiveMatrix = { {1 / (aspectRatio * c), 0, 0, 0},
									{0, 1 / c, 0, 0},
									{0, 0, -(farPlane + nearPlane) / (farPlane - nearPlane), -1},
									{0, 0, -(2 * farPlane * nearPlane) / (farPlane - nearPlane), 1} };

	return perspectiveMatrix;
}

void Camera::Update() {
	//orbit
	glm::vec3 newPos = glm::normalize(position);
	newPos.x = (position.x * cos(orbitSpeed/100)) - (position.z * sin(orbitSpeed/100));
	newPos.z = (position.z * cos(orbitSpeed/100)) + (position.x * sin(orbitSpeed/100));
	position = orbitRadius * glm::normalize(newPos);

	//update from sliders
	fov = fieldOfView;
	orthographicSize = orthographicHeight;
	orthographic = orthographicToggle;
}

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	Shader shader("shaders/vertexShader.vert", "shaders/fragmentShader.frag");

	MeshData cubeMeshData;
	createCube(1.0f, 1.0f, 1.0f, cubeMeshData);

	Mesh cubeMesh(&cubeMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Array of cubes
	Transform *cubes[5];
	srand(time_t(0));
	for (int i = 0; i < 5; i++) {
		cubes[i] = new Transform(glm::vec3((rand() % 10) - 5, (rand() % 10) - 5, (rand() % 10) - 5),
								glm::vec3((rand() % 360), (rand() % 360), (rand() % 360)),
								glm::vec3((rand() % 5) + 0.1, (rand() % 5) + 0.1, (rand() % 5) + 0.1));
	}

	Transform coob(glm::vec3(0, 0, 0), glm::vec3(45, 45, 45), glm::vec3(1, 1, 1));

	Camera cam;

	while (!glfwWindowShouldClose(window)) {
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		//Draw
		shader.use();
		shader.setMat4("_View", cam.GetViewMatrix());
		shader.setMat4("_Projection", cam.GetProjectionMatrix());
		//shader.setMat4("_Model", coob.GetModelMatrix());
		//cubeMesh.draw();
		for (int i = 0; i < sizeof(cubes) / sizeof(cubes[0]); i++) {
			shader.setMat4("_Model", cubes[i]->GetModelMatrix());
			cubeMesh.draw();
		}

		//Draw UI
		ImGui::Begin("Settings");
		ImGui::Checkbox("Orthographic Toggle", &orthographicToggle);
		if (orthographicToggle) {
			ImGui::SliderFloat("Orthographic Height", &orthographicHeight, 1.0f, 100.0f);
		}
		else {
			ImGui::SliderFloat("Field of View", &fieldOfView, 0.0f, 3.14f);
		}
		ImGui::SliderFloat("Orbit Radius", &orbitRadius, 1.0f, 50.0f);
		ImGui::SliderFloat("Orbit Speed", &orbitSpeed, 0.0f, 10.0f);
		ImGui::End();
		cam.Update();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}