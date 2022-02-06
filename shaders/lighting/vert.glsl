#version 460 core
layout (location = 0) in vec3 b_position;
layout (location = 1) in vec2 b_tex_coords;

out vec2 v_tex_coords;

void main()
{
    v_tex_coords = b_tex_coords;
    gl_Position = vec4(b_position, 1.0);
}
