#include "ChunkRenderer.h"

#include <glm/vec3.hpp>

#include <glbinding/gl/enum.h>

#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/globjects.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/TransformFeedback.h>
#include <globjects/VertexAttributeBinding.h>

#include "LookUpData.h"
#include "Chunk.h"

using namespace gl;
using namespace glm;
using namespace globjects;

const ivec3 dimensions(32, 32, 32);
const int margin(1);

ChunkRenderer::ChunkRenderer()
    : m_densityPositions()
    , m_edgeConnectList()
    , m_positions()
    , m_transform()
{
    setupTransformFeedback();

    setupProgram();
    setupRendering();

}

ChunkRenderer::~ChunkRenderer() = default;

void ChunkRenderer::render(std::vector<ref_ptr<Chunk>> chunks)
{
    m_renderProgram->use();
    m_renderProgram->setUniform(m_transformLocation, m_transform);
    m_renderProgram->setUniform("a_dim", dimensions);
    m_renderProgram->setUniform("a_margin", margin);
    m_renderProgram->setUniform("a_caseToNumPolys", LookUpData::m_caseToNumPolys);
    m_renderProgram->setUniform("a_edgeToVertices", LookUpData::m_edgeToVertices);
    m_renderProgram->setUniform("densities", 0);

    // Setup uniform block for edge connect list

    auto ubo = m_renderProgram->uniformBlock("edgeConnectList");
    m_edgeConnectList->bindBase(GL_UNIFORM_BUFFER, 1);
    ubo->setBinding(1);


    m_renderingVao->bind();

    for (auto chunk : chunks)
    {
        m_renderProgram->setUniform("a_offset", chunk->offset());
        
        chunk->draw(m_renderingVao, m_positionsSize);
    }
    m_renderingVao->unbind();
    m_renderProgram->release();
}

void ChunkRenderer::setTransform(mat4x4 transform)
{
    m_transform = transform;
}

void ChunkRenderer::setupProgram()
{
    m_renderProgram = new Program{};
    m_renderProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/marchingcubes.vert"),
        Shader::fromFile(GL_GEOMETRY_SHADER, "data/marchingcubes/marchingcubes.geom"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/marchingcubes/marchingcubes.frag")
    );

    m_transformLocation = m_renderProgram->getUniformLocation("transform");
}

void ChunkRenderer::setupRendering()
{
    m_renderingVao = new VertexArray;

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

    auto positionsBinding = m_renderingVao->binding(0);
    positionsBinding->setAttribute(0);
    positionsBinding->setBuffer(m_positions, 0, sizeof(vec3));
    positionsBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_renderingVao->enable(0);
}

void ChunkRenderer::setupTransformFeedback()
{
    // Setup the program

    m_transformFeedbackProgram = new Program();
    m_transformFeedbackProgram->attach(Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/transformfeedback.vert"));
    m_transformFeedbackProgram->link();

    // Setup the transform feedback itself

    m_transformFeedback = new TransformFeedback();
    m_transformFeedback->setVaryings(m_transformFeedbackProgram, { "out_density" }, GL_INTERLEAVED_ATTRIBS);

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

void ChunkRenderer::runTransformFeedback(Chunk * chunk)
{
    chunk->setupTransformFeedback(m_densityPositionsSize);
    m_densityPositionVao->bind();
    m_transformFeedback->bind();
    m_transformFeedbackProgram->setUniform("a_offset", chunk->offset());
    chunk->densities()->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    glEnable(GL_RASTERIZER_DISCARD);

    m_transformFeedbackProgram->use();
    m_transformFeedback->begin(GL_POINTS);
    m_densityPositionVao->drawArrays(GL_POINTS, 0, m_densityPositionsSize);
    m_transformFeedback->end();
    m_transformFeedback->unbind();
    m_transformFeedbackProgram->release();

    glDisable(GL_RASTERIZER_DISCARD);

    m_densityPositionVao->unbind();
}
