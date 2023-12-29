#pragma once
#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "camera.h"
#include "shader.h"
#include "drawable.h"
#include "mesh/mesh.h"

class Viewer
{
public:
	Viewer(const std::string& name);
	virtual ~Viewer();

	// The main loop of the viewer
	virtual void mainLoop();

	// Create GUI here
	virtual void createGUIWindow();

	// Draw all of the scene here
	virtual void drawScene();

	// Set all callback functions
	virtual void setCallbacks();

	// Some callback functions
	virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	virtual void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	virtual void scrollCallBack(GLFWwindow* window, double xoffset, double yoffset);

protected:
	virtual void createGridGround();
	virtual void drawGridGround(const glm::mat4& projViewModel);

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	GLFWwindow* window;

	// Some shaders
	std::unique_ptr<Shader> mPointShader;
	std::unique_ptr<Shader> mCurveShader;
	std::unique_ptr<Shader> mModelShader;
	std::unique_ptr<Shader> mGridShader;

	// Some drawables
	std::unique_ptr<Drawable> mGridGround;
	std::unique_ptr<Mesh> mMesh;
	std::unique_ptr<MeshDisplay> mMeshDisplay;
	std::unique_ptr<MeshOperator> mMeshOperator;
	std::unique_ptr<MeshIO> mMeshIO;
	std::unique_ptr<Drawable> drawable;

	// Screen size, update in each frame
	int windowWidth = 1920;
	int windowHeight = 1080;

	glm::vec3 mLightPos;

	// Camera control
	Camera mCamera;
	bool mFirstMouse = true;
	double mLastX, mLastY;
	bool mHoldLeftButton, mHoldMidButton, mHoldRightButton;
	bool mHoldLCtrl;

private:
	bool showDemoWindow = true;
	bool showAnotherWindow = false;
};