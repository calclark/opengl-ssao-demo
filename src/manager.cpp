#include "manager.hpp"

#include <fmt/core.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>

// Use the anonymous namespace for applicable private constants/functions
namespace {
constexpr auto g_width = uint16_t{1024}; // window width
constexpr auto g_height = uint16_t{1024}; // window height

constexpr auto fov = glm::radians(45.0F); // field-of-view
auto projection = glm::infinitePerspective(fov, 1.0F, 1.0F);

// Handle debug messages coming from opengl
void GLAPIENTRY gl_message_callback(
    GLenum,
    GLenum,
    GLuint,
    GLenum severity,
    GLsizei,
    const GLchar* message,
    const void*) {
    fmt::print(stderr, "{}\n", message);
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        std::terminate();
    }
};

// Toggle opengl wireframe mode
void toggle_wireframe() {
    static GLenum mode = GL_FILL;
    mode = mode == GL_FILL ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, mode);
}

// Initialize the geometry shader
Shader geometry_shader() {
    auto sh =
        Shader{"shaders/geometry/vert.glsl", "shaders/geometry/frag.glsl"};

    sh.use();
    auto projection_loc = glGetUniformLocation(sh.id(), "u_projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

    auto diff_loc = glGetUniformLocation(sh.id(), "u_diffuse");
    glUniform1i(diff_loc, 0);

    auto norm_loc = glGetUniformLocation(sh.id(), "u_normal");
    glUniform1i(norm_loc, 1);

    auto spec_loc = glGetUniformLocation(sh.id(), "u_specular");
    glUniform1i(spec_loc, 2);

    return sh;
}

// Initialize the lighting shader.
Shader lighting_shader(bool enable_ssao) {
    auto sh =
        Shader{"shaders/lighting/vert.glsl", "shaders/lighting/frag.glsl"};

    sh.use();
    auto location = glGetUniformLocation(sh.id(), "u_diffuse_spec");
    glUniform1i(location, 0);

    location = glGetUniformLocation(sh.id(), "u_occlusion");
    glUniform1i(location, 1);

    location = glGetUniformLocation(sh.id(), "u_enable_ssao");
    glUniform1i(location, enable_ssao);
    return sh;
}

// Generate the position offsets that will be used to sample around each
// fragment in the ssao shader.
std::array<glm::vec3, 64> generate_sample_kernel() {
    auto generator = std::default_random_engine{std::random_device{}()};
    auto random_float = std::uniform_real_distribution<GLfloat>{0.0, 1.0};
    auto kernel = std::array<glm::vec3, 64>{};
    for (auto i = unsigned{0}; i < kernel.size(); i++) {
        auto sample = glm::vec3(
            random_float(generator) * 2.0 - 1.0,
            random_float(generator) * 2.0 - 1.0,
            random_float(generator));
        sample = glm::normalize(sample);
        sample *= random_float(generator);
        auto scale = i / 64.0F;
        scale = glm::mix(0.1F, 1.0F, scale * scale);
        sample *= scale;
        kernel[i] = sample;
    }
    return kernel;
};

// Initialize the ssao shader.
Shader ssao_shader() {
    auto sh =
        Shader{"shaders/lighting/vert.glsl", "shaders/ssao/depth-frag.glsl"};

    sh.use();
    auto location = glGetUniformLocation(sh.id(), "u_position");
    glUniform1i(location, 0);

    location = glGetUniformLocation(sh.id(), "u_normal");
    glUniform1i(location, 1);

    location = glGetUniformLocation(sh.id(), "u_noise");
    glUniform1i(location, 2);

    auto samples = generate_sample_kernel();
    for (auto i = size_t{0}; i < samples.size(); i++) {
        location = glGetUniformLocation(
            sh.id(),
            fmt::format("u_samples[{}]", i).c_str());
        glUniform3fv(location, 1, glm::value_ptr(samples[i]));
    }

    location = glGetUniformLocation(sh.id(), "u_projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));

    auto scale = glm::vec2{g_width / 4.0, g_height / 4.0};
    location = glGetUniformLocation(sh.id(), "u_noise_scale");
    glUniform2fv(location, 1, glm::value_ptr(scale));

    return sh;
}

// Initialize the blur shader.
Shader ssao_blur_shader() {
    auto sh =
        Shader{"shaders/lighting/vert.glsl", "shaders/ssao/blur-frag.glsl"};

    sh.use();
    auto location = glGetUniformLocation(sh.id(), "u_occlusion");
    glUniform1i(location, 0);

    return sh;
}

// Generate a screen-filling quad Vertex Array Object and return its opengl id.
GLuint generate_quad() {
    auto data = std::array<float, 20>{
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };

    auto vbo = GLuint{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(
        vbo,
        static_cast<GLsizei>(data.size() * sizeof(float)),
        data.data(),
        GL_DYNAMIC_STORAGE_BIT);

    auto vao = GLuint{};
    glCreateVertexArrays(1, &vao);

    constexpr auto binding_idx = 0;
    glVertexArrayVertexBuffer(vao, binding_idx, vbo, 0, sizeof(float) * 5);

    auto attrib_idx = 0; // positions
    glEnableVertexArrayAttrib(vao, attrib_idx);
    glVertexArrayAttribBinding(vao, attrib_idx, binding_idx);
    glVertexArrayAttribFormat(vao, attrib_idx, 3, GL_FLOAT, GL_FALSE, 0);

    attrib_idx = 1; // texture coordinates
    glEnableVertexArrayAttrib(vao, attrib_idx);
    glVertexArrayAttribBinding(vao, attrib_idx, binding_idx);
    glVertexArrayAttribFormat(
        vao,
        attrib_idx,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(float) * 3);

    glDeleteBuffers(1, &vbo);
    return vao;
}

// Create a texture of random noise for use in the ssao shader.
GLuint generate_noise() {
    auto generator = std::default_random_engine{std::random_device{}()};
    auto random_float = std::uniform_real_distribution<GLfloat>{0.0, 1.0};
    auto noise = std::vector<glm::vec3>{};
    for (auto i = 0; i < 16; i++) {
        auto sample = glm::vec3(
            random_float(generator) * 2.0 - 1.0,
            random_float(generator) * 2.0 - 1.0,
            0.0);
        noise.emplace_back(sample);
    }

    auto tex = GLuint{};
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(tex, 1, GL_RGBA32F, 4, 4);
    glTextureSubImage2D(tex, 0, 0, 0, 4, 4, GL_RGBA, GL_FLOAT, noise.data());

    return tex;
}

// Update the camera based on change in time and held keys.
void update_camera(Camera& camera, uint32_t delta_time) {
    auto step = delta_time / 100.0F;
    const auto* state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_W]) {
        camera.move_forward(step);
    } else if (state[SDL_SCANCODE_S]) {
        camera.move_backward(step);
    }

    if (state[SDL_SCANCODE_A]) {
        camera.move_left(step);
    } else if (state[SDL_SCANCODE_D]) {
        camera.move_right(step);
    }

    if (state[SDL_SCANCODE_SPACE]) {
        camera.move_up(step);
    } else if (state[SDL_SCANCODE_LSHIFT]) {
        camera.move_down(step);
    }

    step *= 10;
    if (state[SDL_SCANCODE_I]) {
        camera.rotate(step, 0.0F);
    } else if (state[SDL_SCANCODE_K]) {
        camera.rotate(-step, 0.0F);
    }

    if (state[SDL_SCANCODE_J]) {
        camera.rotate(0.0F, -step);
    } else if (state[SDL_SCANCODE_L]) {
        camera.rotate(0.0F, step);
    }
}
} // namespace

