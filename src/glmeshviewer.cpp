#include "glmeshviewer.h"
#include "mesh/meshdisplay.h"
#include "mesh/meshoperator.h"
#include "mesh/meshio.h"
#include "qmorph/qmorph_display.h"
#include "thread_support/thread_support.h"

#include <future>


static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Copy from ImGUI/examples/example_glfw_opengl3
Viewer::Viewer(const std::string& name) :
	windowWidth(1920), windowHeight(1080),
	mCamera(windowWidth, windowHeight, glm::vec3(0, 0, 1000), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
	mMesh(std::make_unique<Mesh>()),
	mMeshOperator(std::make_unique<MeshOperator>(mMesh.get())), 
	mMeshDisplay(std::make_unique<MeshDisplay>(mMesh.get())),
	mMeshIO(std::make_unique<MeshIO>(mMesh.get()))
	//changed camerea position value to (0, 500, 800) from (0, 100, 500)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return;

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
	glfwWindowHint(GLFW_SAMPLES, 4);
	// Create window with graphics context
	window = glfwCreateWindow(windowWidth, windowHeight, "Mesh Viewer", nullptr, nullptr);
	if (window == nullptr)
		return;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
	glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
	glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	/* Our initializaton */
	//setCallbacks();

	mHalfedgeShader = std::make_unique<Shader>("../glsl/point3d.vert.glsl", "../glsl/point3d.frag.glsl",
		"../glsl/halfedge3d.geom.glsl");
	mLineShader = std::make_unique<Shader>("../glsl/line.vert.glsl", "../glsl/line.frag.glsl");
	mCurveShader = std::make_unique<Shader>("../glsl/curve.vert.glsl", "../glsl/curve.frag.glsl",
		"../glsl/curve.geom.glsl");
	mModelShader = std::make_unique<Shader>("../glsl/model.vert.glsl", "../glsl/model.frag.glsl");
	mGridShader = std::make_unique<Shader>("../glsl/grid.vert.glsl", "../glsl/grid.frag.glsl");
	mPointShader = std::make_unique<Shader>("../glsl/point.vert.glsl", "../glsl/point.frag.glsl",
		"../glsl/point.geom.glsl");
	meshShading = std::make_unique<Drawable>();
	meshFrame = std::make_unique<Drawable>();
	meshPoint = std::make_unique<Drawable>();
	heSelect = std::make_unique<Drawable>();
	createGridGround();
	mMeshIO->loadM("../test/data/mesh_214370.m");
	testOperator = std::make_unique<TestOperator>(mMesh.get());
	qmorphOperator = std::make_unique<QMorphOperator>(mMesh.get());
	mQMorphDisplay = std::make_unique<QMorphDisplay>(qmorphOperator.get(), mMeshDisplay.get());
	qmorphOperator->setDisplay(mQMorphDisplay->display);
	qmorphOperator->create();
	pauseMutex = std::make_unique<std::mutex>();
	testOperator->display = mQMorphDisplay.get();
	testOperator->qmorphOperator = qmorphOperator.get();
	mMeshDisplay->create();
	mMeshDisplay->createFrame();
}

Viewer::~Viewer()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Viewer::mainLoop()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Main loop
#ifdef __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = nullptr;
	EMSCRIPTEN_MAINLOOP_BEGIN
#else
	while (!glfwWindowShouldClose(window))
