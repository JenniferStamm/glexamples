#include "Chunk.h"

#include <vector>

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

using namespace gl;
using namespace glm;
using namespace globjects;

const ivec3 dimensions(32, 32, 32);

Chunk::Chunk(glm::vec3 offset)
    : AbstractDrawable()
    , m_densityPositions()
    , m_densities()
    , m_densitiesTexture()
    , m_edgeConnectList()
    , m_positions()
    , m_transform()
    , m_offset(offset)
{
    m_vao = new VertexArray;

    setupTransformFeedback();
    runTransformFeedback();

    setupProgram();
    setupRendering();

}

Chunk::~Chunk() = default;

void Chunk::draw()
{
    m_renderProgram->use();
    m_renderProgram->setUniform(m_transformLocation, m_transform);
    m_renderProgram->setUniform("a_dim", dimensions);
    m_renderProgram->setUniform("a_offset", m_offset);
    m_renderProgram->setUniform("a_caseToNumPolys", LookUpData::m_caseToNumPolys);
    m_renderProgram->setUniform("a_edgeToVertices", LookUpData::m_edgeToVertices);

    // Setup uniform block for edge connect list

    auto ubo = m_renderProgram->uniformBlock("edgeConnectList");
    m_edgeConnectList->bindBase(GL_UNIFORM_BUFFER, 1);
    ubo->setBinding(1);

    // Setup density buffer texture

    m_densitiesTexture->bindActive(GL_TEXTURE0);
    m_densitiesTexture->texBuffer(GL_R32F, m_densities);
    m_renderProgram->setUniform("densities", 0);


    m_vao->bind();
    m_vao->drawArrays(GL_POINTS, 0, m_size);
    m_vao->unbind();

    m_renderProgram->release();
}

void Chunk::setTransform(mat4x4 transform)
{
    m_transform = transform;
}

void Chunk::setupProgram()
{
    m_renderProgram = new Program{};
    m_renderProgram->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/marchingcubes.vert"),
        Shader::fromFile(GL_GEOMETRY_SHADER, "data/marchingcubes/marchingcubes.geom"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/marchingcubes/marchingcubes.frag")
    );

    m_transformLocation = m_renderProgram->getUniformLocation("transform");
}

void Chunk::setupRendering()
{
    m_densitiesTexture = new Texture(GL_TEXTURE_BUFFER);

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

    m_positions = new Buffer();
    m_positions->setData(positions, GL_STATIC_DRAW);

    // Setup positions binding

    auto positionsBinding = m_vao->binding(0);
    positionsBinding->setAttribute(0);
    positionsBinding->setBuffer(m_positions, 0, sizeof(vec3));
    positionsBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_size = positions.size();
}

void Chunk::setupTransformFeedback()
{
    // Setup the program

    m_transformFeedbackProgram = new Program();
    m_transformFeedbackProgram->attach(Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/transformfeedback.vert"));
    m_transformFeedbackProgram->link();
    m_transformFeedbackProgram->setUniform("a_offset", m_offset);

    // Setup the transform feedback itself

    m_transformFeedback = new TransformFeedback();
    m_transformFeedback->setVaryings(m_transformFeedbackProgram, { "out_density" }, GL_INTERLEAVED_ATTRIBS);

    // Fill positions buffer (with border!)

    std::vector<vec3> densityPositions;
    for (int z = 0; z < dimensions.z + 1; ++z)
    {
        for (int y = 0; y < dimensions.y + 1; ++y)
        {
            for (int x = 0; x < dimensions.x + 1; ++x)
            {
                densityPositions.push_back(vec3(x, y, z) / vec3(dimensions));
            }
        }
    }

    m_densityPositionsSize = densityPositions.size();

    m_densityPositions = new Buffer();
    m_densityPositions->setData(densityPositions, GL_STATIC_DRAW);

    // Setup result buffer

    m_densities = new Buffer();
    m_densities->setData(densityPositions.size() * sizeof(float), nullptr, GL_STATIC_READ);
    
    // Setup positions binding

    m_densityPositionVao = new VertexArray();

    auto densityPositionsBinding = m_densityPositionVao->binding(0);
    densityPositionsBinding->setAttribute(0);
    densityPositionsBinding->setBuffer(m_densityPositions, 0, sizeof(vec3));
    densityPositionsBinding->setFormat(3, GL_FLOAT);
    m_densityPositionVao->enable(0);
}

void Chunk::runTransformFeedback()
{
    m_densityPositionVao->bind();
    m_transformFeedback->bind();
    m_densities->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

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
