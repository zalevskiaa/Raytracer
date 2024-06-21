#pragma once

#include <image.h>
#include <camera_options.h>
#include <render_options.h>

#include <scene.h>
#include <geometry.h>

#include <string>

// const double kEps = 0.0001;
const double kInf = 1000000;

bool NormalZeros(const Object& object) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (std::fabs(object.normal.GetVertex(i)[j]) > kEps) {
                return false;
            }
        }
    }
    return true;
}

std::vector<std::vector<Vector>> ComputeRayDirections(const CameraOptions& camera_options) {
    std::vector<std::vector<Vector>> result(camera_options.screen_width,
                                            std::vector<Vector>(camera_options.screen_height));

    double aspect_ratio =
        static_cast<double>(camera_options.screen_width) / camera_options.screen_height;
    double scale = std::tan(camera_options.fov / 2);

    Vector forward = Vector(camera_options.look_from) - Vector(camera_options.look_to);
    forward.Normalize();
    Vector right =
        std::fabs(forward[1]) + kEps > 1 ? Vector{1, 0, 0} : CrossProduct({0, 1, 0}, forward);
    right.Normalize();
    Vector up = CrossProduct(forward, right);
    up.Normalize();

    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            double x = aspect_ratio * scale * (2 * (i + 0.5) / camera_options.screen_width - 1);
            double y = -1 * scale * (2 * (j + 0.5) / camera_options.screen_height - 1);
            double z = -1;
            result[i][j] = x * right + y * up + z * forward;
            result[i][j].Normalize();
        }
    }

    return result;
}

Image RenderDepth(const std::string& filename, const CameraOptions& camera_options) {
    Scene scene = ReadScene(filename);

    auto ray_directions = ComputeRayDirections(camera_options);
    std::vector<std::vector<double>> img(camera_options.screen_width,
                                         std::vector<double>(camera_options.screen_height, kInf));

    double max_distance = 0;

    for (int i = 0; i < camera_options.screen_width; ++i) {
        for (int j = 0; j < camera_options.screen_height; ++j) {
            Ray ray(Vector(camera_options.look_from), ray_directions[i][j]);

            for (const SphereObject& object : scene.GetSphereObjects()) {
                auto intersection = GetIntersection(ray, object.sphere);
                if (!intersection) {
                    continue;
                }
                img[i][j] =
                    std::min(img[i][j], Length(ray.GetOrigin() - intersection->GetPosition()));
            }

            for (const Object& object : scene.GetObjects()) {
                auto intersection = GetIntersection(ray, object.polygon);
                if (!intersection) {
                    continue;
                }
                img[i][j] =
                    std::min(img[i][j], Length(ray.GetOrigin() - intersection->GetPosition()));
            }

            if (img[i][j] < kInf - 1) {
                max_distance = std::max(img[i][j], max_distance);
            }
        }
    }

    if (max_distance < kEps) {
        throw std::runtime_error("lol, max_distance <= 0");
    }

    Image result(camera_options.screen_width, camera_options.screen_height);
    for (int i = 0; i < camera_options.screen_width; ++i) {
        for (int j = 0; j < camera_options.screen_height; ++j) {
            int pixel = img[i][j] < kInf - 1 ? 256 * img[i][j] / max_distance : 255;
            result.SetPixel({pixel, pixel, pixel}, j, i);
        }
    }

    return result;
}