#endif
	{
		static const Halfedge* he = nullptr;
		static const Vertex* v = nullptr;
		static const Face* f = nullptr;
		static int counter = 0;
		static int heId = 0;
		static int faceId = 0;
		static int vertId = 0;
		static std::future<void> fu;
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize); // Create main panel
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			//ImGui::Checkbox("Display Frame", &displayFrame);
			//ImGui::SliderFloat("Frame Line Width", &lineWidth, 0.0001f, 0.01f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::SliderFloat("halfedgeOffset", &halfedgeOffset, 0.0001f, 0.01f);
			//ImGui::SliderFloat("halfedgeLengthOffset", &halfedgeLengthOffset, 0.0001f, 0.01f);
			//ImGui::SliderFloat("Scale Model", &modelScale, 0.1f, 1000.0f);
			if (ImGui::Button("Refresh Model")) {
				mMeshDisplay->create();
				mMeshDisplay->createFrame();
			}
			ImGui::DragInt("Face ID", &faceId, 1, 0, mMesh->getFaceIdTotal() - 1);
			ImGui::SameLine();
			if (ImGui::Button("Display Face"))
				if (mMesh->getFaces().find(faceId) == mMesh->getFaces().cend()) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else {
					mMeshDisplay->markFace(&mMesh->getFaces().at(faceId));
				}
			ImGui::DragInt("Vertex ID", &vertId, 1, 0, mMesh->getVertexIdTotal() - 1);
			ImGui::SameLine();
			if (ImGui::Button("Display Vertex"))
				if (mMesh->getVertices().find(vertId) == mMesh->getVertices().cend()) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else {
					mMeshDisplay->markVertex(&mMesh->getVertices().at(vertId));
					v = &mMesh->getVertices().at(vertId);
				}
			ImGui::SameLine();
			if (ImGui::Button("Vertex.Halfedge")) {
				if (!v) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else {
					mMeshDisplay->markHalfedge(v->getHalfedge());
					he = v->getHalfedge();
					heId = he->getId();
				}
			}
			ImGui::DragInt("Halfedge ID", &heId, 1, 0, mMesh->getHalfedgeIdTotal() - 1);
			if (ImGui::Button("Display Halfedge"))
				if (mMesh->getHalfedges().find(heId) == mMesh->getHalfedges().cend()) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else
				{
					he = &mMesh->getHalfedges().at(heId);
					mMeshDisplay->createFrame();
					mMeshDisplay->markHalfedge(he);
				}
			ImGui::SameLine();
			if (ImGui::Button("Next"))
				if (!he) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else
				{
					he = he->getNext();
					mMeshDisplay->createFrame();
					mMeshDisplay->markHalfedge(he);
				}
			ImGui::SameLine();
			if (ImGui::Button("Prev"))
				if (!he) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else
				{
					he = he->getPrev();
					mMeshDisplay->createFrame();
					mMeshDisplay->markHalfedge(he);
				}
			ImGui::SameLine();
			if (ImGui::Button("Sym"))
				if (!he) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else
				{
					he = he->getSym();
					mMeshDisplay->createFrame();
					mMeshDisplay->markHalfedge(he);
				}
			ImGui::SameLine();
			if (ImGui::Button("Cycle"))
				if (mMesh->getHalfedges().find(heId) == mMesh->getHalfedges().cend()) {
					ImGui::OpenPopup("Error Marking Component");
				}
				else
				{
					mMeshDisplay->createFrame();
					mMeshDisplay->markHalfedgeCycle(he);
				}
			ImGui::SameLine();
			if (ImGui::Button("Boundaries")) {
				mMeshDisplay->markBoundaries();
			}
			if (ImGui::Button("Process Test Operation")) {
				mMesh->setIntegrityCheck(true);
				fu = testOperator->async();
			}
			if (ImGui::Button("Process Q-Morph Operation")) {
				mMesh->setIntegrityCheck(true);
				fu = qmorphOperator->async();
			}

			if (ImGui::Button("Mark Front Edges")) {
				mQMorphDisplay->markFrontEdges();
			}

			if (ImGui::Button("Mark Front Edge Classes")) {
				mQMorphDisplay->markFrontEdgeClass();
			}
			if (!check_pause()) {
				if (ImGui::Button("Pause")) {
					call_pause();
				}
			}
			else {
				if (ImGui::Button("Resume")) {
					call_resume();
				}
			}

			if (ImGui::Button("Debug Break")) {
				callDebugBreak();
			}

			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("Error Marking Component", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Cannot find target component to mark!");
				ImGui::Separator();
				if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::SetItemDefaultFocus();
				ImGui::EndPopup();
			}
			ImGui::End();
			if (!check_pause()) {
				if (fu.valid()) {
					try
					{
						//fu.get();
						std::cout << "[Thread Complete] \n";
					}
					catch (std::exception& e)
					{
						std::cout << "[Thread Exception] " << e.what() << std::endl;
					}
					mMeshDisplay->create();
					mMeshDisplay->createFrame();
				}
			}
		}
		if (!io.WantCaptureMouse && ImGui::IsMouseDown(1)) {
			mCamera.PolarRotateAboutX(-io.MouseDelta.y / windowHeight * 1000.f);
			mCamera.PolarRotateAboutY(-io.MouseDelta.x / windowWidth * 1000.f);
		}
		else if (!io.WantCaptureMouse && io.MouseWheel != 0.0f) {
			mCamera.PolarZoom(io.MouseWheel * 300);
		}
		else if (!io.WantCaptureMouse && ImGui::IsMouseDown(0)) {
			v = nullptr;
			he = nullptr;
			f = nullptr;
			v = mMeshDisplay->selectVertex(glm::vec4(mCamera.eye, 1.0), mCamera.getRay(io.MousePos.x,io.MousePos.y));
			if (v) {
				mMeshDisplay->createFrame();
				mMeshDisplay->markVertex(v);
				vertId = v->getId();
			}
			else if (he = mMeshDisplay->selectHalfedge(glm::vec4(mCamera.eye, 1.0), 
				mCamera.getRay(io.MousePos.x, io.MousePos.y))) {
				heId = he->getId();
				mMeshDisplay->createFrame();
				mMeshDisplay->markHalfedge(he);
				heId = he->getId();
			}
		}
		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		drawScene();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_END;
#endif
	return;
}

void Viewer::createGUIWindow()
{
	
}

