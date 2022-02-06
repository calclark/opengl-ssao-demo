#include "shader.hpp"

#include <fmt/core.h>

#include <fstream>

// Use the anonymous namespace for private constants/methods
namespace {
constexpr auto read_size = size_t{1024};

// Assert that a shader operation was successful (COMPILE or LINK).
void assert_status(GLuint id, GLenum type) {
    void (*get_param)(GLuint, GLenum, GLint*) = nullptr;
    void (*info_log)(GLuint, GLsizei, GLsizei*, GLchar*) = nullptr;
    if (type == GL_COMPILE_STATUS) {
        get_param = glGetShaderiv;
        info_log = glGetShaderInfoLog;
    } else if (type == GL_LINK_STATUS) {
        get_param = glGetProgramiv;
        info_log = glGetProgramInfoLog;
    } else {
        std::terminate();
    }

    auto success = int{};
    get_param(id, type, &success);
    if (success == 0) {
        auto log_length = int{};
        get_param(id, GL_INFO_LOG_LENGTH, &log_length);
        auto log = std::string(log_length, ' ');
        info_log(id, log_length, nullptr, log.data());
        fmt::print(stderr, "{}\n", log);
        std::terminate();
    }
}

// Read a file as shader source code.
// https://stackoverflow.com/a/116220
std::string shader_source(const std::filesystem::path& path) {
    auto file = std::ifstream(path);
    file.exceptions(std::ios_base::badbit);

    auto out = std::string{};
    auto buf = std::string(read_size, '\0');
    while (!file.eof()) {
        file.read(&buf[0], read_size);
        out.append(buf, 0, file.gcount());
    }

    return out;
}

// Compile a file to an opengl shader.
GLuint compile_shader(const std::filesystem::path& path, GLenum type) {
    auto source = shader_source(path);
    const auto* csource = source.c_str();

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &csource, nullptr);
    glCompileShader(shader);

    assert_status(shader, GL_COMPILE_STATUS);
    return shader;
}
} // namespace

Shader::Shader(
    const std::filesystem::path& vertex_path,
    const std::filesystem::path& fragment_path)
    : _id{glCreateProgram()} {
    auto vertex_shader = compile_shader(vertex_path, GL_VERTEX_SHADER);
    auto fragment_shader = compile_shader(fragment_path, GL_FRAGMENT_SHADER);

    glAttachShader(_id, vertex_shader);
    glAttachShader(_id, fragment_shader);
    glLinkProgram(_id);

    assert_status(_id, GL_LINK_STATUS);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Shader::~Shader() {
    glDeleteProgram(_id);
}

Shader::Shader(Shader&& s) noexcept : _id{0} {
    Shader::_swap(*this, s);
}

Shader& Shader::operator=(Shader&& s) noexcept {
    Shader::_swap(*this, s);
    return *this;
}

GLuint Shader::id() const {
    return _id;
}

void Shader::use() const {
    glUseProgram(_id);
}

void Shader::_swap(Shader& a, Shader& b) {
    std::swap(a._id, b._id);
}
