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

add_executable(turbogui 
	src/public/main.cpp
	src/public/imstb_truetype.h
	src/public/imstb_textedit.h
	src/public/imstb_rectpack.h
	src/public/imgui_internal.h
	src/public/imgui.h
	src/public/imconfig.h
	src/public/gui.h
	src/public/defines.h

	src/private/imgui.cpp
	src/private/imgui_demo.cpp
	src/private/imgui_draw.cpp
	src/private/imgui_widgets.cpp
	src/private/gui.cpp
)
# ------------------ GLM ------------------
find_package (glm 0.9.9.9 QUIET)
if(glm_FOUND)
	message(STATUS "GLM found!")
else()
	message(STATUS "GLM not found! Fetching...")
	FetchContent_Declare(
			glm
			GIT_REPOSITORY https://github.com/g-truc/glm
	)
	FetchContent_GetProperties(glm)
	if(NOT glm_POPULATED)
		FetchContent_Populate(glm)
		add_subdirectory("${glm_SOURCE_DIR}" "${glm_BINARY_DIR}")
	endif()

	FetchContent_Declare(
			glfw
			GIT_REPOSITORY https://github.com/glfw/glfw
	)
endif ()

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
	if(NOT glfw_POPULATED)
	    FetchContent_Populate(glfw)

	    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs" FORCE)
	    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs" FORCE)
	    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation" FORCE)
	    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target" FORCE)

	    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
	endif()
endif()

# ------------------ GLAD ------------------
find_package (glad 0.1.33 QUIET)
if(glad_FOUND)
	message(STATUS "GLAD found!")
else()
	message(STATUS "GLAD not found! Fetching...")
	set(GLAD_API "gl=4.3" CACHE STRING "API type/version pairs, like \"gl=4.3,gles=\", no version means latest")
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


target_link_libraries(turbogui
	glm
	glfw
	glad
)

set_property(TARGET turbogui PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

