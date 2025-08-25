#include <android/imagedecoder.h>
#include "TextureAsset.h"
#include "AndroidOut.h"
#include "Utility.h"

std::shared_ptr<TextureAsset>
TextureAsset::loadAsset(AAssetManager *assetManager, const std::string &assetPath) {
    // Get the image from asset manager
    auto pAndroidRobotPng = AAssetManager_open(
            assetManager,
            assetPath.c_str(),
            AASSET_MODE_BUFFER);

    // Make a decoder to turn it into a texture
    AImageDecoder *pAndroidDecoder = nullptr;
    auto result = AImageDecoder_createFromAAsset(pAndroidRobotPng, &pAndroidDecoder);
    assert(result == ANDROID_IMAGE_DECODER_SUCCESS);

    // make sure we get 8 bits per channel out. RGBA order.
    AImageDecoder_setAndroidBitmapFormat(pAndroidDecoder, ANDROID_BITMAP_FORMAT_RGBA_8888);

    // Get the image header, to help set everything up
    const AImageDecoderHeaderInfo *pAndroidHeader = nullptr;
    pAndroidHeader = AImageDecoder_getHeaderInfo(pAndroidDecoder);

    // important metrics for sending to GL
    auto width = AImageDecoderHeaderInfo_getWidth(pAndroidHeader);
    auto height = AImageDecoderHeaderInfo_getHeight(pAndroidHeader);
    auto stride = AImageDecoder_getMinimumStride(pAndroidDecoder);

    // Get the bitmap data of the image
    auto upAndroidImageData = std::make_unique<std::vector<uint8_t>>(height * stride);
    auto decodeResult = AImageDecoder_decodeImage(
            pAndroidDecoder,
            upAndroidImageData->data(),
            stride,
            upAndroidImageData->size());
    assert(decodeResult == ANDROID_IMAGE_DECODER_SUCCESS);

    // Get an opengl texture. TODO:
    GLuint textureId = 0;
    // GLuint textureId = uploadTexture(...)

    // cleanup helpers
    AImageDecoder_delete(pAndroidDecoder);
    AAsset_close(pAndroidRobotPng);

    // Create a shared pointer so it can be cleaned up easily/automatically
    return std::shared_ptr<TextureAsset>(new TextureAsset(textureId));
}

GLuint TextureAsset::uploadTexture(
        GLuint shader_program_id,
        const std::string& sampler_uniform_name,
        int gl_texture_slot_number,
        u_char* image_buffer,
        int width,
        int height,
        int sampler_wrapS,
        int sampler_wrapT,
        int sampler_min_filter,
        int sampler_mag_filter)
{
    // Get an opengl texture
    glActiveTexture(GL_TEXTURE0 + gl_texture_slot_number);
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Clamp to the edge, you'll get odd results alpha blending if you don't
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler_wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler_wrapT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler_mag_filter);

    // Load the texture into VRAM
    glTexImage2D(
            GL_TEXTURE_2D, // target
            0, // mip level
            GL_RGBA, // internal format, often advisable to use BGR
            width, // width of the texture
            height, // height of the texture
            0, // border (always 0)
            GL_RGBA, // format
            GL_UNSIGNED_BYTE, // type
            image_buffer // Data to upload
    );

    int loc = glGetUniformLocation(shader_program_id, sampler_uniform_name.c_str());
    if( loc >= 0 )
        glUniform1i(loc, gl_texture_slot_number);

    // generate mip levels. Not really needed for 2D, but good to do
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureId;
}

TextureAsset::~TextureAsset() {
    // return texture resources
    glDeleteTextures(1, &textureID_);
    textureID_ = 0;
}