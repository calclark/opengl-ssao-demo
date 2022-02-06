#include "loader.hpp"

#include <fmt/core.h>
#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.hpp>
#include <stb_image.h>

#include <fstream>

// Use an anonymous namespace for private helper methods.
namespace {
// A PathHash is a wrapper struct for hashing filesystem paths.
struct PathHash {
    size_t operator()(const std::filesystem::path& p) const noexcept {
        return std::filesystem::hash_value(p);
    }
};

// A TextureMap is a list of textures and a mapping from filesystem paths to
// indices in that list.
struct TextureMap {
    std::vector<Texture> textures;
    std::unordered_map<std::filesystem::path, size_t, PathHash> map;
};

// Read a wavefront .obj file to an ObjReader and return it.
tinyobj::ObjReader read_obj(const std::filesystem::path& path) {
    auto reader = tinyobj::ObjReader{};
    if (!reader.ParseFromFile(path)) {
        if (!reader.Error().empty()) {
            fmt::print(stderr, "TinyObjReader: {}", reader.Error());
        }
        std::terminate();
    }

    if (!reader.Warning().empty()) {
        fmt::print(stderr, "TinyObjReader: {}", reader.Warning());
    }

    return reader;
}

// Replace windows specific path seperators with universal path seperators
std::string fix_path(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
}

// Convert the number of components in an image to the appropriate texture
// format.
TextureData::Format num_components_to_format(int num_components) {
    switch (num_components) {
    case (1):
        return TextureData::Format::Greyscale;
    case (2):
        return TextureData::Format::GreyAlpha;
    case (3):
        return TextureData::Format::Rgb;
    case (4):
        return TextureData::Format::Rgba;
    default:
        std::terminate();
    }
}

// Read a P3 ppm file to a texture.
Texture read_ppm(const std::filesystem::path& path) {
    auto file = std::ifstream{path};

    // skip magic header
    file.seekg(2);

    auto data = std::vector<uint8_t>{};
    auto width = int{};
    auto height = int{};

    auto got_width = false;
    auto got_height = false;
    auto got_max_val = false;
    auto index = 0;
    while (!file.eof()) {
        auto n = int{};
        file >> n;

        // fails on comment; skip rest of line
        if (file.fail()) {
            file.clear();
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (!got_width) {
            width = n;
            got_width = true;
        } else if (!got_height) {
            height = n;
            got_height = true;
        } else if (!got_max_val) {
            got_max_val = true;
            data.resize(width * height * 3);
        } else {
            auto x = index % (width * 3);
            auto y = index / (width * 3);
            auto t = x / 3;
            data[(height - 1 - y) * width * 3 + (width * 3 - t * 3 + x % 3)] =
                n;
            index++;
        }
    }

    auto texture_data =
        TextureData{data.data(), width, height, num_components_to_format(3)};

    return Texture(texture_data);
}

// Read a file to a Texture.
Texture read_texture(const std::filesystem::path& path) {
    auto width = int{};
    auto height = int{};
    auto num_components = int{};
    stbi_set_flip_vertically_on_load(true);
    auto* data = stbi_load(path.c_str(), &width, &height, &num_components, 0);
    if (data != nullptr) {
        auto texture_data = TextureData{
            data,
            width,
            height,
            num_components_to_format(num_components)};
        auto texture = Texture{texture_data};
        stbi_image_free(data);
        return texture;
    } else if (path.extension() == ".ppm") {
        return read_ppm(path);
    }

    fmt::print(stderr, "Bad texture file: {}\n", path.c_str());
    std::terminate();
}

// Read and load a Texture from a filesystem path if not already loaded.
void load_texture(TextureMap& data, const std::filesystem::path& path) {
    if (!data.map.contains(path)) {
        auto texture = read_texture(path);
        data.map[path] = data.textures.size();
        data.textures.emplace_back(std::move(texture));
    }
}

// Load all relevant textures used by a wavefront .obj file
TextureMap load_textures(
    const std::vector<tinyobj::material_t>& materials,
    const std::filesystem::path& directory) {
    auto data = TextureMap{};
    for (auto& material : materials) {
        if (!material.diffuse_texname.empty()) {
            auto path = directory / fix_path(material.diffuse_texname);
            load_texture(data, path);
        }
        if (!material.normal_texname.empty()) {
            auto path = directory / fix_path(material.normal_texname);
            load_texture(data, path);
        }
        if (!material.specular_texname.empty()) {
            auto path = directory / fix_path(material.specular_texname);
            load_texture(data, path);
        }
    }
    return data;
}

// Create a vertex from face indices to vertex position, normal, and texture-
// coordinate data.
Vertex
gen_vertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& idx) {
    auto vertex = Vertex{};
    vertex.position = glm::vec3{
        attrib.vertices[3 * idx.vertex_index + 0],
        attrib.vertices[3 * idx.vertex_index + 1],
        attrib.vertices[3 * idx.vertex_index + 2]};
    if (idx.normal_index >= 0) {
        vertex.normal = glm::vec3{
            attrib.normals[3 * idx.normal_index + 0],
            attrib.normals[3 * idx.normal_index + 1],
            attrib.normals[3 * idx.normal_index + 2]};
    }
    if (idx.texcoord_index >= 0) {
        vertex.tex_coord = glm::vec2{
            attrib.texcoords[2 * idx.texcoord_index + 0],
            attrib.texcoords[2 * idx.texcoord_index + 1]};
    }
    return vertex;
}

// Generate the texture tangents for a face.
void generate_tangents(
    std::vector<Vertex>& vertices, const glm::uvec3& indices) {
    auto& v0 = vertices[indices.x];
    auto& v1 = vertices[indices.y];
    auto& v2 = vertices[indices.z];

    auto e1 = v1.position - v0.position;
    auto e2 = v2.position - v0.position;
    auto duv1 = v1.tex_coord - v0.tex_coord;
    auto duv2 = v2.tex_coord - v0.tex_coord;

    auto f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);

