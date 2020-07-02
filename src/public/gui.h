
#include "defines.h"

namespace TurboGUI {

	class TurboGuiException : public std::exception {
		const std::string msg;
	public:
		TurboGuiException(std::string _msg) : msg(_msg) {};
		virtual const char* what() const throw() {
			return msg.c_str();
		}
	};

	//template<class T>
	class GUI {

		GLuint VAO[2], VBO[2], EBO[2];
		GLuint tex;
		GLuint shader;

		float* VBO_ptr[2];
		ushort* EBO_ptr[2];

		GLsync syncObj[2];

		uint currIndex = 0;

		float fb_width, fb_height;

		ImGuiContext* context;

	public:
		/* set up ImGui fonts and style after calling this */
		void initIMGUI();
		void findUpperBound(uint&, uint&);
		void initGL(uint, uint);
		/* use ImGui::GetIO() to set up mouse and keyboard inputs before calling this */
		void begin();
		void draw();
		void sync();

		

	};


}