Image RenderNormal(const std::string& filename, const CameraOptions& camera_options) {
    Scene scene = ReadScene(filename);

    auto ray_directions = ComputeRayDirections(camera_options);
    std::vector<std::vector<Vector>> img(
        camera_options.screen_width,
        std::vector<Vector>(camera_options.screen_height, {-kInf, -kInf, -kInf}));

    for (int i = 0; i < camera_options.screen_width; ++i) {
        for (int j = 0; j < camera_options.screen_height; ++j) {
            Ray ray(Vector(camera_options.look_from), ray_directions[i][j]);
            double distance = kInf;

            for (const SphereObject& object : scene.GetSphereObjects()) {
                auto intersection = GetIntersection(ray, object.sphere);
                if (!intersection) {
                    continue;
                }
                if (Length(ray.GetOrigin() - intersection->GetPosition()) < distance) {
                    distance = Length(ray.GetOrigin() - intersection->GetPosition());
                    img[i][j] = intersection->GetNormal();
                }
            }

            for (const Object& object : scene.GetObjects()) {
                auto intersection = GetIntersection(ray, object.polygon);
                if (!intersection) {
                    continue;
                }

                if (Length(ray.GetOrigin() - intersection->GetPosition()) < distance) {
                    distance = Length(ray.GetOrigin() - intersection->GetPosition());

                    if (NormalZeros(object)) {
                        img[i][j] = intersection->GetNormal();
                        continue;
                    }

                    Vector barycentric =
                        GetBarycentricCoords(object.polygon, intersection->GetPosition());
                    img[i][j] = {0, 0, 0};
                    for (int k = 0; k != 3; ++k) {
                        img[i][j] = img[i][j] + barycentric[k] * (*object.GetNormal(k));
                    }
                }
            }
        }
    }

    Image result(camera_options.screen_width, camera_options.screen_height);
    for (int i = 0; i < camera_options.screen_width; ++i) {
        for (int j = 0; j < camera_options.screen_height; ++j) {
            if (img[i][j][0] < -kInf + 1) {
                result.SetPixel({0, 0, 0}, j, i);
                continue;
            }
            Vector pixel = 255 * 0.5 * (img[i][j] + Vector{1, 1, 1});
            int r = pixel[0];
            int g = pixel[1];
            int b = pixel[2];
            result.SetPixel({r, g, b}, j, i);
        }
    }
    return result;
}

// WARNING: value of 1e-6 and less does shit on test deer in release
const double kEps2 = 1e-5;
const double kEps3 = 1e-5;

bool NoIntersection(const Scene& scene, const Ray& ray, double length) {
    for (const SphereObject& object : scene.GetSphereObjects()) {
        auto intersection = GetIntersection(ray, object.sphere);
        if (!intersection) {
            continue;
        }
        if (Length(ray.GetOrigin() - intersection->GetPosition()) < length + kEps3) {
            return false;
        }
    }
    for (const Object& object : scene.GetObjects()) {
        auto intersection = GetIntersection(ray, object.polygon);
        if (!intersection) {
            continue;
        }
        if (Length(ray.GetOrigin() - intersection->GetPosition()) < length + kEps3) {
            return false;
        }
    }
    return true;
}

Vector ComputeLights(const Scene& scene, const Intersection& intersection, const Material& material,
                     const Vector& normal, const Vector& from) {
    Vector result = material.ambient_color + material.intensity;

    for (const Light& light : scene.GetLights()) {
        Vector direction = light.position - intersection.GetPosition();
        direction.Normalize();

        double length = Length(intersection.GetPosition() - light.position);
        bool no_intersection = NoIntersection(
            scene, Ray(intersection.GetPosition() + kEps2 * normal, direction), length);

        if (no_intersection) {
            Vector v_l = light.position - intersection.GetPosition();
            Vector v_e = from - intersection.GetPosition();
            v_l.Normalize();
            v_e.Normalize();

            double diffuse = std::max(0.0, DotProduct(normal, v_l));
            double specular = std::pow(std::max(0.0, DotProduct(v_e, Reflect(-1. * v_l, normal))),
                                       material.specular_exponent);
            result = result +
                     material.albedo[0] * light.intensity *
                         (diffuse * material.diffuse_color + specular * material.specular_color);
        }
    }

    return result;
}

