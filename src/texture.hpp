#pragma once

#include <glad/glad.h>

// A TextureData is a collection of raw bytes and the texture interpretation
// of those bytes.
struct TextureData {
    enum class Format { Greyscale, GreyAlpha, Rgb, Rgba };

    uint8_t* data;
    GLsizei width;
    GLsizei height;
    TextureData::Format format;
};

// A Texture is wrapper over opengl textures.
class Texture {
  public:
    // Create a new Texture from this given data.
    Texture(const TextureData&);

    // Disallow moves but allow copies.
    Texture(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) noexcept;
    ~Texture();

    // Get the opengl id of this texture.
    GLuint id() const;

  private:
    GLuint _tex_id; // opengl id of this texture

    // Swap the id of two Textures.
    static void _swap(Texture&, Texture&);
};

// A TextureGroup is a collection of textures that may be used in a mesh.
struct TextureGroup {
    const Texture* diffuse;
    const Texture* normal;
    const Texture* specular;
};
