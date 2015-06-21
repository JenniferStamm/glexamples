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
    , m_densities()
    , m_vertexPositions()
    , m_vertexNormals()
    , m_densitiesTexture()
    , m_offset(offset)
    , m_triangleCount(0)
    , m_isEmpty(false)
{
    setupRendering();
}

Chunk::~Chunk() = default;

void Chunk::draw()
{
    // Don't draw if the cunk is empty
    if (isEmpty())
    {
        return;
    }
    m_vao->bind();
    m_vao->drawArrays(GL_TRIANGLES, 0, m_triangleCount);
    
    m_vao->unbind();
}

void Chunk::setupRendering()
{
    m_vao = new VertexArray;

    // Setup positions binding

    auto positionsBinding = m_vao->binding(0);
    positionsBinding->setAttribute(0);
    positionsBinding->setFormat(4, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    // Setup normals binding

    auto normalsBinding = m_vao->binding(1);
    normalsBinding->setAttribute(1);
    normalsBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(1);
}

void Chunk::setTriangleCount(unsigned triangleCount)
{
    m_triangleCount = triangleCount;

    // Vertices buffer can be freed if chunk is empty
    if (m_triangleCount == 0)
    {
        m_isEmpty = true;
        m_vertexPositions = nullptr;
        m_vertexNormals = nullptr;
        return;
    }

}

void Chunk::setupDensityGeneration(unsigned int densitySize)
{

    // Setup result buffer

    m_densities = new Buffer();
    m_densities->setData(densitySize * sizeof(float), nullptr, GL_STATIC_COPY);
    
}

void Chunk::teardownDensityGeneration()
{
}

void Chunk::setupMeshGeneration(unsigned int verticesSize)
{
    m_vertexPositions = new Buffer();
    //TODO Use proper size?
    m_vertexPositions->setData(verticesSize * 15 * sizeof(vec4), nullptr, GL_STATIC_COPY);

    m_vertexNormals = new Buffer();
    //TODO Use proper size?
    m_vertexNormals->setData(verticesSize * 15 * sizeof(vec3), nullptr, GL_STATIC_COPY);

    m_densitiesTexture = new Texture(GL_TEXTURE_BUFFER);
    m_densitiesTexture->bindActive(GL_TEXTURE0);
    m_densitiesTexture->texBuffer(GL_R32F, m_densities);
}

void Chunk::teardownMeshGeneration()
{
    m_vao->binding(0)->setBuffer(m_vertexPositions, 0, sizeof(vec4));
    m_vao->binding(1)->setBuffer(m_vertexNormals, 0, sizeof(vec3));
    // Density is not needed anymore

    m_densities = nullptr;
    m_densitiesTexture = nullptr;
}