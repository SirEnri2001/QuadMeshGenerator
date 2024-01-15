#pragma once
#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <memory>
#include <thread>

#include "camera.h"
#include "shader.h"
#include "drawable.h"
#include "mesh/mesh.h"
#include "test_operator.h"
#include "qmorph/qmorph_operator.h"

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
	void integrationTest();
protected:
	virtual void createGridGround();
	virtual void drawGridGround(const glm::mat4& projViewModel);

	bool show_demo_window = false;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	GLFWwindow* window;

	// Some shaders
	std::unique_ptr<Shader> mHalfedgeShader;
	std::unique_ptr<Shader> mLineShader;
	std::unique_ptr<Shader> mCurveShader;
	std::unique_ptr<Shader> mModelShader;
	std::unique_ptr<Shader> mGridShader;
	std::unique_ptr<Shader> mPointShader;

	// Some drawables
	std::unique_ptr<Drawable> mGridGround;
	std::unique_ptr<Mesh> mMesh;
	std::unique_ptr<MeshDisplay> mMeshDisplay;
	std::unique_ptr<QMorphDisplay> mQMorphDisplay;
	std::unique_ptr<MeshOperator> mMeshOperator;
	std::unique_ptr<MeshIO> mMeshIO;
	std::unique_ptr<MeshAssert> mMeshAssert;

	std::unique_ptr<Drawable> meshShading;
	std::unique_ptr<Drawable> meshFrame;
	std::unique_ptr<Drawable> heSelect;
	std::unique_ptr<Drawable> meshPoint;

	std::unique_ptr<std::thread> mThread;
	
	std::unique_ptr<TestOperator> testOperator;
	std::unique_ptr<QMorphOperator> qmorphOperator;
	std::unique_ptr<std::mutex> pauseMutex;

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

	bool displayFrame = true;
	float lineWidth = 1.f;
	float modelScale = 500.f;
	float halfedgeOffset = 2;
	float halfedgeLengthOffset = 9;

	void callDebugBreak() {
		__debugbreak();
	}
};