#include "ChunkFactory.h"

#include <glm/gtc/noise.hpp>

#include <glbinding/gl/enum.h>

#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/globjects.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Query.h>
#include <globjects/TransformFeedback.h>

#include "LookUpData.h"
#include "Chunk.h"

using namespace gl;
using namespace glm;
using namespace globjects;

const ivec3 dimensions(32, 32, 32);
const int margin(5);

ChunkFactory::ChunkFactory()
    : m_densityPositions()
    , m_edgeConnectList()
    , m_positions()
{
    setupNoiseTextures();
    setupDensityGeneration();
    setupListGeneration();
    setupMeshGeneration();
}

ChunkFactory::~ChunkFactory() = default;


void ChunkFactory::setupDensityGeneration()
{
    // Setup the program

    m_densityGenerationProgram = new Program();
    m_densityGenerationProgram->attach(Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/densitygeneration.vert"));
    m_densityGenerationProgram->link();

    // Setup the transform feedback itself

    m_densityGenerationTransformFeedback = new TransformFeedback();
    m_densityGenerationTransformFeedback->setVaryings(m_densityGenerationProgram, { "out_density" }, GL_INTERLEAVED_ATTRIBS);

    // Fill positions buffer (with border!)

    std::vector<vec3> densityPositions;
    for (int z = -margin; z < dimensions.z + margin + 1; ++z)
    {
        for (int y = -margin; y < dimensions.y + margin + 1; ++y)
        {
            for (int x = -margin; x < dimensions.x + margin + 1; ++x)
            {
                densityPositions.push_back(vec3(x, y, z) / vec3(dimensions));
            }
        }
    }

    m_densityPositionsSize = densityPositions.size();

    m_densityPositions = new Buffer();
    m_densityPositions->setData(densityPositions, GL_STATIC_DRAW);
    
    // Setup positions binding

    m_densityPositionVao = new VertexArray();

    auto densityPositionsBinding = m_densityPositionVao->binding(0);
    densityPositionsBinding->setAttribute(0);
    densityPositionsBinding->setBuffer(m_densityPositions, 0, sizeof(vec3));
    densityPositionsBinding->setFormat(3, GL_FLOAT);
    m_densityPositionVao->enable(0);
}

void ChunkFactory::setupListGeneration()
{
    // Setup the program

    m_listGenerationProgram = new Program();
    m_listGenerationProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/listgeneration.vert"),
        Shader::fromFile(GL_GEOMETRY_SHADER, "data/marchingcubes/listgeneration.geom"));
    m_listGenerationProgram->link();

    // Setup the transform feedback itself

    m_listGenerationTransformFeedback = new TransformFeedback();
    m_listGenerationTransformFeedback->setVaryings(m_listGenerationProgram, { "out_value" }, GLenum::GL_SEPARATE_ATTRIBS);


    m_listVao = new VertexArray;

    // Setup edge connect list

    m_edgeConnectList = new Buffer();
    m_edgeConnectList->bind(GL_UNIFORM_BUFFER);
    m_edgeConnectList->setData(sizeof(ivec4) * LookUpData::m_edgeConnectList.size(), LookUpData::m_edgeConnectList.data(), GL_STATIC_DRAW);
    m_edgeConnectList->unbind(GL_UNIFORM_BUFFER);

    std::vector<vec3> positions;
    for (int z = 0; z < dimensions.z; ++z)
    {
        for (int y = 0; y < dimensions.y; ++y)
        {
            for (int x = 0; x < dimensions.x; ++x)
            {
                positions.push_back(vec3(x, y, z) / vec3(dimensions));
            }
        }
    }

    m_positionsSize = positions.size();

    m_positions = new Buffer();
    m_positions->setData(positions, GL_STATIC_DRAW);

    // Setup positions binding

    ref_ptr<VertexAttributeBinding> positionsBinding = m_listVao->binding(1);
    positionsBinding->setAttribute(0);
    positionsBinding->setBuffer(m_positions, 0, sizeof(vec3));
    positionsBinding->setFormat(4, GL_FLOAT, GL_FALSE, 0);
    m_listVao->enable(1);
}

void ChunkFactory::setupMeshGeneration()
{
    // Setup the program

    m_meshGenerationProgram = new Program();
    m_meshGenerationProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/meshgeneration.vert"),
        Shader::fromFile(GL_GEOMETRY_SHADER, "data/marchingcubes/meshgeneration.geom"));
    m_meshGenerationProgram->link();

    // Setup the transform feedback itself

    m_meshGenerationTransformFeedback = new TransformFeedback();
    m_meshGenerationTransformFeedback->setVaryings(m_meshGenerationProgram, { "out_position", "out_normal" }, GLenum::GL_SEPARATE_ATTRIBS);


    m_meshVao = new VertexArray;


    // Setup value binding

    ref_ptr<VertexAttributeBinding> valueBinding = m_meshVao->binding(1);
    valueBinding->setAttribute(0);
    valueBinding->setFormat(1, GL_UNSIGNED_INT, GL_FALSE, 0);
    m_meshVao->enable(1);
}

void ChunkFactory::setupNoiseTextures()
{
    // Random values as offsets
    m_noiseTexture1 = setupNoiseTexture(vec3(0, 0, 0));
    m_noiseTexture2 = setupNoiseTexture(vec3(42, 1, 5));
    m_noiseTexture3 = setupNoiseTexture(vec3(2, 3, 4));
    m_noiseTexture4 = setupNoiseTexture(vec3(1, 33, 7));
}