void Viewer::drawScene()
{
	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(modelScale));
	glm::mat4 projView = mCamera.getProjView();
	drawGridGround(projView);
	mModelShader->use();
	mModelShader->setMat4("uProjView", projView);
	mModelShader->setVec3("uLightPos", mCamera.eye);
	mModelShader->setVec3("uColor", glm::vec3(0.8, 0.8, 0.8));
	mModelShader->setMat4("uModel", model);
	mModelShader->setMat3("uModelInvTr", glm::inverse(model));
	glBindVertexArray(meshShading->VAO);
	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshShading->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mMeshDisplay->indices.size() * sizeof(GLuint), mMeshDisplay->indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, meshShading->VBO);
	glBufferData(GL_ARRAY_BUFFER, mMeshDisplay->vertexBuffer.size() * sizeof(glm::vec4), mMeshDisplay->vertexBuffer.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
	glBindVertexArray(meshShading->VAO);
	glDrawElements(GL_TRIANGLES, mMeshDisplay->indices.size(), GL_UNSIGNED_INT, 0);

	if (displayFrame) {

		mHalfedgeShader->use();
		mHalfedgeShader->setMat4("uProjView", projView);
		mHalfedgeShader->setVec3("uLightPos", mCamera.eye);
		mHalfedgeShader->setVec3("uColor", glm::vec3(0, 0, 0));
		mHalfedgeShader->setMat4("uModel", model);
		mHalfedgeShader->setMat3("uModelInvTr", glm::inverse(model));
		mHalfedgeShader->setVec2("uScreenSize", glm::vec2(windowWidth, windowHeight));
		mHalfedgeShader->setInt("markCount", 5);
		mHalfedgeShader->setFloat("thickness", lineWidth);
		mHalfedgeShader->setFloat("halfedgeOffset", halfedgeOffset);
		mHalfedgeShader->setFloat("halfedgeLengthOffset", halfedgeLengthOffset);
		glBindVertexArray(meshFrame->VAO);
		// Allocate space and upload the data from CPU to GPU
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshFrame->IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mMeshDisplay->frameIndices.size() * sizeof(GLuint), mMeshDisplay->frameIndices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, meshFrame->VBO);
		glBufferData(GL_ARRAY_BUFFER, mMeshDisplay->frameVertexBuffer.size() * sizeof(glm::vec4), mMeshDisplay->frameVertexBuffer.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)(2*sizeof(glm::vec4)));
		glBindVertexArray(meshFrame->VAO);
		glDrawElements(GL_LINES, mMeshDisplay->frameIndices.size(), GL_UNSIGNED_INT, 0);
		
		mPointShader->use();
		mPointShader->setFloat("uSize", 5.0);
		mPointShader->setMat4("uModel", model);
		mPointShader->setMat4("uProjView", projView);
		mPointShader->setVec3("uColor", glm::vec3(1, 0, 0));
		glBindVertexArray(meshPoint->VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshPoint->IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mMeshDisplay->pointIndices.size() * sizeof(GLuint), mMeshDisplay->pointIndices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, meshPoint->VBO);
		glBufferData(GL_ARRAY_BUFFER, mMeshDisplay->pointScatter.size() * sizeof(glm::vec4), mMeshDisplay->pointScatter.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
		glBindVertexArray(meshPoint->VAO);
		glDrawElements(GL_POINTS, mMeshDisplay->pointIndices.size(), GL_UNSIGNED_INT, 0);
	}

	mMeshDisplay->modelMat = model;
}

void Viewer::createGridGround()
{
	mGridGround = std::make_unique<Drawable>();
	int lineNum = 21;	// Number of grid line
	float length = 500;
	float padding = 50;
	mGridGround->elementCount = lineNum * 2 * 2;	// X direction and Z direction, each has two vertices
	std::vector<glm::vec3> buffer;	// Position and color
	for (int i = -lineNum / 2; i <= lineNum / 2; ++i)
	{
		glm::vec3 color{ 0.75, 0.75, 0.75 };
		// X direction
		if (i == 0) { color = glm::vec3(1, 0, 0); }
		buffer.emplace_back(glm::vec3{ -length, 0, i * padding });
		buffer.emplace_back(color);
		buffer.emplace_back(glm::vec3{ length, 0, i * padding });
		buffer.emplace_back(color);
		// Z direction
		if (i == 0) { color = glm::vec3(0, 1, 0); }
		buffer.emplace_back(glm::vec3{ i * padding, 0, -length });
		buffer.emplace_back(color);
		buffer.emplace_back(glm::vec3{ i * padding, 0, length });
		buffer.emplace_back(color);
	}
	glBindVertexArray(mGridGround->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, mGridGround->VBO);
	glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(glm::vec3), buffer.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);	// pos
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);	// col
}

void Viewer::drawGridGround(const glm::mat4& projViewModel)
{
	mGridShader->use();
	mGridShader->setMat4("uProjViewModel", projViewModel);
	glBindVertexArray(mGridGround->VAO);
	glDrawArrays(GL_LINES, 0, mGridGround->elementCount);
}