Manager::Manager() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::terminate();
    }

    constexpr auto gl_major_version = 4;
    constexpr auto gl_minor_version = 6;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_version);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_version);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    _window = SDL_CreateWindow(
        "rend",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        g_width,
        g_height,
        SDL_WINDOW_OPENGL);

    if (_window == nullptr) {
        std::terminate();
    }

    _context = SDL_GL_CreateContext(_window);
    if (_context == nullptr) {
        std::terminate();
    }

    if (gladLoadGLLoader(SDL_GL_GetProcAddress) == 0) {
        std::terminate();
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_message_callback, nullptr);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);

    glViewport(0, 0, g_width, g_height);
    _geometry_shader.emplace(geometry_shader());
    _ssao_shader.emplace(ssao_shader());
    _ssao_blur_shader.emplace(ssao_blur_shader());
    _lighting_shader.emplace(lighting_shader(_enable_ssao));
    construct_gbuffer();
    construct_ssao_buffers();
    _quad = generate_quad();
    _noise_tex = generate_noise();
}

void Manager::construct_gbuffer() {
    glCreateFramebuffers(1, &_gbuffer);

    glCreateTextures(GL_TEXTURE_2D, 1, &_gposition);
    glTextureParameteri(_gposition, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_gposition, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_gposition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(_gposition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(_gposition, 1, GL_RGBA16F, g_width, g_height);
    glNamedFramebufferTexture(_gbuffer, GL_COLOR_ATTACHMENT0, _gposition, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &_gnormal);
    glTextureParameteri(_gnormal, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_gnormal, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_gnormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(_gnormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(_gnormal, 1, GL_RGBA16F, g_width, g_height);
    glNamedFramebufferTexture(_gbuffer, GL_COLOR_ATTACHMENT1, _gnormal, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &_gdiffuse);
    glTextureParameteri(_gdiffuse, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_gdiffuse, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_gdiffuse, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(_gdiffuse, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(_gdiffuse, 1, GL_RGBA8, g_width, g_height);
    glNamedFramebufferTexture(_gbuffer, GL_COLOR_ATTACHMENT2, _gdiffuse, 0);

    auto buffers = std::array<GLenum, 3>{
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2};
    glNamedFramebufferDrawBuffers(_gbuffer, 3, buffers.data());

    auto render_buffer = GLuint{};
    glCreateRenderbuffers(1, &render_buffer);
    glNamedRenderbufferStorage(
        render_buffer,
        GL_DEPTH_COMPONENT,
        g_width,
        g_height);
    glNamedFramebufferRenderbuffer(
        _gbuffer,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        render_buffer);
}

void Manager::construct_ssao_buffers() {
    auto tex = GLuint{};
    glCreateFramebuffers(1, &_ssao_buffer);
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(tex, 1, GL_R16F, g_width, g_height);
    glNamedFramebufferTexture(_ssao_buffer, GL_COLOR_ATTACHMENT0, tex, 0);
    _ssao_color_tex = tex;

    glCreateFramebuffers(1, &_ssao_blur_buffer);
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureStorage2D(tex, 1, GL_R16F, g_width, g_height);
    glNamedFramebufferTexture(_ssao_blur_buffer, GL_COLOR_ATTACHMENT0, tex, 0);
    _ssao_color_blur_tex = tex;
}

Manager::~Manager() {
    SDL_GL_DeleteContext(_context);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

void Manager::render() {
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_scene_idx) {
        // PASS 1: Fill the G-buffer
        _geometry_shader->use();
        auto scale = 1.0F / 1.0F;
        auto model = glm::scale(glm::mat4{1.0}, glm::vec3{scale});
        auto model_loc =
            glGetUniformLocation(_geometry_shader->id(), "u_model");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));

        auto view = _camera.transform();
        auto view_loc = glGetUniformLocation(_geometry_shader->id(), "u_view");
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

        glBindFramebuffer(GL_FRAMEBUFFER, _gbuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _scenes[*_scene_idx].render();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // PASS 2: Generate the ssao texture
        glBindFramebuffer(GL_FRAMEBUFFER, _ssao_buffer);
        glClear(GL_COLOR_BUFFER_BIT);
        _ssao_shader->use();
        glBindTextureUnit(0, _gposition);
        glBindTextureUnit(1, _gnormal);
        glBindTextureUnit(2, _noise_tex);
        draw_quad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // PASS 3: Blur the ssao texture
        glBindFramebuffer(GL_FRAMEBUFFER, _ssao_blur_buffer);
        glClear(GL_COLOR_BUFFER_BIT);
        _ssao_blur_shader->use();
        glBindTextureUnit(0, _ssao_color_tex);
        draw_quad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // PASS 4: Calculate the final lighting and output to screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _lighting_shader->use();
        glBindTextureUnit(0, _gdiffuse);
        glBindTextureUnit(1, _ssao_color_blur_tex);
        draw_quad();
    }

    SDL_GL_SwapWindow(_window);
}

void Manager::toggle_ssao() {
    _enable_ssao = !_enable_ssao;
    _lighting_shader->use();
    auto location =
        glGetUniformLocation(_lighting_shader->id(), "u_enable_ssao");
    glUniform1i(location, _enable_ssao);
}

void Manager::draw_quad() {
    glBindVertexArray(_quad);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

Manager& Manager::instance() {
    static auto m = Manager{};
    return m;
}

void Manager::loop() {
    auto last_frame = 0U;
    auto quit = false;
    auto event = SDL_Event{};

    while (!quit) {
        auto curr_frame = SDL_GetTicks();
        while (SDL_PollEvent(&event) != 0) {
            quit |= handle_event(event);
        }

        update_camera(_camera, curr_frame - last_frame);
        render();

        last_frame = curr_frame;
    }
}

void Manager::add_scene(Scene m) {
    _scenes.emplace_back(std::move(m));
    if (!_scene_idx) {
        _scene_idx.emplace(0);
    }
}

bool Manager::handle_event(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        return true;
    }
    if (event.type == SDL_KEYDOWN) {
        auto keycode = event.key.keysym.sym;
        switch (keycode) {
        case SDLK_q:
            return true;
        case SDLK_f:
            toggle_wireframe();
            break;
        case SDLK_e:
            toggle_ssao();
            break;
        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_4:
        case SDLK_5:
        case SDLK_6:
        case SDLK_7:
        case SDLK_8:
        case SDLK_9:
            auto idx =
                static_cast<size_t>(std::stoi(SDL_GetKeyName(keycode)) - 1);
            if (idx < _scenes.size()) {
                _scene_idx.emplace(idx);
            }
        }
    }
    return false;
}
