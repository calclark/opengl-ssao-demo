#include "scene.hpp"

Scene::Scene(std::vector<Texture> textures, std::vector<Mesh> meshes)
    : _textures{std::move(textures)}, _meshes{std::move(meshes)} {
}

void Scene::render() {
    for (auto& mesh : _meshes) {
        mesh.draw();
    }
}
