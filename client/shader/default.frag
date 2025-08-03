#version 410 core

in vec2 f_tex;

out vec4 final_color;

uniform sampler2D background;

void main()
{
    final_color = texture(background, f_tex);
}