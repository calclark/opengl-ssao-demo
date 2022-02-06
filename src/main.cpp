#include "loader.hpp"
#include "manager.hpp"

#include <cstdlib>

/*
 * SCREEN SPACE AMBIENT OCCLUSION (SSAO)
 *
 * Usage:
 *  ./project [FILE.obj]*
 * 
 *  Loads the provided sponza model by default.
 *  Additional wavefront .obj scenes can be added via command line arguments.
 *  Beware that the SSAO constants are optimized for the size of the sponza model, and may result in poor looking scenes with differently sized models.
 *  Those constants can be changed in shaders/ssao/depth-frag.glsl.
 *  Scene switching is supported via the hotkeys listed below.
 *
 * Controls:
 *  - W: Move camera forward
 *  - A: Move camera left
 *  - S: Move camera backward
 *  - D: Move camera right
 *  - Space: Move camera up
 *  - LShift: Move camera down
 *  - I: Pitch camera up
 *  - J: Yaw camera left
 *  - K: Pitch camera down
 *  - L: Yaw camera right
 *  - Q: Quit
 *  - E: Toggle SSAO
 *  - F: Toggle wireframe mode
 *  - 1-9: Switch scene
 *
 * Libraries used:
 *  - glm
 *  - glad
 *  - tinyobjloader
 *  - stb-image
 *  - fmt
 */
int main(int argc, char* argv[]) {
    auto& manager = Manager::instance();
    manager.add_scene(Loader::load_obj("sponza/sponza.obj"));
    auto args = std::span(argv + 1, static_cast<size_t>(argc - 1));
    for (auto& obj_file : args) {
        manager.add_scene(Loader::load_obj(obj_file));
    }
    manager.loop();
    return EXIT_SUCCESS;
}
