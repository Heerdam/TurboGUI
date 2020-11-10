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
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
endif()

include(FetchContent)

add_library(turbogui STATIC
	include/tb_gui.h
)

set_target_properties(turbogui PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(turbogui PROPERTIES VERSION 0.1)

target_include_directories(turbogui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/public)

# ------------------ GLAD ------------------
set(GLAD_API "gl=4.3" CACHE STRING "API type/version pairs, like \"gl=4.3,gles=\", no version means latest")
set(GLAD_EXTENSIONS "GL_ARB_buffer_storage" CACHE STRING "Path to extensions file or comma separated list of extensions, if missing all extensions are included")
find_package (glad 0.1.33 QUIET)
if(glad_FOUND)
	message(STATUS "GLAD found!")
else()
	message(STATUS "GLAD not found! Fetching...")
	FetchContent_Declare(
		glad
		GIT_REPOSITORY https://github.com/Dav1dde/glad.git
		GIT_TAG        v0.1.33
	)
	FetchContent_MakeAvailable(glad)
endif()

# ------------------ IMGUI ------------------
message(STATUS "Fetching imgui...")
FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
)
FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
	FetchContent_Populate(imgui)

	message(STATUS "Preparing imgui...")

	set(IMGUI_FILES
		"${imgui_SOURCE_DIR}/imgui_demo.cpp"
		"${imgui_SOURCE_DIR}/imgui_draw.cpp"
		"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
		"${imgui_SOURCE_DIR}/imgui.cpp"
	)

	add_library(imgui "${IMGUI_FILES}")
	
	target_include_directories(turbogui PUBLIC "${imgui_SOURCE_DIR}")
endif()

if (MSVC)
	target_link_libraries(turbogui PUBLIC
		glad
		imgui
	)
else()
	target_link_libraries(turbogui PUBLIC
		glad
		imgui
		dl
	)
endif()
