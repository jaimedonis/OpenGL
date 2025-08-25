#include "Shader.h"

#include "AndroidOut.h"
#include "Model.h"
#include "TextureAsset.h"
#include "resource_builder/ResourceBuilder.h"
#include "resource_builder/ArchiveResource.h"

static const std::string g_vertex_shader_file_path = "shaders/pbr.vert";
static const std::string g_fragment_shader_file_path = "shaders/pbr.frag";

struct Shader::ShaderParametersDefinition {
    std::string position_name_;
    std::string normal_name_;
    std::string tangent_name_;
    std::string uv_name_;

    std::string model_name_;
    std::string camera_view_name_;
    std::string projection_name;

    std::string camera_position_name_;
    std::string number_of_lights_name;

    std::string pbr_lighting_block_name_;
    std::string pbr_material_block_name_;

    GLint position_idx_ = -1;
    GLint normal_idx_ = -1;
    GLint tangent_idx_ = -1;
    GLint uv_idx_ = -1;

    GLint model_idx_ = -1;
    GLint camera_view_idx_ = -1;
    GLint projection_idx_ = -1;

    GLint camera_position_idx_ = -1;
    GLint number_of_lights_idx_ = -1;

    GLuint pbr_lighting_block_idx_ = -1;
    GLint pbr_lighting_block_binding_point_ = 1;
    GLuint pbr_material_block_idx_ = -1;
    GLint pbr_material_block_binding_point_ = 2;

    std::string color_texture_sampler_name;
    int color_texture_slot_number = -1;
    std::string metallic_roughness_texture_sampler_name;
    int metallic_roughness_texture_slot_number = -1;
    std::string normal_texture_sampler_name;
    int normal_texture_slot_number = -1;
};

struct MeshMaterial
{
    glm::vec3 surface_albedo = glm::vec3(M_PI * 1.3);
    float specular_power = 300.0;
    glm::vec3 specular_color = glm::vec3(0.5, 0.5, 0.5); // percents of reflectivity;
    glm::vec3 ambient_color = glm::vec3(1.0, 1.0, 1.0);  // noon/daylight;
    glm::vec3 diffuse_color = glm::vec3(1.0, 1.0, 1.0);  // noon/daylight;
};


Shader::Shader(GLuint program_id)
: program_id_(program_id)
{
    params_ = new ShaderParametersDefinition;
    params_->position_name_ = "inPosition";
    params_->normal_name_ = "inNormal";
    params_->tangent_name_ = "inTangent";
    params_->uv_name_ = "inUV";

    params_->model_name_ = "uModel";
    params_->camera_view_name_ = "uCameraView";
    params_->projection_name = "uProjection";

    params_->camera_position_name_ = "uCameraPosition";

    params_->pbr_lighting_block_name_ = "uPBRLightingBlock";
    params_->number_of_lights_name = "uNumberOfLights";
    params_->pbr_material_block_name_ = "uMaterialBlock";

    params_->color_texture_sampler_name = "uColorTexture";
    params_->color_texture_slot_number = 0;
    params_->metallic_roughness_texture_sampler_name = "uMetallicRoughnessTexture";
    params_->metallic_roughness_texture_slot_number = 1;
    params_->normal_texture_sampler_name = "uNormalTexture";
    params_->normal_texture_slot_number = 2;
}

Shader::~Shader() {
    if (program_id_ != -1) {
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }
    if(params_ != nullptr) {
        delete params_;
        params_ = nullptr;
    }
}

Shader *Shader::loadShader()
{
    Shader *shader = nullptr;

    auto vert_shader_asset = ResourceBuilder::Open(g_vertex_shader_file_path);
    std::string vert_shader_text;
    if(vert_shader_asset->IsOpen()) {
        vert_shader_asset->ReadText(vert_shader_text);
    }
    auto frag_shader_asset = ResourceBuilder::Open(g_fragment_shader_file_path);
    std::string frag_shader_text;
    if(frag_shader_asset->IsOpen()) {
        frag_shader_asset->ReadText(frag_shader_text);
    }

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vert_shader_text);
    if (!vertexShader) {
        return nullptr;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, frag_shader_text);
    if (!fragmentShader) {
        glDeleteShader(fragmentShader);
        return nullptr;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_TRUE) {
            shader = new Shader(program);
        } else {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

            // If we fail to link the shader program, log the result for debugging
            if (logLength) {
                GLchar *log = new GLchar[logLength];
                glGetProgramInfoLog(program, logLength, nullptr, log);
                aout << "Failed to link program with:\n" << log << std::endl;
                delete[] log;
            }
            glDeleteProgram(program);
        }
    }

    // The shaders are no longer needed once the program is linked. Release their memory.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shader;
}

