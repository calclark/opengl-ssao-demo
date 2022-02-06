#version 460 core
uniform sampler2D u_diffuse_spec;
uniform sampler2D u_occlusion;

uniform bool u_enable_ssao;

in vec2 v_tex_coords;

out vec4 v_frag_color;

void main()
{
    vec3 diffuse_color = texture(u_diffuse_spec, v_tex_coords).rgb;
    float occlusion = 1.0;
    if (u_enable_ssao) {
        occlusion = texture(u_occlusion, v_tex_coords).r;
    }
    vec3 ambient = diffuse_color * occlusion;
    v_frag_color = vec4(ambient, 1.0);
}
