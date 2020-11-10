cmake_minimum_required(VERSION 3.11)

project(TurboGUIExample)

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

add_executable(tbgex
	src/main.cpp
)

set_target_properties(tbgex PROPERTIES LINKER_LANGUAGE CXX)

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
	
	target_include_directories(tbgex PUBLIC "${imgui_SOURCE_DIR}")
endif()

# ------------------ TURBOGUI ------------------
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." tbg)

# ------------------ GLFW ------------------
find_package (glfw3 3.3.2 QUIET)
if(glfw3_FOUND)
	message(STATUS "GLFW found!")
else()
	message(STATUS "GLFW not found! Fetching...")
	FetchContent_Declare(
			glfw
			GIT_REPOSITORY https://github.com/glfw/glfw
	)

	FetchContent_GetProperties(glfw)

	set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs" FORCE)
	set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation" FORCE)
	set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target" FORCE)

	FetchContent_MakeAvailable(glfw)
endif()

target_link_libraries(tbgex
	imgui
	glfw
	turbogui
)

