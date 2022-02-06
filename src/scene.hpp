#pragma once

#include "mesh.hpp"
#include "texture.hpp"

#include <vector>

// A scene is a collection of meshes and textures in those meshes
class Scene {
  public:
    // Create a scene from lists of textures and meshes.
    Scene(std::vector<Texture>, std::vector<Mesh>);

    // Render the scene.
    void render();

  private:
    std::vector<Texture> _textures;
    std::vector<Mesh> _meshes;
};
