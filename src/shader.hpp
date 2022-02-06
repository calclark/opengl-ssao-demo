#pragma once

#include <glad/glad.h>

#include <filesystem>

// A shader is a wrapper for opengl shaders
class Shader {
  public:
    // Create a new shader program from paths to vertex and fragment shaders
    Shader(const std::filesystem::path&, const std::filesystem::path&);

    // Allow moves but disallow copies.
    Shader(const Shader&) = delete;
    Shader(Shader&&) noexcept;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&) noexcept;
    ~Shader();

    // Get the opengl id of this shader program.
    [[nodiscard]] GLuint id() const;

    // Bind this shader program.
    void use() const;

  private:
    GLuint _id; // opengl id of this shader program

    // Swap the ids of two shaders.
    static void _swap(Shader&, Shader&);
};
