#include "Shader.h"

#include "AndroidOut.h"
#include "Model.h"
#include "Utility.h"

// Vertex shader, you'd typically load this from assets
// TBD: pg 120, GL shader book
static const char* g_vertex_source = R"vertex(#version 300 es

precision mediump float;

in vec3 inPosition;
in vec3 inNormal;
in vec4 inTangent; // ****TODO: Missing vertex attribute!****
in vec2 inUV;

uniform mat4 uModel;
uniform mat4 uCameraView;
uniform mat4 uProjection;

uniform vec3 uCameraPosition;

out vec3 vPosition;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBiTangent;
out vec2 vUV;

void main()
{
    mat4 modelViewMatrix = uCameraView * uModel;

    gl_Position = uProjection * modelViewMatrix * vec4(inPosition, 1.0);

    mat4 model_inverse = inverse(uModel);

    vec4 world_pos = uModel * vec4(inPosition, 1.0);
    vPosition = world_pos.xyz / world_pos.w;
    vNormal = normalize(mat3(model_inverse) * inNormal);
    vTangent = normalize(mat3(uModel) * inTangent.xyz);
    vBiTangent = cross(vNormal, vTangent) * inTangent.w;
    vUV = inUV;
}
)vertex";

// Fragment shader, you'd typically load this from assets
static const char* g_fragment_source = R"fragment(#version 300 es

precision mediump float;

#define PI 3.1415
#define MAX_LIGHTS 5

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBiTangent;
in vec2 vUV;

uniform mat4 uModel;
uniform mat4 uCameraView;
uniform mat4 uProjection;

uniform vec3 uCameraPosition;
uniform vec3 uLightPosition;
uniform vec3 uLightColor;

uniform sampler2D uColorTexture;  // for diffuse mapping
uniform sampler2D uNormalTexture; // for normal mapping

struct Material {
    vec3 surface_albedo;
    float specular_power;
    vec3 specular_color;
    vec3 ambient_color;
    vec3 diffuse_color;
};
layout(std140) uniform uMaterialBlock
{
    Material uMaterial;
};

out vec4 fragColor;


vec3 FetchObjectNormal(vec2 uv, vec3 normal, vec3 tangent, vec3 bitangent)
{
    vec3 bump_map_normal = texture(uNormalTexture, uv).rgb;   // given in [-1, 1] range
    bump_map_normal = normalize(bump_map_normal) * 2.0 - 1.0; // remap to [0, 1] range  (* 2.0 - 1.0)

    mat3 TBN = mat3( tangent,
                     bitangent,
                     normal );

    vec3 new_normal = TBN * bump_map_normal;

    return new_normal;
}

vec3 ComputeDiffuseReflection(vec3 normal, vec3 light_direction, vec3 light_color, vec3 diffuse_color)
{
    vec3 direct_color = light_color * dot(normal, light_direction);
    return (uMaterial.ambient_color + direct_color) * diffuse_color;
}

vec3 ComputeSpecularReflection(vec3 normal, vec3 half_vector, float nl, vec3 light_color)
{
    float highlight = pow(clamp(dot(normal, half_vector),0.0, 1.0), uMaterial.specular_power) * float(nl > 0.0);
    return light_color * uMaterial.specular_color * highlight;
}

vec3 Shade(vec3 world_position, vec3 normal, vec3 camera_position, vec3 diffuse_color)
{
    vec3 light_direction = normalize(uLightPosition - world_position);
    vec3 diffuse = ComputeDiffuseReflection(normal, light_direction, uLightColor, diffuse_color);

    vec3 eye_direction = normalize(camera_position - world_position);
    vec3 half_vector = normalize(light_direction + eye_direction);
    float nl = clamp(dot(normal, light_direction), 0.0, 1.0);
    vec3 specular = ComputeSpecularReflection(normal, half_vector, nl, uLightColor);

    float nh = clamp(dot(normal, half_vector), 0.0, 1.0);
    vec3 direct_color = (((uMaterial.surface_albedo / PI) * (diffuse * nl)) + (specular * pow(nh, uMaterial.specular_power))) * uLightColor;

    return direct_color;
}

void main()
{
    //uMaterial.surface_albedo = vec3(PI * 1.3);
    //uMaterial.specular_power = 300.0;
    //uMaterial.specular_color = vec3(0.5, 0.5, 0.5); // percents of reflectivity
    //uMaterial.ambient_color = vec3(1.0, 1.0, 1.0);  // noon/daylight
    //uMaterial.diffuse_color = vec3(1.0);

    // re-normalize, after interpolation.
    vec3 normal = normalize( vNormal );
    vec3 tangent = normalize( vTangent );
    vec3 bitangent = normalize( vBiTangent );

    // color texture value
    vec4 diffuse_color = texture(uColorTexture, vUV).rgba;

    normal = FetchObjectNormal(vUV, normal, tangent, bitangent);

    vec3 material_color = Shade(vPosition, normal, uCameraPosition, diffuse_color.rgb);

    fragColor = vec4(material_color, diffuse_color.a);
}
)fragment";



