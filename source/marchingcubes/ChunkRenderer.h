#pragma once

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <gloperate/primitives/AbstractDrawable.h>

class Chunk;

namespace globjects
{
    class Program;
    class Buffer;
    class Texture;
    class TransformFeedback;
    class VertexArray;
}

class ChunkRenderer : public globjects::Referenced
{

public:
    ChunkRenderer();
    ~ChunkRenderer();

    virtual void render(std::vector<globjects::ref_ptr<Chunk>> chunks);

    void setTransform(glm::mat4x4 transform);

    void generateDensities(Chunk * chunk);
    void generateMesh(Chunk * chunk);

protected:
    void setupProgram();
    void setupRendering();
    void setupDensityGeneration();
    void setupMeshGeneration();
    void setupNoiseTextures();
    globjects::ref_ptr<globjects::Texture> setupNoiseTexture(glm::vec3 offset);

protected:
    globjects::ref_ptr<globjects::Buffer> m_positions;
    gl::GLint m_transformLocation;

    globjects::ref_ptr<globjects::VertexArray> m_densityPositionVao;
    globjects::ref_ptr<globjects::VertexArray> m_meshVao;

    globjects::ref_ptr<globjects::Buffer> m_densityPositions;
    gl::GLsizei m_densityPositionsSize;

    globjects::ref_ptr<globjects::Buffer> m_edgeConnectList;

    globjects::ref_ptr<globjects::TransformFeedback> m_densityGenerationTransformFeedback;
    globjects::ref_ptr<globjects::Program> m_densityGenerationProgram;

    globjects::ref_ptr<globjects::TransformFeedback> m_meshGenerationTransformFeedback;
    globjects::ref_ptr<globjects::Program> m_meshGenerationProgram;

    globjects::ref_ptr<globjects::Program> m_renderProgram;

    gl::GLsizei m_positionsSize;

    glm::mat4x4 m_transform;

    globjects::ref_ptr<globjects::Texture> m_noiseTexture1;
    globjects::ref_ptr<globjects::Texture> m_noiseTexture2;
    globjects::ref_ptr<globjects::Texture> m_noiseTexture3;
    globjects::ref_ptr<globjects::Texture> m_noiseTexture4;
};
