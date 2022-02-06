#version 460 core
layout (location = 0) in vec3 b_position;
layout (location = 1) in vec3 b_normal;
layout (location = 2) in vec2 b_tex_coords;
layout (location = 3) in vec3 b_tex_tangent;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 v_tex_coords;
out vec3 v_frag_pos;
out mat3 v_tbn;

void main()
{
    vec4 view_position = u_view * u_model * vec4(b_position, 1.0);
    v_tex_coords = b_tex_coords;
    v_frag_pos = view_position.xyz; // pass position in view coordinates

    vec3 t = normalize(vec3(u_model * vec4(b_tex_tangent, 0.0)));
    vec3 n = normalize(vec3(u_model * vec4(b_normal, 0.0)));
    t = normalize(t - dot(t, n) * n);
    vec3 b = cross(n, t);
    v_tbn = mat3(u_view) * mat3(t, b, n); // have tbn matrix convert to view space

    gl_Position = u_projection * view_position;
}
