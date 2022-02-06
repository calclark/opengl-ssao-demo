#pragma once

#include "texture.hpp"

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <span>

// A vertex is a point in a mesh - along with its relevant data.
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coord;
    glm::vec3 tex_tangent;
};

// A mesh is a piece of geometry.
class Mesh {
  public:
    // Create a mesh from vertices, face indices, and textures.
    Mesh(std::span<Vertex>, std::span<glm::uvec3>, TextureGroup);

    // Allow moves but disallow copies
    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) noexcept;
    ~Mesh();

    // Render this geometry.
    void draw() const;

  private:
    TextureGroup _texture{}; // textures associated with this mesh
    GLuint _vao; // vertex array object id
    GLsizei _vert_count{}; // number of elements to this mesh

    // Swap the contents of two meshes.
    static void _swap(Mesh&, Mesh&);
};
