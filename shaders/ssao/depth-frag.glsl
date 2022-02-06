#version 460 core
uniform sampler2D u_position;
uniform sampler2D u_normal;
uniform sampler2D u_noise;

uniform vec3 u_samples[64];
uniform mat4 u_projection;
uniform vec2 u_noise_scale;

in vec2 v_tex_coords;

out float v_frag_color;

// Adjust scale based on size of scene.
// Default: sponza is big
const float c_scale = 1000;
const float c_radius = 0.5 * c_scale;
const float c_bias = 0.025 * c_scale;

void main()
{
    vec3 frag_pos = texture(u_position, v_tex_coords).xyz;
    vec3 normal = normalize(texture(u_normal, v_tex_coords).rgb);
    vec3 random = normalize(texture(u_noise, v_tex_coords * u_noise_scale).xyz);

    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < 64; i++) {
        vec3 sample_pos = tbn * u_samples[i];
        sample_pos = frag_pos + sample_pos * c_radius;

        vec4 offset = vec4(sample_pos, 1.0);
        offset = u_projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sample_depth = texture(u_position, offset.xy).z;

        float boundary = smoothstep(0.0, 1.0, c_radius / abs(frag_pos.z - sample_depth));
        occlusion += (sample_depth >= sample_pos.z + c_bias ? 1.0 : 0.0) * boundary;
    }
    occlusion = 1.0 - (occlusion / 64.0);
    v_frag_color = occlusion;
}
