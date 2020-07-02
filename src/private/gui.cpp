
#include "../public/gui.h"

void TurboGUI::GUI::initIMGUI() {
    context = ImGui::CreateContext();
}

void TurboGUI::GUI::findUpperBound(uint& _vbo_upper_bound, uint& _ebo_upper_bound) {

    ImGui::Render();

    auto draw_data = ImGui::GetDrawData();

    uint idx_offset = 0;
    uint v_offset = 0;

    for (uint n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        v_offset += cmd_list->VtxBuffer.Size;
        idx_offset += cmd_list->IdxBuffer.Size;
    }

    _vbo_upper_bound = v_offset;
    _ebo_upper_bound = idx_offset;
}

void TurboGUI::GUI::initGL(uint _vbo_upper_bound, uint _ebo_upper_bound) {
    //map buffers
    glGenVertexArrays(2, VAO);

    for (uint i = 0; i < 2; ++i) {
        glBindVertexArray(VAO[i]);

        //vbo
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferStorage(GL_ARRAY_BUFFER, _vbo_upper_bound, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_CLIENT_STORAGE_BIT);
        VBO_ptr[i] = reinterpret_cast<float*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, _vbo_upper_bound, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT));

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
        glBindBuffer(GL_ARRAY_BUFFER, EBO[i]);
        glBufferStorage(GL_ARRAY_BUFFER, _ebo_upper_bound, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_CLIENT_STORAGE_BIT);
        EBO_ptr[i] = reinterpret_cast<ushort*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, _ebo_upper_bound, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }   

    {
        const GLchar* vertex_shader =
            "#version 460 core\n"
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
            "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
            "}\n";

        const GLchar* fragment_shader =
            "#version 460 core\n"
            "in vec2 Frag_UV;\n"
            "in vec4 Frag_Color;\n"
            "layout (location = 4) uniform sampler2D Texture;\n"
            "layout (location = 0) out vec4 Out_Color;\n"
            "void main()\n"
            "{\n"
            "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
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
    ImGui::SetCurrentContext(context);
    ImGui::NewFrame();
}

void TurboGUI::GUI::draw() {

    ImGui::Render();

	auto draw_data = ImGui::GetDrawData();

    const ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    std::vector<uint> offsets;
    std::vector<const ImDrawCmd*> cmdCache;

    //pre-run
    {
        uint idx_offset = 0;
        uint v_offset = 0;

        for (uint n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];

            std::memcpy(VBO_ptr[currIndex] + v_offset, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * (uint)sizeof(ImDrawVert));
            std::memcpy(EBO_ptr[currIndex] + idx_offset, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * (uint)sizeof(ImDrawIdx));

            offsets.push_back(idx_offset);

            v_offset += cmd_list->VtxBuffer.Size;
            idx_offset += cmd_list->IdxBuffer.Size;

            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                cmdCache.push_back(&cmd_list->CmdBuffer[cmd_i]);
        }
    }

    //draw
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);

        glUseProgram(shader);
        glBindVertexArray(currIndex);

        uint fb_width = draw_data->DisplaySize.x;
        uint fb_height = draw_data->DisplaySize.y;

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
        glUniformMatrix4fv(3, 1, GL_FALSE, &ortho_projection[0][0]);

        for (size_t i = 0; i < cmdCache.size(); ++i) {
            const auto cmd = cmdCache[i];

            ImVec4 clip_rect;
            clip_rect.x = (cmd->ClipRect.x - clip_off.x) * clip_scale.x;
            clip_rect.y = (cmd->ClipRect.y - clip_off.y) * clip_scale.y;
            clip_rect.z = (cmd->ClipRect.z - clip_off.x) * clip_scale.x;
            clip_rect.w = (cmd->ClipRect.w - clip_off.y) * clip_scale.y;

            if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.f && clip_rect.w >= 0.f) {
                glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)cmd->TextureId);
                glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)cmd->ElemCount, GL_UNSIGNED_SHORT, (void*)(intptr_t)(cmd->IdxOffset * sizeof(ImDrawIdx)), offsets[i]);
            }

        }
        glBindVertexArray(0);
        glUseProgram(0);
    }

}

void TurboGUI::GUI::sync() {
    uint syncI = (++currIndex) % 2;
    glClientWaitSync(syncObj[syncI], GL_SYNC_FLUSH_COMMANDS_BIT, 5e6);
    glDeleteSync(syncObj[syncI]);
    syncObj[currIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    currIndex = syncI;
}
