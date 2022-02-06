#pragma once

#include "camera.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include "shader.hpp"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include <optional>

// The manager is a program controller singleton.
class Manager {
  public:
    // Get a reference to the singleton.
    static Manager& instance();

    // Disallow copies and moves.
    Manager(Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&) = delete;
    Manager& operator=(Manager&&) = delete;

    // Enter the main control loop.
    void loop();

    // Add a scene to the list of scenes to render.
    void add_scene(Scene);

  private:
    SDL_Window* _window;
    SDL_GLContext _context;

    Camera _camera;
    std::vector<Scene> _scenes; // list of scenes to render
    std::optional<size_t> _scene_idx; // index of currently rendering scene

    GLuint _quad; // vertex array object id for screen quad (passes 2-4)
    GLuint _noise_tex; // texture id for random noise

    std::optional<Shader> _geometry_shader; // geometry pass shader
    GLuint _gbuffer; // framebuffer id for geometry pass
    GLuint _gposition; // texture id for g-buffer position data
    GLuint _gnormal; // texture id g-buffer normal data
    GLuint _gdiffuse; // texture id g-buffer color data

    std::optional<Shader> _ssao_shader; // ssao pass shader
    GLuint _ssao_buffer; // framebuffer id for ssao pass
    GLuint _ssao_color_tex; // texture id for ssao pass output

    std::optional<Shader> _ssao_blur_shader; // blur pass shader
    GLuint _ssao_blur_buffer; // framebuffer id for blur pass
    GLuint _ssao_color_blur_tex; // texture id for blur pass output

    std::optional<Shader> _lighting_shader; // final pass shader
    bool _enable_ssao = true;

    // Keep constructor/destructor private for singletons.
    Manager();
    ~Manager();

    // Run the full rendering pipeline (all passes).
    void render();

    // Draw the screen-filling quad.
    void draw_quad();

    // Toggle ssao appearance in output.
    void toggle_ssao();

    // Handle certain SDL events for user input.
    bool handle_event(const SDL_Event& event);

    // Create the g-buffer framebuffer and all associated textures.
    void construct_gbuffer();

    // Create the ssao framebuffers and all associated textures.
    void construct_ssao_buffers();
};
