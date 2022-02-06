#version 460 core
layout (location = 0) out vec3 b_position;
layout (location = 1) out vec3 b_normal;
layout (location = 2) out vec4 b_diffuse_spec;

uniform sampler2D u_diffuse;
uniform sampler2D u_normal;
uniform sampler2D u_specular;

in vec2 v_tex_coords;
in vec3 v_frag_pos;
in mat3 v_tbn;

void main()
{
    // write position texture
    b_position = v_frag_pos;

    // write normal texture
    vec3 normal = texture(u_normal, v_tex_coords).rgb;
    normal = normal * 2.0 - 1.0;
    b_normal = normalize(v_tbn * normal);

    // write color texture
    b_diffuse_spec.rgb = texture(u_diffuse, v_tex_coords).rgb;
    b_diffuse_spec.a = texture(u_specular, v_tex_coords).r;
}
