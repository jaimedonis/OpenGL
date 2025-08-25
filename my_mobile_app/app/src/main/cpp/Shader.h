#pragma once

#include "scene/SceneLight.h"

#include <string>
#include <memory>
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#include <GLES3/gl3.h>


class Model;

/*!
 * A class representing a simple shader program. It consists of vertex and fragment components. The
 * input attributes are a position (as a Vector3) and a uv (as a Vector2). It also takes a uniform
 * to be used as the entire model/view/projection matrix. The shader expects a single texture for
 * fragment shading, and does no other lighting calculations (thus no uniforms for lights or normal
 * attributes).
 */
class Shader {
public:
    /*!
     * Loads a shader given the full sourcecode and names for necessary attributes and uniforms to
     * link to. Returns a valid shader on success or null on failure. Shader resources are
     * automatically cleaned up on destruction.
     *
     * @param vertexSource The full source code for your vertex program
     * @param fragmentSource The full source code of your fragment program
     * @param positionAttributeName The name of the position attribute in your vertex program
     * @param uvAttributeName The name of the uv coordinate attribute in your vertex program
     * @param projectionMatrixUniformName The name of your model/view/projection matrix uniform
     * @return a valid Shader on success, otherwise null.
     */
    static Shader *loadShader();

    ~Shader();

    /*!
     * Prepares the shader for use, call this before executing any draw commands
     */
    void activate() const;

    /*!
     * Cleans up the shader after use, call this after executing any draw commands
     */
    void deactivate() const;

    /*!
     * Renders a single model
     * @param model a model to render
     */
    void drawModel(Model& model, const glm::mat4& world_transform, const glm::vec3& camera_position, const std::vector<SceneLight>& lights);

    /*!
     * Sets the camera view matrix in the shader.
     * @param cameraViewMatrix sixteen floats, column major, defining an OpenGL projection matrix.
     */
    void setCameraViewMatrix(const glm::mat4& camera_view_matrix);

    /*!
     * Sets the projection matrix in the shader.
     * @param projectionMatrix sixteen floats, column major, defining an OpenGL projection matrix.
     */
    void setProjectionMatrix(const glm::mat4& projection_matrix);


private:

    /*!
     * Constructs a new instance of a shader. Use @a loadShader
     * @param program the GL program id of the shader
     * @param position the attribute location of the position
     * @param uv the attribute location of the uv coordinates
     * @param projectionMatrix the uniform location of the projection matrix
     */
    Shader(GLuint program_id);

    /*!
     * Helper function to load a shader of a given type
     * @param shaderType The OpenGL shader type. Should either be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @param shaderSource The full source of the shader
     * @return the id of the shader, as returned by glCreateShader, or 0 in the case of an error
     */
    static GLuint loadShader(GLenum shaderType, const std::string &shaderSource);

    /*!
     * Creates all gpu shader resources for the given model, before rendering.
     */
    void useShader(Model& model, const std::vector<SceneLight>& lights);

    GLuint program_id_ = -1;

    struct ShaderParametersDefinition;
    ShaderParametersDefinition* params_ = nullptr;

    glm::mat4 camera_view_matrix_ = glm::mat4(1.0);
    glm::mat4 projection_matrix_ = glm::mat4(1.0);
};
