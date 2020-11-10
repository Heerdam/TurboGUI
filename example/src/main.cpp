
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <imgui.h>

#include <tb_gui.h>

void GLAPIENTRY MessageCallback(GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
	if (type != GL_DEBUG_TYPE_ERROR) return;
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

static bool g_MouseJustPressed[ImGuiMouseButton_COUNT] = {};

static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data) {
	return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text) {
	glfwSetClipboardString((GLFWwindow*)user_data, text);
}

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
		g_MouseJustPressed[button] = true;
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	// Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
#ifdef _WIN32
	io.KeySuper = false;
#else
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
}

void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c) {
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(c);
}

static void ImGui_ImplGlfw_UpdateMousePosAndButtons(GLFWwindow* _window) {
	// Update buttons
	ImGuiIO& io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(_window, i) != 0;
		g_MouseJustPressed[i] = false;
	}

	// Update mouse position
	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
#ifdef __EMSCRIPTEN__
	const bool focused = true; // Emscripten
#else
	const bool focused = glfwGetWindowAttrib(_window, GLFW_FOCUSED) != 0;
#endif
	if (focused) {
		if (io.WantSetMousePos) {
			glfwSetCursorPos(_window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
		} else {
			double mouse_x, mouse_y;
			glfwGetCursorPos(_window, &mouse_x, &mouse_y);
			io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		}
	}
}

int main() {

	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_REFRESH_RATE, 144);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	const float width = 1920.f;
	const float height = 1080.f;

	GLFWwindow* window = glfwCreateWindow(width, height, "TurboGUI Example 0.1a", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return -1;
	}

	if (glfwExtensionSupported("GL_ARB_buffer_storage") == GLFW_FALSE) {
		std::cout << "extension GL_ARB_buffer_storage not supported" << std::endl;
		return 0;
	}

	glDebugMessageCallback(MessageCallback, 0);

	glfwSwapInterval(0);

	//create GUI before trying to set configs
	TurboGUI::GUI gui;

	//Fonts and size
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();
		io.DisplaySize = ImVec2(width, height);
	}
	//Inputs
	{
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
		io.BackendPlatformName = "imgui_glfw";

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
		glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
		glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
		glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
	}

	gui.initGL((unsigned int)(100000), (unsigned int)(200000));

	bool show_demo_window = true;

	double time = glfwGetTime();
	unsigned int frame = 0, fps = 0;
	while (!glfwWindowShouldClose(window)) {

		double ctime = glfwGetTime();

		glClearColor(0.f, 0.f, 0.4f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glfwPollEvents();
		ImGui_ImplGlfw_UpdateMousePosAndButtons(window);

		gui.begin();
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		gui.drawStatsWindow(fps);

		gui.draw();
		gui.sync();

		glfwSwapBuffers(window);

		frame++;
		if (ctime - time >= 1.0) {
			
			fps = frame;
			frame = 0;
			time = ctime;
			
		}
	}
	glfwTerminate();
	return 0;
}