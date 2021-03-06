#pragma once

#include <glm/vec3.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>

#include <reflectionzeug/base/FilePath.h>

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
    ChunkFactory(reflectionzeug::FilePath densityGenerationShaderFilePath);
    ~ChunkFactory();

    void generateDensities(Chunk * chunk);
    void generateList(Chunk * chunk);
    void generateMesh(Chunk * chunk);

    void setRemoveFloaters(bool removeFloaters);
    void updateDensityGenerationShaderFilePath(reflectionzeug::FilePath densityGenerationShaderFilePath);
    globjects::Program * densityGenerationProgram() const;

protected:
    void setupDensityGeneration();
    void setupListGeneration();
    void setupMeshGeneration();
    void setupNoiseTextures();
    globjects::ref_ptr<globjects::Texture> setupNoiseTexture(glm::vec3 offset);
    void setupPositionBuffer();

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

    reflectionzeug::FilePath m_densityGenerationShaderFilePath;
};
