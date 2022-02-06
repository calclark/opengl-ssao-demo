#version 460 core
uniform sampler2D u_occlusion;

in vec2 v_tex_coords;

out float v_frag_color;

void main()
{
    vec2 texel_size = 1.0 / vec2(textureSize(u_occlusion, 0)); 
    float sum = 0.0;
    for (int x = -2; x < 2; x++) {
        for (int y = -2; y < 2; y++) {
            vec2 offset = vec2(x, y) * texel_size;
            sum += texture(u_occlusion, v_tex_coords + offset).r;
        }
    }

    v_frag_color = sum / 16.0;
}