struct Shader::ShaderParametersDefinition {
    std::string position_name_;
    std::string normal_name_;
    std::string tangent_name_;
    std::string uv_name_;

    std::string model_name_;
    std::string camera_view_name_;
    std::string projection_name;

    std::string camera_position_name_;
    std::string light_position_name_;
    std::string light_color_name_;

    std::string material_block_name_;

    GLint position_idx_ = -1;
    GLint normal_idx_ = -1;
    GLint tangent_idx_ = -1;
    GLint uv_idx_ = -1;

    GLint model_idx_ = -1;
    GLint camera_view_idx_ = -1;
    GLint projection_idx_ = -1;

    GLint camera_position_idx_ = -1;
    GLint light_position_idx_ = -1;
    GLint light_color_idx_ = -1;

    GLuint material_block_idx_ = -1;
    GLint material_block_binding_point_ = 1;

    std::string color_texture_sampler_name;
    int color_texture_slot_number = -1;
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
    params_->light_position_name_ = "uLightPosition";
    params_->light_color_name_ = "uLightColor";

    params_->material_block_name_ = "uMaterialBlock";

    params_->color_texture_sampler_name = "uColorTexture";
    params_->color_texture_slot_number = 0;
    params_->normal_texture_sampler_name = "uNormalTexture";
    params_->normal_texture_slot_number = 1;
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

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, g_vertex_source);
    if (!vertexShader) {
        return nullptr;
    }

    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, g_fragment_source);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
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
    Utility::assertGlError();
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

void Shader::useShader(Model& model)
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
    if(params_->light_position_idx_ == -1) {
        params_->light_position_idx_ = glGetUniformLocation(program_id_, params_->light_position_name_.c_str());
    }
    if(params_->light_color_idx_ == -1) {
        params_->light_color_idx_ = glGetUniformLocation(program_id_, params_->light_color_name_.c_str());
    }

    if(params_->material_block_idx_ == -1) {
        params_->material_block_idx_ = glGetUniformBlockIndex(program_id_, params_->material_block_name_.c_str());
        // Associate the uniform block index with a binding point
        glUniformBlockBinding ( program_id_, params_->material_block_idx_, params_->material_block_binding_point_ );
        // Get the size of lightData; alternatively,
        // we can calculate it using sizeof(lightData) in this example
        GLint block_size = 0;
        glGetActiveUniformBlockiv ( program_id_, params_->material_block_idx_,
                        GL_UNIFORM_BLOCK_DATA_SIZE,
                        &block_size );
        // Create and fill a buffer object
        MaterialUBO mesh_material; // create a default material
        GLuint buffer_id = -1;
        glGenBuffers ( 1, &buffer_id );
        glBindBuffer ( GL_UNIFORM_BUFFER, buffer_id );
        glBufferData ( GL_UNIFORM_BUFFER, block_size, &mesh_material,
                GL_DYNAMIC_DRAW);
        // Bind the buffer object to the uniform block binding point
        glBindBufferBase ( GL_UNIFORM_BUFFER, params_->material_block_binding_point_, buffer_id );
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

void Shader::drawModel(Model& model, const glm::vec3& camera_position, const std::vector<SceneLight>& lights) {

    static std::vector<SceneLight> lights_buffer(lights.size());

    assert(lights.size() <= 5);

    useShader(model);

    // --scene-wide level attributes--
    glUniform3fv(params_->camera_position_idx_, 1, glm::value_ptr(camera_position));

    // set light info
    glUniform3fv(params_->light_position_idx_, 1, glm::value_ptr(lights[0].light_position));
    glUniform3fv(params_->light_color_idx_, 1, glm::value_ptr(lights[0].light_color));

    for(const auto& mesh : model.GetMeshes()) {
        // --upload mvp for this draw call--
        glUniformMatrix4fv(params_->model_idx_, 1, false, glm::value_ptr(mesh->_model_transform));
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
        glActiveTexture(GL_TEXTURE0); // GL_TEXTURE0.  (texture unit = GL_TEXTURE0 + idx)
        glBindTexture(GL_TEXTURE_2D, mesh->_material._pbr_base_color_texture._id);
        // activate the normal texture
        glActiveTexture(GL_TEXTURE1); // GL_TEXTURE1.  (texture unit = GL_TEXTURE0 + idx)
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