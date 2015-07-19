#pragma once

#include <unordered_map>

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

class ChunkFactory : public globjects::Referenced
{

public:
    ChunkFactory();
    ~ChunkFactory();

    void generateDensities(Chunk * chunk);
    void generateList(Chunk * chunk);
    void generateMesh(Chunk * chunk);

    globjects::Program * densityGenerationProgram() const { return m_densityGenerationProgram.get(); }

protected:
    void setupDensityGeneration();
    void setupListGeneration();
    void setupMeshGeneration();
    void setupNoiseTextures();
    globjects::ref_ptr<globjects::Texture> setupNoiseTexture(glm::vec3 offset);

protected:
    globjects::ref_ptr<globjects::Buffer> m_positions;

    globjects::ref_ptr<globjects::VertexArray> m_densityPositionVao;
    globjects::ref_ptr<globjects::VertexArray> m_meshVao;
    globjects::ref_ptr<globjects::VertexArray> m_listVao;

    globjects::ref_ptr<globjects::Buffer> m_densityPositions;
    gl::GLsizei m_densityPositionsSize;

    globjects::ref_ptr<globjects::Buffer> m_edgeConnectList;

    globjects::ref_ptr<globjects::TransformFeedback> m_densityGenerationTransformFeedback;
    globjects::ref_ptr<globjects::Program> m_densityGenerationProgram;

    globjects::ref_ptr<globjects::TransformFeedback> m_listGenerationTransformFeedback;
    globjects::ref_ptr<globjects::Program> m_listGenerationProgram;

    globjects::ref_ptr<globjects::TransformFeedback> m_meshGenerationTransformFeedback;
    globjects::ref_ptr<globjects::Program> m_meshGenerationProgram;


    gl::GLsizei m_positionsSize;


    globjects::ref_ptr<globjects::Texture> m_noiseTexture1;
    globjects::ref_ptr<globjects::Texture> m_noiseTexture2;
    globjects::ref_ptr<globjects::Texture> m_noiseTexture3;
    globjects::ref_ptr<globjects::Texture> m_noiseTexture4;
};
