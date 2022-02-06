#pragma once

#include "scene.hpp"

#include <filesystem>

namespace Loader {
// Load a wavefront .obj file to a scene.
Scene load_obj(const std::filesystem::path& path);
} // namespace Loader