GLuint Shader::loadShader(GLenum shaderType, const std::string &shaderSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        auto *shaderRawString = (GLchar *) shaderSource.c_str();
        GLint shaderLength = shaderSource.length();
        glShaderSource(shader, 1, &shaderRawString, &shaderLength);
        glCompileShader(shader);

        GLint shaderCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompiled);

        // If the shader doesn't compile, log the result to the terminal for debugging
        if (!shaderCompiled) {
            GLint infoLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

            if (infoLength) {
                auto *infoLog = new GLchar[infoLength];
                glGetShaderInfoLog(shader, infoLength, nullptr, infoLog);
                aout << "Failed to compile with:\n" << infoLog << std::endl;
                delete[] infoLog;
            }

            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

void Shader::useShader(Model& model, const std::vector<SceneLight>& lights)
{
    // if we haven't fetched the shader's attribute and uniform locations, do so at this time.
    if(params_->position_idx_ == -1) {
        params_->position_idx_ = glGetAttribLocation(program_id_, params_->position_name_.c_str());
    }
    if(params_->normal_idx_ == -1) {
        params_->normal_idx_ = glGetAttribLocation(program_id_, params_->normal_name_.c_str());
    }
    if(params_->tangent_idx_ == -1) {
        params_->tangent_idx_ = glGetAttribLocation(program_id_, params_->tangent_name_.c_str());
    }
    if(params_->uv_idx_ == -1) {
        params_->uv_idx_ = glGetAttribLocation(program_id_, params_->uv_name_.c_str());
    }

    if(params_->model_idx_ == -1) {
        params_->model_idx_ = glGetUniformLocation(program_id_, params_->model_name_.c_str());
    }
    if(params_->camera_view_idx_ == -1) {
        params_->camera_view_idx_ = glGetUniformLocation(program_id_, params_->camera_view_name_.c_str());
    }
    if(params_->projection_idx_ == -1) {
        params_->projection_idx_ = glGetUniformLocation(program_id_, params_->projection_name.c_str());
    }

    if(params_->camera_position_idx_ == -1) {
        params_->camera_position_idx_ = glGetUniformLocation(program_id_, params_->camera_position_name_.c_str());
    }

    if(params_->pbr_lighting_block_idx_ == -1) {
        params_->pbr_lighting_block_idx_ = glGetUniformBlockIndex(program_id_, params_->pbr_lighting_block_name_.c_str());
        // Associate the uniform block index with a binding point
        glUniformBlockBinding ( program_id_, params_->pbr_lighting_block_idx_, params_->pbr_lighting_block_binding_point_ );
        // Get the size of <light data>; alternatively,
        // we can calculate it using sizeof(<light data>)
        GLint block_size = 0;
        glGetActiveUniformBlockiv ( program_id_, params_->pbr_lighting_block_idx_,
                GL_UNIFORM_BLOCK_DATA_SIZE,
                &block_size );
        auto struct_size = sizeof(SceneLight);
        GLint light_buffer_size = sizeof(SceneLight) * lights.size();
        //assert(block_size % sizeof(SceneLight) == 0);
        // Create and fill a buffer object
        GLuint buffer_id = -1;
        glGenBuffers ( 1, &buffer_id );
        glBindBuffer ( GL_UNIFORM_BUFFER, buffer_id );
        glBufferData ( GL_UNIFORM_BUFFER, light_buffer_size, lights.data(),
                GL_STATIC_DRAW);
        // Bind the buffer object to the uniform block binding point
        glBindBufferBase ( GL_UNIFORM_BUFFER, params_->pbr_lighting_block_binding_point_, buffer_id );
    }

    if(params_->number_of_lights_idx_ == -1) {
        params_->number_of_lights_idx_ = glGetUniformLocation(program_id_, params_->number_of_lights_name.c_str());
    }

    if(params_->pbr_material_block_idx_ == -1) {
        params_->pbr_material_block_idx_ = glGetUniformBlockIndex(program_id_, params_->pbr_material_block_name_.c_str());
        // Associate the uniform block index with a binding point
        glUniformBlockBinding ( program_id_, params_->pbr_material_block_idx_, params_->pbr_material_block_binding_point_ );
        // Get the size of lightData; alternatively,
        // we can calculate it using sizeof(lightData) in this example
        GLint block_size = 0;
        glGetActiveUniformBlockiv ( program_id_, params_->pbr_material_block_idx_,
                        GL_UNIFORM_BLOCK_DATA_SIZE,
                        &block_size );
        // Create and fill a buffer object
        PBRMaterialUBO material; // create a default material
        GLint material_size = sizeof(PBRMaterialUBO);
        assert(block_size % sizeof(PBRMaterialUBO) == 0);
        GLuint buffer_id = -1;
        glGenBuffers ( 1, &buffer_id );
        glBindBuffer ( GL_UNIFORM_BUFFER, buffer_id );
        glBufferData ( GL_UNIFORM_BUFFER, material_size, &material,
                GL_DYNAMIC_DRAW);
        // Bind the buffer object to the uniform block binding point
        glBindBufferBase ( GL_UNIFORM_BUFFER, params_->pbr_material_block_binding_point_, buffer_id );
    }

    // NOTE: for larger datasets, consider using Shader Storage Buffer Objects (SSBOs)

    // upload textures, if we haven't done so already.
    for(const auto& mesh : model.GetMeshes()) {
        if(mesh->_material._pbr_base_color_texture._id == -1) {
            mesh->_material._pbr_base_color_texture._id = TextureAsset::uploadTexture(
                    program_id_,
                    params_->color_texture_sampler_name,
                    params_->color_texture_slot_number,
                    mesh->_material._pbr_base_color_texture._image_data.data(),
                    mesh->_material._pbr_base_color_texture._image_width,
                    mesh->_material._pbr_base_color_texture._image_height,
                    mesh->_material._pbr_base_color_texture._sampler_wrap_s,
                    mesh->_material._pbr_base_color_texture._sampler_wrap_t,
                    mesh->_material._pbr_base_color_texture._sampler_min_filter,
                    mesh->_material._pbr_base_color_texture._sampler_mag_filter);
        }
        if(mesh->_material._pbr_metallic_roughness_texture._id == -1) {
            mesh->_material._pbr_metallic_roughness_texture._id = TextureAsset::uploadTexture(
                    program_id_,
                    params_->metallic_roughness_texture_sampler_name,
                    params_->metallic_roughness_texture_slot_number,
                    mesh->_material._pbr_metallic_roughness_texture._image_data.data(),
                    mesh->_material._pbr_metallic_roughness_texture._image_width,
                    mesh->_material._pbr_metallic_roughness_texture._image_height,
                    mesh->_material._pbr_metallic_roughness_texture._sampler_wrap_s,
                    mesh->_material._pbr_metallic_roughness_texture._sampler_wrap_t,
                    mesh->_material._pbr_metallic_roughness_texture._sampler_min_filter,
                    mesh->_material._pbr_metallic_roughness_texture._sampler_mag_filter);
        }
        if( mesh->_material._normal_texture._id == -1 ) {
            mesh->_material._normal_texture._id = TextureAsset::uploadTexture(
                    program_id_,
                    params_->normal_texture_sampler_name,
                    params_->normal_texture_slot_number,
                    mesh->_material._normal_texture._image_data.data(),
                    mesh->_material._normal_texture._image_width,
                    mesh->_material._normal_texture._image_height,
                    mesh->_material._normal_texture._sampler_wrap_s,
                    mesh->_material._normal_texture._sampler_wrap_t,
                    mesh->_material._normal_texture._sampler_min_filter,
                    mesh->_material._normal_texture._sampler_mag_filter);
        }
    }
}

void Shader::activate() const {
    glUseProgram(program_id_);
}

void Shader::deactivate() const {
    glUseProgram(0);
}

void Shader::drawModel(Model& model, const glm::mat4& world_transform, const glm::vec3& camera_position, const std::vector<SceneLight>& lights) {

    static std::vector<SceneLight> lights_buffer(lights.size());

    useShader(model, lights);
    glUniform1i(params_->number_of_lights_idx_, int(lights.size()));

    // --scene-wide level attributes--
    glUniform3fv(params_->camera_position_idx_, 1, glm::value_ptr(camera_position));

    for(const auto& mesh : model.GetMeshes()) {
        // --upload mvp for this draw call--
        glUniformMatrix4fv(params_->model_idx_, 1, false, glm::value_ptr(world_transform * mesh->_model_transform));
        glUniformMatrix4fv(params_->camera_view_idx_, 1, false, glm::value_ptr(camera_view_matrix_));
        glUniformMatrix4fv(params_->projection_idx_, 1, false, glm::value_ptr(projection_matrix_));
        // -- vertex attributes --
        // The position attribute is 3 floats
        glVertexAttribPointer(
                params_->position_idx_, // attrib
                3, // elements
                GL_FLOAT, // of type float
                GL_FALSE, // don't normalize
                sizeof(glm::vec3), // stride is Vertex bytes
                mesh->_vertices.data() // pull from the start of the vertex data
        );
        glEnableVertexAttribArray(params_->position_idx_);
        // The normal attribute is 3 floats
        glVertexAttribPointer(
                params_->normal_idx_, // attrib
                3, // elements
                GL_FLOAT, // of type float
                GL_FALSE, // don't normalize
                sizeof(glm::vec3), // stride is Vertex bytes
                mesh->_normals.data() // pull from the start of the vertex data
        );
        glEnableVertexAttribArray(params_->normal_idx_);
        // The tangent attribute is 4 floats
        glVertexAttribPointer(
                params_->tangent_idx_, // attrib
                4, // elements
                GL_FLOAT, // of type float
                GL_FALSE, // don't normalize
                sizeof(glm::vec4), // stride is Vertex bytes
                mesh->_tangents.data() // pull from the start of the vertex data
        );
        glEnableVertexAttribArray(params_->tangent_idx_);
        // The uv attribute is 2 floats
        glVertexAttribPointer(
                params_->uv_idx_, // attrib
                2, // elements
                GL_FLOAT, // of type float
                GL_FALSE, // don't normalize
                sizeof(glm::vec2), // stride is Vertex bytes
                mesh->_tex_coords.data()
        );
        glEnableVertexAttribArray(params_->uv_idx_);
        // --textures--
        // activate the base color textures
        glActiveTexture(GL_TEXTURE0 + params_->color_texture_slot_number);
        glBindTexture(GL_TEXTURE_2D, mesh->_material._pbr_base_color_texture._id);
        // activate the metallic roughness textures
        glActiveTexture(GL_TEXTURE0 + params_->metallic_roughness_texture_slot_number);
        glBindTexture(GL_TEXTURE_2D, mesh->_material._pbr_metallic_roughness_texture._id);
        // activate the normal texture
        glActiveTexture(GL_TEXTURE0 + params_->normal_texture_slot_number);
        glBindTexture(GL_TEXTURE_2D, mesh->_material._normal_texture._id);

        // --Draw as indexed triangles--
        glDrawElements(GL_TRIANGLES, mesh->_indices.size(), GL_UNSIGNED_SHORT, mesh->_indices.data());

        glDisableVertexAttribArray(params_->uv_idx_);
        glDisableVertexAttribArray(params_->tangent_idx_);
        glDisableVertexAttribArray(params_->normal_idx_);
        glDisableVertexAttribArray(params_->position_idx_);
    }
}

void Shader::setCameraViewMatrix(const glm::mat4& camera_view_matrix)
{
    camera_view_matrix_ = camera_view_matrix;
}

void Shader::setProjectionMatrix(const glm::mat4& projection_matrix)
{
    projection_matrix_ = projection_matrix;
}