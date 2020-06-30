
#include "defines.h"

namespace TurboGUI {

	class GUI {

		GLuint VAO, VBO, EBO;

		float* VBO_ptr[2];
		uint* EBO_ptr[2];

		uint currIndex = 0;

	public:
		void init();
		void draw();

	};

}