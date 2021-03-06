#pragma once

// System Headers
#include <glad/glad.h>

// Standard Headers
#include <vector>

// Local Headers
#include "graphics/texture.hpp"

class FrameBuffer
{

  public:

    const GLuint id, width, height, samples;

    SharedTexture colorTexture;
    SharedTexture depthTexture;

    /**
     * Constructor for a (multisampled) FrameBuffer.
     * NOTE: multisampling is not supported in WebGL, samples will automatically be set to 0.
     */
    FrameBuffer(GLuint width, GLuint height, GLuint samples=0);

    ~FrameBuffer();

    void bind();

    void unbind();

    // format can be GL_RGB or GL_RGBA. magFilter and minFilter can be GL_LINEAR for example.
    void addColorTexture(GLuint format, GLuint magFilter, GLuint minFilter);

    void addColorBuffer(GLuint format);

    void addDepthTexture(GLuint magFilter, GLuint minFilter);

    void addDepthBuffer();

    // function to get the color pixels. Note: it binds the FrameBuffer
    void bindAndGetPixels(GLenum format, std::vector<GLubyte> &out, unsigned int outOffset);

  private:
    FrameBuffer *sampled = NULL;

    static void unbindCurrent();

};
