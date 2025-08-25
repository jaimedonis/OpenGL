#ifndef ANDROIDGLINVESTIGATIONS_MODEL_H
#define ANDROIDGLINVESTIGATIONS_MODEL_H

#include <vector>
#include "TextureAsset.h"
#include "scene/SceneNode.h"
#include "Utility.h"

namespace Sampler {
    // all these constants correspond to the same GL constant values defined in glew.h
    static constexpr GLint WRAP_NONE = -1;
    static constexpr GLint WRAP_CLAMP_TO_EDGE = 33071;
    static constexpr GLint WRAP_MIRRORED_REPEAT = 33648;
    static constexpr GLint WRAP_REPEAT = 10497;
    static constexpr GLint FILTER_NONE = -1;
    static constexpr GLint FILTER_NEAREST = 9728; // common to min and mag filters
    static constexpr GLint FILTER_LINEAR = 9729; // common to min and mag filters
    static constexpr GLint FILTER_NEAREST_MIPMAP_NEAREST =9984;
    static constexpr GLint FILTER_LINEAR_MIPMAP_NEAREST = 9985;
    static constexpr GLint FILTER_NEAREST_MIPMAP_LINEAR = 9986;
    static constexpr GLint FILTER_LINEAR_MIPMAP_LINEAR = 9987;
};

struct Texture {
    std::string _name;
    std::vector<u_char> _image_data;
    std::string _image_uri;
    int _image_width = 0;
    int _image_height = 0;
    // this is the gl resource id, which must be created by the renderer.
    GLuint _id = -1;
    // how the texture coordinates are sampled when they fall outside the range [0,1]
    GLint _sampler_wrap_s = Sampler::WRAP_NONE;
    GLint _sampler_wrap_t = Sampler::WRAP_NONE;
    // the min filter property of a sampler specifies how a texture is sampled when the texture appears
    // smaller on the screen than the source image.
    GLint _sampler_min_filter = Sampler::FILTER_NONE;
    // the mag filter determines how a texture is magnified (zoomed in) when it's rendered on a surface where the
    // texture's resolution is lower than the screen's resolution for that area.
    GLint _sampler_mag_filter = Sampler::FILTER_NONE;
};

struct Material {
    std::string _name;
    Texture _pbr_base_color_texture;
    Texture _normal_texture;
};

struct MaterialUBO
{   // ( each field is 4-byte padded, based on std140 rule )
    glm::vec3 surface_albedo = glm::vec3(M_PI * 1.3);
    float specular_power = 300.0f;
    glm::vec4 specular_color = glm::vec4(0.5, 0.5, 0.5, 1.0); // percents of reflectivity;
    glm::vec4 ambient_color = glm::vec4(1.0); // noon/daylight
    glm::vec4 diffuse_color = glm::vec4(1.0);
};

struct ModelMesh {
    std::vector<glm::vec3> _vertices;
    std::vector<Index> _indices;
    std::vector<glm::vec3> _normals;
    std::vector<glm::vec4> _tangents;
    std::vector<glm::vec2> _tex_coords;
    Material _material;
    glm::mat4 _model_transform = glm::mat4(1.0f);

    void ComputeTangentSpace();
    void ComputeTangentSpaceHelper(glm::ivec3 triangleVertexIndices, bool useStoredNormals, std::vector<int>& averager);
};

class Model : public SceneNode
{
public:
    Model() = default;

    inline void AddMesh(const std::shared_ptr<ModelMesh>& mesh) {
        _meshes.emplace_back(mesh);
    }
    inline void RemoveMesh(const std::shared_ptr<ModelMesh>& mesh) {
        std::remove(_meshes.begin(), _meshes.end(), mesh);
    }
    inline const std::vector<std::shared_ptr<ModelMesh>>& GetMeshes() const {
        return _meshes;
    }

private:
    std::vector<std::shared_ptr<ModelMesh>> _meshes;
};

#endif //ANDROIDGLINVESTIGATIONS_MODEL_H