Vector SendRay(const Scene& scene, const RenderOptions& render_options, const Ray& ray, bool inside,
               int level) {
    if (level >= render_options.depth) {
        return {0, 0, 0};
    }
    bool intersection_found = false;
    double distance;
    Vector normal;
    const Material* material;
    Intersection result_intersection;

    for (const SphereObject& object : scene.GetSphereObjects()) {
        auto intersection = GetIntersection(ray, object.sphere);
        if (!intersection.has_value()) {
            continue;
        }
        if (!intersection_found ||
            Length(ray.GetOrigin() - intersection.value().GetPosition()) < distance) {
            intersection_found = true;
            distance = Length(ray.GetOrigin() - intersection.value().GetPosition());
            normal = intersection.value().GetNormal();
            material = object.material;
            result_intersection = {intersection.value().GetPosition(),
                                   intersection.value().GetNormal(),
                                   intersection.value().GetDistance()};
        }
    }

    for (const Object& object : scene.GetObjects()) {
        std::optional<Intersection> intersection = GetIntersection(ray, object.polygon);
        if (!intersection.has_value()) {
            continue;
        }
        if (!intersection_found ||
            Length(ray.GetOrigin() - intersection.value().GetPosition()) < distance) {
            intersection_found = true;
            distance = Length(ray.GetOrigin() - intersection.value().GetPosition());

            if (NormalZeros(object)) {
                normal = intersection.value().GetNormal();
            } else {
                normal = {0, 0, 0};
                Vector barycentric =
                    GetBarycentricCoords(object.polygon, intersection.value().GetPosition());
                for (int k = 0; k < 3; ++k) {
                    normal = normal + barycentric[k] * (*object.GetNormal(k));
                }
            }
            material = object.material;
            result_intersection = {intersection.value().GetPosition(),
                                   intersection.value().GetNormal(),
                                   intersection.value().GetDistance()};
        }
    }

    if (!intersection_found) {
        return {0, 0, 0};
    }

    Vector vector = result_intersection.GetPosition() - ray.GetOrigin();
    vector.Normalize();
    Vector reflected = Reflect(vector, normal);

    if (inside) {
        Vector refracted = *Refract(vector, normal, material->refraction_index);
        return ComputeLights(scene, result_intersection, *material, normal, ray.GetOrigin()) +
               (material->albedo[1] + material->albedo[2]) *
                   SendRay(scene, render_options,
                           Ray(result_intersection.GetPosition() - kEps2 * normal, refracted),
                           false, level + 1);
    }

    Vector refracted = *Refract(vector, normal, 1 / material->refraction_index);
    return ComputeLights(scene, result_intersection, *material, normal, ray.GetOrigin()) +
           material->albedo[1] *
               SendRay(scene, render_options,
                       Ray(result_intersection.GetPosition() + kEps2 * normal, reflected), false,
                       level + 1) +
           material->albedo[2] *
               SendRay(scene, render_options,
                       Ray(result_intersection.GetPosition() - kEps2 * normal, refracted), true,
                       level + 1);
}

void PostProcessing(std::vector<std::vector<Vector>>* img) {
    // post processing
    double max_value = 0;
    for (size_t i = 0; i < img->size(); ++i) {
        for (size_t j = 0; j < (*img)[0].size(); ++j) {
            for (int k = 0; k < 3; ++k) {
                if ((*img)[i][j][k] > max_value) {
                    max_value = (*img)[i][j][k];
                }
            }
        }
    }
    for (size_t i = 0; i < img->size(); ++i) {
        for (size_t j = 0; j < (*img)[0].size(); ++j) {
            (*img)[i][j] = (*img)[i][j] *
                           (Vector{1, 1, 1} + (*img)[i][j] / Vector{max_value * max_value,
                                                                    max_value * max_value,
                                                                    max_value * max_value}) /
                           (Vector{1, 1, 1} + (*img)[i][j]);
        }
    }

    // gamma
    for (size_t i = 0; i < img->size(); ++i) {
        for (size_t j = 0; j < (*img)[0].size(); ++j) {
            for (int k = 0; k < 3; ++k) {
                (*img)[i][j][k] = std::pow((*img)[i][j][k], 1 / 2.2);
            }
        }
    }
}

Image ImgToImage(const std::vector<std::vector<Vector>>& img, int width, int height) {
    Image image(width, height);
    for (int i = 0; i != width; ++i) {
        for (int j = 0; j != height; ++j) {
            int r = 255 * img[i][j][0];
            int g = 255 * img[i][j][1];
            int b = 255 * img[i][j][2];
            image.SetPixel({r, g, b}, j, i);
        }
    }
    return image;
}

Image RenderFull(const std::string& filename, const CameraOptions& camera_options,
                 const RenderOptions& render_options) {
    Scene scene = ReadScene(filename);

    auto ray_directions = ComputeRayDirections(camera_options);
    std::vector<std::vector<Vector>> img(camera_options.screen_width,
                                         std::vector<Vector>(camera_options.screen_height));

    for (int i = 0; i != camera_options.screen_width; ++i) {
        for (int j = 0; j != camera_options.screen_height; ++j) {
            Ray ray(Vector(camera_options.look_from), ray_directions[i][j]);
            img[i][j] = SendRay(scene, render_options, ray, false, 0);
        }
    }

    PostProcessing(&img);

    return ImgToImage(img, camera_options.screen_width, camera_options.screen_height);
}

Image Render(const std::string& filename, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    if (render_options.mode == RenderMode::kDepth) {
        return RenderDepth(filename, camera_options);
    }
    if (render_options.mode == RenderMode::kNormal) {
        // throw std::runtime_error("not implemented yet");
        return RenderNormal(filename, camera_options);
    }

    if (render_options.mode == RenderMode::kFull) {
        // throw std::runtime_error("not implemented");
        return RenderFull(filename, camera_options, render_options);
    }
    throw std::runtime_error("not implemented, and never gonna be");
}
