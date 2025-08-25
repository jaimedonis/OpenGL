#pragma once

#include <memory>
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>

class TextureAsset {
public:
    /*!
     * Loads a texture asset from the assets/ directory
     * @param assetManager Asset manager to use
     * @param assetPath The path to the asset
     * @return a shared pointer to a texture asset, resources will be reclaimed when it's cleaned up
     */
    static std::shared_ptr<TextureAsset>
    loadAsset(AAssetManager *assetManager, const std::string &assetPath);

    static GLuint uploadTexture(
            GLuint shader_program_id,
            const std::string& sampler_uniform_name,
            int gl_texture_slot_number, // 0,1,2,... = GL_TEXTURE<gl_texture_index>
            u_char* image_buffer,
            int width,
            int height,
            int sampler_wrapS,
            int sampler_wrapT,
            int sampler_min_filter,
            int sampler_mag_filter);

    ~TextureAsset();

    /*!
     * @return the texture id for use with OpenGL
     */
    constexpr GLuint getTextureID() const { return textureID_; }

private:
    inline TextureAsset(GLuint textureId) : textureID_(textureId) {}

    GLuint textureID_;
};
