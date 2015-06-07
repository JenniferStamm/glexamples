#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/primitives/AbstractDrawable.h>

namespace globjects
{
    class Program;
    class Buffer;
    class Texture;
    class TransformFeedback;
    class VertexArray;
}

class Chunk : public globjects::Referenced
{
public:
    Chunk(glm::vec3 offset);
    ~Chunk();

    virtual void draw(globjects::VertexArray * vao, gl::GLsizei positionsSize);

    glm::vec3 offset() const { return m_offset; }
    globjects::ref_ptr<globjects::Buffer> densities() const { return m_densities; }
    void setupTransformFeedback(unsigned int densitySize);

protected:
    void setupRendering();

    globjects::ref_ptr<globjects::Buffer> m_densities;
    globjects::ref_ptr<globjects::Texture> m_densitiesTexture;

    glm::vec3 m_offset;
};
