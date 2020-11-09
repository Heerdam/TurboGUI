#ifndef TB_GUI_H
#define TB_GUI_H

#include <vector>
#include <utility>
#include <exception>
#include <string>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cstring>

#include <glad/glad.h>

#include <imgui.h>

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
		uint timeOutSync = static_cast<uint>(5e6);
		uint syncTime = 0;

		ImGuiContext* context;

	public:
		GUI() { context = ImGui::CreateContext(); }
		~GUI();
		void initGL(uint, uint);
		/* use ImGui::GetIO() to set up mouse and keyboard inputs before calling this */
		void begin();
		void draw();
		void sync();

		//draws a window with stats
		void drawStatsWindow(uint = 0);
		//inserts the stats in an already existing window
		void drawStats();

		//sets the timeout for the syncinc process in ns. setting it too low at own perill. 
		void setSyncTimeOut(uint _time) {
			timeOutSync = _time;
		}

		uint getIdxCount() { return idx; }
		uint getVertCount() { return vert; }
		uint getSyncTime() { return syncTime; }
		float getDrawTime() { return drawTime; }
		float getMeanDrawTime() { return meanTime; }

		ImGuiContext* getContext() { return context; }
	};

}

TurboGUI::GUI::~GUI() {
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(2, EBO);
    glDeleteProgram(shader);
    glDeleteTextures(1, &tex);
    glDeleteSync(syncObj[0]);
    glDeleteSync(syncObj[1]);
}

void TurboGUI::GUI::initGL(uint _vbo_upper_bound, uint _ebo_upper_bound) {
 
    std::memset(drawTimeMean.data(), 0, drawTimeMean.size() * sizeof(float));

    {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }

    idxBound = _ebo_upper_bound;
    vertBound = _vbo_upper_bound;

    //map buffers
    glGenVertexArrays(2, VAO);

    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    for (uint i = 0; i < 2; ++i) {
        glBindVertexArray(VAO[i]);

        //vbo
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferStorage(GL_ARRAY_BUFFER, _vbo_upper_bound * (uint)sizeof(ImDrawVert), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_CLIENT_STORAGE_BIT);
        VBO_ptr[i] = reinterpret_cast<float*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, _vbo_upper_bound * (uint)sizeof(ImDrawVert), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

        //pos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        glEnableVertexAttribArray(0);

        //uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        glEnableVertexAttribArray(1);

        //col
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
        glEnableVertexAttribArray(2);

        //ebo
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, _ebo_upper_bound * (uint)sizeof(ImDrawIdx), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_CLIENT_STORAGE_BIT);
        EBO_ptr[i] = reinterpret_cast<ushort*>(glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, _ebo_upper_bound * (uint)sizeof(ImDrawIdx), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }   

    {
        const GLchar* vertex_shader =
            "#version 430 core\n"
            "layout (location = 0) in vec2 Position;\n"
            "layout (location = 1) in vec2 UV;\n"
            "layout (location = 2) in vec4 Color;\n"
            "layout (location = 3) uniform mat4 ProjMtx;\n"
            "out vec2 Frag_UV;\n"
            "out vec4 Frag_Color;\n"
            "void main()\n"
            "{\n"
            "    Frag_UV = UV;\n"
            "    Frag_Color = Color;\n"
            "    gl_Position = ProjMtx * vec4(Position.xy,0.f,1.f);\n"
            "}\n";

        const GLchar* fragment_shader =
            "#version 430 core\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "layout (location = 4) uniform sampler2D Texture;\n"
            "out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.xy);\n"
            "}\n";

        //Compile Vertex
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_shader, nullptr);
        glCompileShader(vertex);
        GLint isCompiled = 0;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(vertex, maxLength, &maxLength, &errorLog[0]);
            glDeleteShader(vertex);
            throw TurboGuiException(std::string(errorLog.data()));
        }

        //Compile Frag
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag, 1, &fragment_shader, nullptr);
        glCompileShader(frag);
        isCompiled = 0;
        glGetShaderiv(frag, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(frag, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(frag, maxLength, &maxLength, &errorLog[0]);
            glDeleteShader(frag);
            throw TurboGuiException(std::string(errorLog.data()));
        }

        //Link
        shader = glCreateProgram();
        glAttachShader(shader, vertex);
        glAttachShader(shader, frag);

        glLinkProgram(shader);

        GLint isLinked = 0;
        glGetProgramiv(shader, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> errorLog(maxLength);
            glGetProgramInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
            glDeleteShader(vertex);
            glDeleteShader(frag);
            if (shader != -1) glDeleteProgram(shader);
            throw TurboGuiException(std::string(errorLog.data()));
        }

        glDetachShader(shader, vertex);
        glDetachShader(shader, frag);
    }

    {
        //syncObj[0] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        syncObj[1] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
 
}

void TurboGUI::GUI::begin() {
    time = std::chrono::high_resolution_clock::now();
    ImGui::SetCurrentContext(context);
    ImGui::NewFrame();   
}

