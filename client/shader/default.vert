#version 410 core

layout(location = 0) in vec2 v_pos;
layout(location = 1) in vec2 v_tex;

out vec2 f_tex;

void main()
{
    gl_Position = vec4(v_pos, 0.0, 1.0);
    f_tex = v_tex;
}