ref_ptr<Texture> ChunkFactory::setupNoiseTexture(vec3 offset)
{
    static const int size(16);

    float data[size * size * size];

    int i = 0;
    for (int z = 0; z < size; ++z)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x, ++i)
            {
                float val = perlin(vec3(x, y, z) / float(size) + offset);
                data[i] = val;
            }
        }
    }
    auto texture = Texture::createDefault(GL_TEXTURE_3D);
    texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture->setParameter(GLenum::GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    texture->setParameter(GLenum::GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    texture->setParameter(GLenum::GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    texture->image3D(0, GL_R32F, size, size, size, 0, GL_RED, GL_FLOAT, data);
    return texture;
}

void ChunkFactory::generateDensities(Chunk * chunk)
{
    chunk->setupDensityGeneration(m_densityPositionsSize);
    m_densityPositionVao->bind();
    m_noiseTexture1->bindActive(GL_TEXTURE1);
    m_noiseTexture2->bindActive(GL_TEXTURE2);
    m_noiseTexture3->bindActive(GL_TEXTURE3);
    m_noiseTexture4->bindActive(GL_TEXTURE4);
    m_densityGenerationTransformFeedback->bind();

    m_densityGenerationProgram->setUniform("a_terrainPositions", chunk->terrainPositions());
    m_densityGenerationProgram->setUniform("a_terrainPositionCount", int(chunk->terrainPositions().size()));
    
    m_densityGenerationProgram->setUniform("a_offset", chunk->offset());
    m_densityGenerationProgram->setUniform("noiseTexture1", 1);
    m_densityGenerationProgram->setUniform("noiseTexture2", 2);
    m_densityGenerationProgram->setUniform("noiseTexture3", 3);
    m_densityGenerationProgram->setUniform("noiseTexture4", 4);
    chunk->densities()->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    glEnable(GL_RASTERIZER_DISCARD);

    m_densityGenerationProgram->use();
    m_densityGenerationTransformFeedback->begin(GL_POINTS);
    m_densityPositionVao->drawArrays(GL_POINTS, 0, m_densityPositionsSize);
    m_densityGenerationTransformFeedback->end();
    m_densityGenerationTransformFeedback->unbind();
    m_densityGenerationProgram->release();

    glDisable(GL_RASTERIZER_DISCARD);

    m_noiseTexture1->unbind();
    m_noiseTexture2->unbind();
    m_noiseTexture3->unbind();
    m_noiseTexture4->unbind();
    m_densityPositionVao->unbind();

    glFlush();

    chunk->teardownDensityGeneration();

}

void ChunkFactory::generateList(Chunk* chunk)
{
    chunk->setupListGeneration(m_positionsSize);

    m_listGenerationProgram->use();
    m_listGenerationProgram->setUniform("a_dim", dimensions);
    m_listGenerationProgram->setUniform("a_margin", margin);
    m_listGenerationProgram->setUniform("a_caseToNumPolys", LookUpData::m_caseToNumPolys);
    m_listGenerationProgram->setUniform("densities", 0);

    // Setup uniform block for edge connect list

    auto ubo = m_listGenerationProgram->uniformBlock("edgeConnectList");
    m_edgeConnectList->bindBase(GL_UNIFORM_BUFFER, 1);
    ubo->setBinding(1);

    m_listVao->bind();

    chunk->list()->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    
    glEnable(GL_RASTERIZER_DISCARD);

    m_listGenerationProgram->use();
    ref_ptr<Query> countQuery = new Query;
    countQuery->begin(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    m_listGenerationTransformFeedback->begin(GL_POINTS);
    m_listVao->drawArrays(GL_POINTS, 0, m_positionsSize);
    m_listGenerationTransformFeedback->end();
    countQuery->end(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    m_listGenerationTransformFeedback->unbind();
    m_listGenerationProgram->release();

    glDisable(GL_RASTERIZER_DISCARD);

    m_listVao->unbind();
    m_listGenerationProgram->release();

    glFlush();

    
    unsigned int primitivesWritten = countQuery->get(GL_QUERY_RESULT);
    
    chunk->setTriangleCount(primitivesWritten);
    chunk->teardownListGeneration();
}

void ChunkFactory::generateMesh(Chunk* chunk)
{
    chunk->setupMeshGeneration(m_meshVao.get());

    m_meshGenerationProgram->use();
    m_meshGenerationProgram->setUniform("a_dim", dimensions);
    m_meshGenerationProgram->setUniform("a_margin", margin);
    m_meshGenerationProgram->setUniform("a_edgeToVertices", LookUpData::m_edgeToVertices);
    m_meshGenerationProgram->setUniform("densities", 0);

    m_meshVao->bind();

    chunk->vertexPositions()->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    chunk->vertexNormals()->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1);

    glEnable(GL_RASTERIZER_DISCARD);

    m_meshGenerationProgram->use();
    m_meshGenerationTransformFeedback->begin(GL_POINTS);
    m_meshVao->drawArrays(GL_POINTS, 0, chunk->triangleCount());
    m_meshGenerationTransformFeedback->end();
    m_meshGenerationTransformFeedback->unbind();
    m_meshGenerationProgram->release();

    glDisable(GL_RASTERIZER_DISCARD);

    m_meshVao->unbind();
    m_meshGenerationProgram->release();

    glFlush();

    chunk->teardownMeshGeneration();
    
}

void ChunkFactory::setRemoveFloaters(bool removeFloaters)
{
    m_listGenerationProgram->setUniform("removeFloaters", removeFloaters);
}

globjects::Program* ChunkFactory::densityGenerationProgram() const
{
    return m_densityGenerationProgram.get();
}