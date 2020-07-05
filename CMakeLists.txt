cmake_minimum_required(VERSION 3.11)

project(TurboGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb" )
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG -flto")
endif()

include(FetchContent)

SET(HEADER
	src/public/imstb_truetype.h
	src/public/imstb_textedit.h
	src/public/imstb_rectpack.h
	src/public/imgui_internal.h
	src/public/imgui.h
	src/public/imconfig.h
	src/public/gui.h
	src/public/defines.h
)

add_library(turbogui STATIC
	"${HEADER}"
	src/private/imgui.cpp
	src/private/imgui_demo.cpp
	src/private/imgui_draw.cpp
	src/private/imgui_widgets.cpp
	src/private/gui.cpp
)

target_include_directories(turbogui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/public)
set_target_properties(turbogui PROPERTIES VERSION 0.1)

# ------------------ GLAD ------------------
find_package (glad 0.1.33 QUIET)
if(glad_FOUND)
	message(STATUS "GLAD found!")
else()
	message(STATUS "GLAD not found! Fetching...")
	set(GLAD_API "gl=4.3" CACHE STRING "API type/version pairs, like \"gl=4.3,gles=\", no version means latest")
	set(GLAD_EXTENSIONS "GL_ARB_buffer_storage" CACHE STRING "Path to extensions file or comma separated list of extensions, if missing all extensions are included")
	FetchContent_Declare(
		glad
		GIT_REPOSITORY https://github.com/Dav1dde/glad.git
		GIT_TAG        v0.1.33
	)
	FetchContent_GetProperties(glad)
	if(NOT glad_POPULATED)
		FetchContent_Populate(glad)
		add_subdirectory("${glad_SOURCE_DIR}" "${glad_BINARY_DIR}")
	endif()
endif()

target_link_libraries(turbogui PUBLIC
	glad
	dl
)
