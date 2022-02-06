#include "texture.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <utility>

// Use the anonymouse namespace for private constants/functions.
namespace {
constexpr auto max_anisotropy = 16;

// Convert a TextureData::Format to its opengl base image format.
GLenum gl_base_format(const TextureData::Format& format) {
    switch (format) {
    case TextureData::Format::Greyscale:
        return GL_RED;
    case TextureData::Format::GreyAlpha:
        return GL_RG;
    case TextureData::Format::Rgb:
        return GL_RGB;
    case TextureData::Format::Rgba:
        return GL_RGBA;
    default:
        std::terminate();
    }
}

// Convert a TextureData::Format to its opengl sized image format.
GLenum gl_sized_format(const TextureData::Format& format) {
    switch (format) {
    case TextureData::Format::Greyscale:
        return GL_R8;
    case TextureData::Format::GreyAlpha:
        return GL_RG8;
    case TextureData::Format::Rgb:
        return GL_RGB8;
    case TextureData::Format::Rgba:
        return GL_RGBA8;
    default:
        std::terminate();
    }
}
} // namespace

Texture::Texture(const TextureData& tex_data) : _tex_id{0} {
    glCreateTextures(GL_TEXTURE_2D, 1, &_tex_id);
    glTextureParameteri(_tex_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(_tex_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(
        _tex_id,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(_tex_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(_tex_id, GL_TEXTURE_MAX_ANISOTROPY, max_anisotropy);
    auto levels =
        std::ceil(std::log2(std::max(tex_data.width, tex_data.height))) + 1;
    glTextureStorage2D(
        _tex_id,
        levels,
        gl_sized_format(tex_data.format),
        tex_data.width,
        tex_data.height);
    glTextureSubImage2D(
        _tex_id,
        0,
        0,
        0,
        tex_data.width,
        tex_data.height,
        gl_base_format(tex_data.format),
        GL_UNSIGNED_BYTE,
        tex_data.data);
    glGenerateTextureMipmap(_tex_id);
}

Texture::Texture(Texture&& t) noexcept : _tex_id{0} {
    Texture::_swap(*this, t);
}

Texture& Texture::operator=(Texture&& t) noexcept {
    Texture::_swap(*this, t);
    return *this;
}

Texture::~Texture() {
    glDeleteTextures(1, &_tex_id);
}

GLuint Texture::id() const {
    return _tex_id;
}

void Texture::_swap(Texture& t0, Texture& t1) {
    std::swap(t0._tex_id, t1._tex_id);
}
