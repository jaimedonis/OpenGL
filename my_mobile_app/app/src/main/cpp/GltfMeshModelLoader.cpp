
#include "GltfMeshModelLoader.h"
#include "Model.h"

#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#include "tiny_gltf.h"

#include <GLES3/gl3.h>
#include <string>
#include <memory>
#include <iostream>


struct VertexAttribute
{
    const unsigned char* data_ptr = nullptr;
    size_t data_size = 0;
    int byte_stride = 0;
    bool is_normalized = false;
};

bool GetVertexAttributeBuffer(tinygltf::Model& model, int accessor_idx, VertexAttribute& data_buffer)
{
    if(accessor_idx < 0 || accessor_idx > model.accessors.size() - 1) {
        // TODO: log error
        return false;
    }
    const auto &accessor = model.accessors[accessor_idx];
    const auto &buffer_view = model.bufferViews[accessor.bufferView];
    const auto &buffer = model.buffers[buffer_view.buffer];

    //data_buffer.data_ptr = buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset;
    //data_buffer.data_size = buffer.data.size() - (buffer_view.byteOffset + accessor.byteOffset);
    data_buffer.data_ptr = &buffer.data.at(0) + buffer_view.byteOffset;
    data_buffer.data_size = buffer_view.byteLength;
    data_buffer.byte_stride = accessor.ByteStride(buffer_view);
    data_buffer.is_normalized = accessor.normalized;

    if(data_buffer.data_size <= 0) {
        // TODO: log error
        return false;
    }

    return true;
}

static void QuatToAngleAxis(const std::vector<double>& quaternion,
        float& angle_radians,
        glm::vec3& axis) {

    double qx = quaternion[0];
    double qy = quaternion[1];
    double qz = quaternion[2];
    double qw = quaternion[3]; // gltf quaternion has w here.

    glm::quat rotation = glm::quat(qw, qx, qy, qz); // glm quaternion has w in the first element.
    axis = glm::axis(rotation);
    angle_radians = glm::angle(rotation);
}

static void ConvertTexture(
        const tinygltf::Model& model,
        const tinygltf::Texture& source_texture,
        Texture& model_texture)
{
    // sampler parameters
    if(source_texture.sampler != -1) {
        auto sampler = model.samplers[source_texture.sampler];
        model_texture._sampler_min_filter = sampler.minFilter;
        model_texture._sampler_mag_filter = sampler.magFilter;
        model_texture._sampler_wrap_s = sampler.wrapS;
        model_texture._sampler_wrap_t = sampler.wrapT;
    }
    // image parameters
    auto texture_image = model.images[source_texture.source];
    model_texture._name = texture_image.name;
    model_texture._image_data = texture_image.image;
    model_texture._image_width = texture_image.width;
    model_texture._image_height = texture_image.height;
    model_texture._image_uri = texture_image.uri;
}

static void ConvertSampler(const tinygltf::Sampler& source_sampler, Texture& model_texture)
{
    model_texture._sampler_min_filter = source_sampler.minFilter;
    model_texture._sampler_mag_filter = source_sampler.magFilter;
    model_texture._sampler_wrap_s = source_sampler.wrapS;
    model_texture._sampler_wrap_t = source_sampler.wrapT;
}