    auto tangent = glm::vec3{};
    tangent.x = f * (duv2.y * e1.x - duv1.y * e2.x);
    tangent.y = f * (duv2.y * e1.y - duv1.y * e2.y);
    tangent.z = f * (duv2.y * e1.z - duv1.y * e2.z);

    v0.tex_tangent = tangent;
    v1.tex_tangent = tangent;
    v2.tex_tangent = tangent;
}

// Create a Mesh from a wavefront .obj shape.
Mesh gen_mesh(
    const tinyobj::shape_t& s,
    const tinyobj::attrib_t& attrib,
    const std::vector<tinyobj::material_t>& materials,
    const TextureMap& texture_map,
    const std::filesystem::path& directory) {
    auto offset = 0;
    auto vertices = std::vector<Vertex>{};
    auto indices = std::vector<glm::uvec3>{};
    auto seen = std::unordered_map<glm::ivec3, unsigned>{};
    for (auto fv : s.mesh.num_face_vertices) {
        auto face = glm::uvec3{};
        for (auto v = 0; v < fv; v++) {
            auto idx = s.mesh.indices[offset + v];
            auto ind = glm::ivec3{
                idx.vertex_index,
                idx.normal_index,
                idx.texcoord_index};
            if (!seen.contains(ind)) {
                seen[ind] = vertices.size();
                vertices.emplace_back(gen_vertex(attrib, idx));
            }
            face[v] = seen[ind];
        }
        offset += fv;
        generate_tangents(vertices, face);
        indices.emplace_back(face);
    }

    // Assume all faces in a shape share textures
    // and there is at least one face per shape
    auto midx = s.mesh.material_ids[0];
    auto group = TextureGroup{};
    if (midx >= 0) {
        auto mat = materials[midx];
        if (!mat.diffuse_texname.empty()) {
            auto path = directory / fix_path(mat.diffuse_texname);
            if (texture_map.map.contains(path)) {
                group.diffuse = &texture_map.textures[texture_map.map.at(path)];
            }
        }
        if (!mat.normal_texname.empty()) {
            auto path = directory / fix_path(mat.normal_texname);
            if (texture_map.map.contains(path)) {
                group.normal = &texture_map.textures[texture_map.map.at(path)];
            }
        }
        if (!mat.specular_texname.empty()) {
            auto path = directory / fix_path(mat.specular_texname);
            if (texture_map.map.contains(path)) {
                group.specular =
                    &texture_map.textures[texture_map.map.at(path)];
            }
        }
    }
    return Mesh{vertices, indices, std::move(group)};
}
} // namespace

Scene Loader::load_obj(const std::filesystem::path& path) {
    auto reader = read_obj(path);
    auto texture_map = load_textures(reader.GetMaterials(), path.parent_path());
    auto meshes = std::vector<Mesh>{};
    for (const auto& s : reader.GetShapes()) {
        meshes.emplace_back(gen_mesh(
            s,
            reader.GetAttrib(),
            reader.GetMaterials(),
            texture_map,
            path.parent_path()));
    }
    return Scene{std::move(texture_map.textures), std::move(meshes)};
}
