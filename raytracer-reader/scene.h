#pragma once

#include <material.h>
#include <vector.h>
#include <object.h>
#include <light.h>

#include <vector>
#include <map>
#include <string>

#include <fstream>

class Scene {
public:
    Scene() {
    }

    const std::vector<Object>& GetObjects() const {
        return objects_;
    }
    const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }
    const std::vector<Light>& GetLights() const {
        return lights_;
    }
    const std::map<std::string, Material>& GetMaterials() const {
        return materials_;
    }

    void AddObject(const Object& object) {
        objects_.push_back(object);
    }
    void AddSphereObject(double x, double y, double z, double r, const Material* mat) {
        sphere_objects_.push_back({mat, Sphere({x, y, z}, r)});
    }
    void AddLight(double x, double y, double z, double r, double g, double b) {
        lights_.push_back(Light({x, y, z}, {r, g, b}));  // what about move() ?
    }
    void SetMaterials(std::map<std::string, Material> materials) {
        materials_ = std::move(materials);
    }

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<Light> lights_;
    std::map<std::string, Material> materials_;
};

std::vector<std::string> Tokenize(const std::string& string, const std::string delim = " \t\r") {
    std::vector<std::string> result;
    for (size_t i = string.find_first_not_of(delim, 0), j = string.find_first_of(delim, i);
         i != std::string::npos;
         i = string.find_first_not_of(delim, j), j = string.find_first_of(delim, i)) {
        result.push_back(string.substr(i, j - i));
    }
    return result;
}
std::vector<std::string> TokenizeSlash(const std::string& string, const std::string delim = "/") {
    std::vector<std::string> result;
    size_t i = 0;
    for (size_t j = string.find_first_of(delim, i); j != std::string::npos;
         i = j + 1, j = string.find_first_of(delim, i)) {
        result.push_back(string.substr(i, j - i));
    }
    result.push_back(string.substr(i, string.size() - i));
    return result;
}

Vector GetVectorByIndex(const std::vector<Vector>& vectors, int index) {
    if (index == 0) {
        return {0, 0, 0};
    }
    if (index > 0) {
        return vectors.at(index - 1);
    }
    return vectors.at(vectors.size() + index);
}

Triangle GetTriangleByIndex(const std::vector<Vector>& vectors, const std::vector<int>& indices,
                            int index_0, int index_1, int index_2) {
    return {
        GetVectorByIndex(vectors, indices[index_0]),
        GetVectorByIndex(vectors, indices[index_1]),
        GetVectorByIndex(vectors, indices[index_2]),
    };
}

inline std::map<std::string, Material> ReadMaterials(std::string_view filename) {
    std::map<std::string, Material> materials;

    std::ifstream fin((std::string(filename)));
    std::string line;

    std::string material;
    // bool material_created = false;

    while (std::getline(fin, line)) {
        auto tokens = Tokenize(line);

        if (tokens.empty()) {
            continue;
        }
        if (tokens[0] == "#") {
            continue;
        }

        if (tokens[0] == "newmtl") {
            material = tokens[1];
            materials[material].name = material;

        } else if (tokens[0] == "Ka") {
            materials[material].ambient_color = {std::stod(tokens[1]), std::stod(tokens[2]),
                                                 std::stod(tokens[3])};

        } else if (tokens[0] == "Kd") {
            materials[material].diffuse_color = {std::stod(tokens[1]), std::stod(tokens[2]),
                                                 std::stod(tokens[3])};

        } else if (tokens[0] == "Ks") {
            materials[material].specular_color = {std::stod(tokens[1]), std::stod(tokens[2]),
                                                  std::stod(tokens[3])};

        } else if (tokens[0] == "Ns") {
            materials[material].specular_exponent = std::stod(tokens[1]);

        } else if (tokens[0] == "al") {
            materials[material].albedo = {std::stod(tokens[1]), std::stod(tokens[2]),
                                          std::stod(tokens[3])};

        } else if (tokens[0] == "illum") {

        } else if (tokens[0] == "Ni") {
            materials[material].refraction_index = std::stod(tokens[1]);
        } else if (tokens[0] == "Ke") {
            materials[material].intensity = {std::stod(tokens[1]), std::stod(tokens[2]),
                                             std::stod(tokens[3])};
        }
    }

    return materials;
}

inline Scene ReadScene(const std::string& filename) {
    Scene scene;

    std::ifstream fin((std::string(filename)));
    std::string line;

    const Material* material = nullptr;
    std::vector<Vector> vertices;
    std::vector<Vector> textures;
    std::vector<Vector> normals;

    while (std::getline(fin, line)) {
        auto tokens = Tokenize(line);
        if (tokens.empty()) {
            continue;
        } else if (tokens[0] == "S") {
            scene.AddSphereObject(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]),
                                  std::stod(tokens[4]), material);

        } else if (tokens[0] == "P") {
            scene.AddLight(std::stod(tokens[1]), std::stod(tokens[2]), std::stod(tokens[3]),
                           std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]));

        } else if (tokens[0] == "v") {
            vertices.push_back(
                {std::stod((tokens[1])), std::stod((tokens[2])), std::stod((tokens[3]))});

        } else if (tokens[0] == "vt") {
            textures.push_back(
                {std::stod((tokens[1])), std::stod((tokens[2])), std::stod((tokens[3]))});

        } else if (tokens[0] == "vn") {
            normals.push_back(
                {std::stod((tokens[1])), std::stod((tokens[2])), std::stod((tokens[3]))});

        } else if (tokens[0] == "f") {
            if (tokens.size() != 1 + 3 && false) {
                throw std::runtime_error("idk, maybe there must be 3 vertices? -_-");
            }

            std::vector<int> vertex_indices;
            std::vector<int> texture_indices;
            std::vector<int> normal_indices;

            for (size_t i = 1; i < tokens.size(); ++i) {
                std::vector<std::string> indices = TokenizeSlash(tokens[i], "/");

                if (indices.size() == 1) {
                    vertex_indices.push_back(std::stoi(indices[0]));
                    texture_indices.push_back(0);
                    normal_indices.push_back(0);
                    continue;
                }
                if (indices.size() == 2) {
                    vertex_indices.push_back(std::stoi(indices[0]));
                    texture_indices.push_back(std::stoi(indices[1]));
                    normal_indices.push_back(0);
                    continue;
                }
                if (indices[1].empty()) {
                    vertex_indices.push_back(std::stoi(indices[0]));
                    texture_indices.push_back(0);
                    normal_indices.push_back(std::stoi(indices[2]));
                    continue;
                }
                vertex_indices.push_back(std::stoi(indices[0]));
                texture_indices.push_back(std::stoi(indices[1]));
                normal_indices.push_back(std::stoi(indices[2]));
            }
            for (size_t j = 2; j < vertex_indices.size(); ++j) {
                scene.AddObject({
                    material,
                    GetTriangleByIndex(vertices, vertex_indices, 0, j - 1, j),
                    GetTriangleByIndex(textures, texture_indices, 0, j - 1, j),
                    GetTriangleByIndex(normals, normal_indices, 0, j - 1, j),
                });
            }

        } else if (tokens[0] == "mtllib") {
            scene.SetMaterials(ReadMaterials(
                std::string(filename).substr(0, filename.find_last_of("/") + 1) + tokens[1]));

        } else if (tokens[0] == "usemtl") {
            material = &scene.GetMaterials().at(tokens[1]);
        }
    }

    return scene;
}