std::unique_ptr<Model> GltfMeshModelLoader::LoadModel(const std::string &resource_path)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // TODO: depending on the file extension, determine if we must load either text or binary format.
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, resource_path);
    //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
        return nullptr;
    }
    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
        return nullptr;
    }
    if (!ret) {
        printf("Failed to parse glTF\n");
        return nullptr;
    }

    auto engine_model = std::make_unique<Model>();

    const tinygltf::Scene& scene = model.scenes[model.defaultScene];
    for (int nodeIndex : scene.nodes) {
        const tinygltf::Node& node = model.nodes[nodeIndex];
        if (node.mesh < 0) {
            continue;
        }
        // ==process this node's transform (this is the location in the world)==
        auto mesh_transform = glm::mat4(1.0);

        if(!node.translation.empty()) {
            mesh_transform = glm::translate(mesh_transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        if(!node.rotation.empty()) {
            float angle = 0.0f;
            glm::vec3 axis(0.0);
            QuatToAngleAxis(node.rotation, angle, axis);
            mesh_transform = glm::rotate(mesh_transform, angle, axis);
            //NOTE: If the mesh appears facing in the opposite direction...:
            //To correct glTF's +Z forward into OpenGL’s -Z forward, apply a 180° rotation around Y:
            //glm::mat4 orientation_fix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            //mesh_transform *= orientation_fix;
            //or: mesh_transform = glm::rotate(mesh_transform, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if(!node.scale.empty()) {
            mesh_transform = glm::scale(mesh_transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }

        // ==process this node's mesh==
        tinygltf::Mesh& mesh = model.meshes[node.mesh];
        auto model_mesh = std::make_shared<ModelMesh>();
        model_mesh->_model_transform = mesh_transform; // keep the mesh transformation
        // process mesh primitives
        for (auto& primitive : mesh.primitives) {
            // next, for each primitive extract vertex attributes
            for(auto& attribute_pair : primitive.attributes) {
                VertexAttribute va;
                GetVertexAttributeBuffer(model, attribute_pair.second, va);
                if(va.data_size <= 0) {
                    // TODO: log error
                    continue;
                }
                if(attribute_pair.first == "POSITION") {
                    u_long count = floor(float(va.data_size) / sizeof(glm::vec3));
                    model_mesh->_vertices.resize(count);
                    memcpy(model_mesh->_vertices.data(), va.data_ptr, va.data_size);
                } else if( attribute_pair.first == "NORMAL") {
                    u_long count = floor(float(va.data_size) / sizeof(glm::vec3));
                    model_mesh->_normals.resize(count);
                    memcpy(model_mesh->_normals.data(), va.data_ptr, va.data_size);
                } else if( attribute_pair.first == "TANGENT") {
                    u_long count = floor(float(va.data_size) / sizeof(glm::vec4));
                    model_mesh->_tangents.resize(count);
                    memcpy(model_mesh->_tangents.data(), va.data_ptr, va.data_size);
                } else if( attribute_pair.first == "TEXCOORD_0") {
                    u_long count = floor(float(va.data_size) / sizeof(glm::vec2));
                    model_mesh->_tex_coords.resize(count);
                    memcpy(model_mesh->_tex_coords.data(), va.data_ptr, va.data_size);
                }
            } // attribute_pair
            // process mesh materials
            int material_idx = primitive.material;
            if(material_idx >= 0) {
                auto material = model.materials[material_idx];
                model_mesh->_material._name = material.name;
                // --color texture --
                if(material.pbrMetallicRoughness.baseColorTexture.index != -1) {
                    const auto& source_texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
                    ConvertTexture(model, source_texture, model_mesh->_material._pbr_base_color_texture);
                    if(source_texture.sampler != -1) {
                        auto sampler = model.samplers[source_texture.sampler];
                        ConvertSampler(sampler, model_mesh->_material._pbr_base_color_texture);
                    }
                }
                // --normal texture --
                if(material.normalTexture.index != -1) {
                    const auto& source_texture = model.textures[material.normalTexture.index];
                    ConvertTexture(model, source_texture, model_mesh->_material._normal_texture);
                    if(source_texture.sampler != -1) {
                        auto sampler = model.samplers[source_texture.sampler];
                        ConvertSampler(sampler, model_mesh->_material._normal_texture);
                    }
                }
                // --pbr metallic roughness
                if(material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
                    const auto& source_texture = model.textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
                    ConvertTexture(model, source_texture, model_mesh->_material._pbr_metallic_roughness_texture);
                    if(source_texture.sampler != -1) {
                        auto sampler = model.samplers[source_texture.sampler];
                        ConvertSampler(sampler, model_mesh->_material._pbr_metallic_roughness_texture);
                    }
                }
            } // end material_idx
            // --now, extract the mesh index buffer--
            VertexAttribute va;
            if(GetVertexAttributeBuffer(model, primitive.indices, va)) {
                u_long count = floor(float(va.data_size) / sizeof(Index));
                model_mesh->_indices.resize(count);
                memcpy(model_mesh->_indices.data(), va.data_ptr, va.data_size);
            }
        } // for mesh primitive
        // append this mesh to the engine model
        model_mesh->ComputeTangentSpace();
        engine_model->AddMesh(model_mesh);
    } // for scene nodes

    return engine_model;
}
