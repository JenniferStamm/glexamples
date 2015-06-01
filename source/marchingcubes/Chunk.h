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

class Chunk : public gloperate::AbstractDrawable, public globjects::Referenced
{
public:
    Chunk(glm::vec3 offset);
    ~Chunk();

    virtual void draw() override;

    void setTransform(glm::mat4x4 transform);

protected:
    void setupProgram();
    void setupRendering();
    void setupTransformFeedback();
    void runTransformFeedback();

protected:
    globjects::ref_ptr<globjects::Buffer> m_positions;
    gl::GLint m_transformLocation;

    globjects::ref_ptr<globjects::VertexArray> m_densityPositionVao;

    globjects::ref_ptr<globjects::Buffer> m_densityPositions;
    gl::GLsizei m_densityPositionsSize;

    globjects::ref_ptr<globjects::Buffer> m_densities;
    globjects::ref_ptr<globjects::Texture> m_densitiesTexture;
    globjects::ref_ptr<globjects::Buffer> m_edgeConnectList;

    globjects::ref_ptr<globjects::TransformFeedback> m_transformFeedback;
    globjects::ref_ptr<globjects::Program> m_transformFeedbackProgram;

    globjects::ref_ptr<globjects::Program> m_renderProgram;

    gl::GLsizei m_size;

    glm::mat4x4 m_transform;

    glm::vec3 m_offset;
};