void TurboGUI::GUI::draw() {

    ImGui::Render();

	auto draw_data = ImGui::GetDrawData();

    const ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    std::vector<std::tuple<const ImDrawCmd*, uint, uint>> cmdCache;

    //pre-run
    {
        uint old_v_offset = 0;
        uint old_i_offset = 0;
        uint idx_offset = 0;
        uint v_offset = 0;

        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            std::memcpy(VBO_ptr[currIndex] + v_offset*5, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * (uint)sizeof(ImDrawVert));
            std::memcpy(EBO_ptr[currIndex] + idx_offset, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * (uint)sizeof(ImDrawIdx));

            v_offset += cmd_list->VtxBuffer.Size;
            idx_offset += cmd_list->IdxBuffer.Size;

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                auto cmd = &cmd_list->CmdBuffer[cmd_i];
                cmdCache.push_back({ cmd, old_v_offset, old_i_offset });
            }
            old_v_offset = v_offset;
            old_i_offset = idx_offset;
        }

        idx = idx_offset;
        vert = v_offset;

        maxIdx = std::max(idx, maxIdx);
        maxVert = std::max(vert, maxVert);
    }

    //draw
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
        GLenum current_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin);
        if (current_clip_origin == GL_UPPER_LEFT)
            clip_origin_lower_left = false;
#endif

        glUseProgram(shader);
        glBindVertexArray(VAO[currIndex]);

        uint fb_width = static_cast<uint>(draw_data->DisplaySize.x);
        uint fb_height = static_cast<uint>(draw_data->DisplaySize.y);

        glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
        const float L = draw_data->DisplayPos.x;
        const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        const float T = draw_data->DisplayPos.y;
        const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const float ortho_projection[4][4] =
        {
            { 2.f / (R - L),        0.f,                0.0f,       0.0f },
            { 0.f,                  2.f / (T - B),      0.0f,       0.0f },
            { 0.f,                  0.f,                -1.0f,      0.0f },
            { (R + L) / (L - R),    (T + B) / (B - T),  0.0f,       1.0f },
        };

        glUniform1i(4, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniformMatrix4fv(3, 1, GL_FALSE, &ortho_projection[0][0]);

        for (size_t i = 0; i < cmdCache.size(); ++i) {
            
            const auto cmd = std::get<0>(cmdCache[i]);

            ImVec4 clip_rect;
            clip_rect.x = (cmd->ClipRect.x - clip_off.x) * clip_scale.x;
            clip_rect.y = (cmd->ClipRect.y - clip_off.y) * clip_scale.y;
            clip_rect.z = (cmd->ClipRect.z - clip_off.x) * clip_scale.x;
            clip_rect.w = (cmd->ClipRect.w - clip_off.y) * clip_scale.y;
            if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.f && clip_rect.w >= 0.f) {
                glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
                const uint voffset = (GLint)std::get<1>(cmdCache[i]);
                const uint ioffset = (GLint)std::get<2>(cmdCache[i]);
                glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)cmd->ElemCount, GL_UNSIGNED_SHORT, (void*)(intptr_t)((cmd->IdxOffset + ioffset) * sizeof(ImDrawIdx)), voffset);
                //glDrawElements(GL_TRIANGLES, (GLsizei)cmd->ElemCount, GL_UNSIGNED_SHORT, (void*)(intptr_t)((cmd->IdxOffset + ioffset) * sizeof(ImDrawIdx)));
            }

        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glUseProgram(0);       

        glScissor(0, 0, (GLsizei)draw_data->DisplaySize.x, (GLsizei)draw_data->DisplaySize.y);
    }
    {
        auto deltaT = std::chrono::high_resolution_clock::now() - time;
        long long ns = deltaT.count();
        drawTime = 1.f / static_cast<float>(1e6) * static_cast<float>(ns);
        drawMeanTimeIndex = (++drawMeanTimeIndex) % drawTimeMean.size();
        drawTimeMean[drawMeanTimeIndex] = drawTime;
        meanTime = std::accumulate(drawTimeMean.begin(), drawTimeMean.end(), 0.f);
        meanTime /= drawTimeMean.size();
    }
}

void TurboGUI::GUI::sync() {
    uint syncI = (currIndex + 1) % 2;
    auto t = std::chrono::high_resolution_clock::now();
    glClientWaitSync(syncObj[syncI], GL_SYNC_FLUSH_COMMANDS_BIT, timeOutSync);
    syncTime = static_cast<uint>((std::chrono::high_resolution_clock::now() - t).count());
    glDeleteSync(syncObj[syncI]);
    syncObj[currIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    currIndex = syncI;
}

void TurboGUI::GUI::drawStatsWindow(uint _fps) {
    maxFps = std::max(maxFps, _fps);
    bool open = true;

    ImGui::Begin("TurboGUI stats", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(1.75f);

    //fps
    if( _fps != 0)
        ImGui::Text("fps: %i [%i]", _fps, maxFps);
    //draw time
    ImGui::Text("time: %.3fms [%.3fms]", drawTime, meanTime);
    //vertices
    ImGui::Text("vert: %i [%i] [%i]", vert, maxVert, vertBound);
    //indices
    ImGui::Text("idx: %i [%i] [%i]", idx, maxIdx, idxBound);
    //timeout
    ImGui::Text("sync: %i [%i] [%i]", syncTime, timeOutSync, timeOutSync - syncTime);

    ImGui::End();
}

void TurboGUI::GUI::drawStats() {
    //draw time
    ImGui::Text("time: %.3fms [%.3fms]", drawTime, meanTime);
    //vertices
    ImGui::Text("vert: %i [%i] [%i]", vert, maxVert, vertBound);
    //indices
    ImGui::Text("idx: %i [%i] [%i]", idx, maxIdx, idxBound);
    //timeout
    ImGui::Text("sync: %i [%i] [%i]", syncTime, timeOutSync, timeOutSync - syncTime);
}


#endif TB_GUI_H