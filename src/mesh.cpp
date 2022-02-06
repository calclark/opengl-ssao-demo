#include "mesh.hpp"

Mesh::Mesh(
    std::span<Vertex> vertices,
    std::span<glm::uvec3> indices,
    TextureGroup texture)
    : _texture{texture}, _vao{0}, _vert_count{static_cast<GLsizei>(
                                      indices.size_bytes())} {
    auto vbo = GLuint{};
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(
        vbo,
        static_cast<GLsizei>(vertices.size_bytes()),
        vertices.data(),
        GL_DYNAMIC_STORAGE_BIT);

    auto ebo = GLuint{};
    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(
        ebo,
        static_cast<GLsizei>(indices.size_bytes()),
        indices.data(),
        GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &_vao);

    constexpr auto binding_idx = 0;
    glVertexArrayVertexBuffer(_vao, binding_idx, vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(_vao, ebo);

    auto attrib_idx = 0;
    glEnableVertexArrayAttrib(_vao, attrib_idx);
    glVertexArrayAttribBinding(_vao, attrib_idx, binding_idx);
    glVertexArrayAttribFormat(
        _vao,
        attrib_idx,
        3,
        GL_FLOAT,
        GL_FALSE,
        offsetof(Vertex, position));

    attrib_idx = 1;
    glEnableVertexArrayAttrib(_vao, attrib_idx);
    glVertexArrayAttribBinding(_vao, attrib_idx, binding_idx);
    glVertexArrayAttribFormat(
        _vao,
        attrib_idx,
        3,
        GL_FLOAT,
        GL_FALSE,
        offsetof(Vertex, normal));

    attrib_idx = 2;
    glEnableVertexArrayAttrib(_vao, attrib_idx);
    glVertexArrayAttribBinding(_vao, attrib_idx, binding_idx);
    glVertexArrayAttribFormat(
        _vao,
        attrib_idx,
        2,
        GL_FLOAT,
        GL_FALSE,
        offsetof(Vertex, tex_coord));

    attrib_idx = 3;
    glEnableVertexArrayAttrib(_vao, attrib_idx);
    glVertexArrayAttribBinding(_vao, attrib_idx, binding_idx);
    glVertexArrayAttribFormat(
        _vao,
        attrib_idx,
        3,
        GL_FLOAT,
        GL_FALSE,
        offsetof(Vertex, tex_tangent));

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}

Mesh::Mesh(Mesh&& m) noexcept : _vao{0} {
    Mesh::_swap(*this, m);
}

Mesh& Mesh::operator=(Mesh&& m) noexcept {
    Mesh::_swap(*this, m);
    return *this;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &_vao);
}

void Mesh::draw() const {
    auto diffuse = _texture.diffuse != nullptr ? _texture.diffuse->id() : 0;
    auto normal = _texture.normal != nullptr ? _texture.normal->id() : 0;
    auto specular = _texture.specular != nullptr ? _texture.specular->id() : 0;
    glBindTextureUnit(0, diffuse);
    glBindTextureUnit(1, normal);
    glBindTextureUnit(2, specular);
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, _vert_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::_swap(Mesh& a, Mesh& b) {
    std::swap(a._texture, b._texture);
    std::swap(a._vao, b._vao);
    std::swap(a._vert_count, b._vert_count);
}
