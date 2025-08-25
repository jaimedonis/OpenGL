#pragma once

#include <EGL/egl.h>
#include <memory>

#include "Model.h"
#include "Shader.h"
#include "scene/SceneGraph.h"

struct android_app;

class Renderer {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     */
    explicit Renderer(android_app *pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT) {
        initRenderer();
    }

    virtual ~Renderer();

    void ApplyCurrentScene(std::unique_ptr<SceneGraph>& scene);

    /*!
     * Gets the shader program available for this renderer
     * @return Shader
     */
    Shader* GetShaderProgram() const { return shader_.get(); }

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Renders all the models in the renderer
     */
    void render();

private:
    /*!
     *  draws the entire scene nodes.
     */
    void renderCurrentScene();

    /*!
     * Performs necessary OpenGL initialization. Customize this if you want to change your EGL
     * context or application-wide settings.
     */
    void initRenderer();

    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    int width_ = 0;
    int height_ = 0;
    bool shaderNeedsNewProjectionMatrix_ = false;
    std::shared_ptr<Shader> shader_;
    std::unique_ptr<SceneGraph> _current_scene;
};
