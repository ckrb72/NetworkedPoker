#version 410 core

in vec2 f_tex;

out vec4 final_color;

uniform sampler2D background;

void main()
{
    float sdf_val = texture(background, f_tex).r;
    if(sdf_val < 0.5) {
        discard;
    }

    final_color = vec4(1.0);
}