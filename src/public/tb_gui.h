
#pragma once

#include "tb_defines.h"

namespace TurboGUI {

	class TurboGuiException : public std::exception {
		const std::string msg;
	public:
		TurboGuiException(std::string _msg) : msg(_msg) {};
		virtual const char* what() const throw() {
			return msg.c_str();
		}
	};

	class GUI {

		GLuint VAO[2], VBO[2], EBO[2];
		GLuint tex;
		GLuint shader;

		float* VBO_ptr[2];
		ushort* EBO_ptr[2];

		GLsync syncObj[2];

		uint currIndex = 0;

		std::chrono::high_resolution_clock::time_point time;
		float drawTime = 0.f; //ms
		std::vector<float> drawTimeMean = std::vector<float>(100);
		uint drawMeanTimeIndex = 0;
		float meanTime = 0.f;

		uint idxBound, vertBound;
		uint maxIdx = 0, maxVert = 0;
		uint idx, vert;
		uint maxFps = 0;

		ImGuiContext* context;

	public:
		GUI() { context = ImGui::CreateContext(); }
		void initGL(uint, uint);
		/* use ImGui::GetIO() to set up mouse and keyboard inputs before calling this */
		void begin();
		void draw();
		void sync();

		void drawStats(uint);
	};


}