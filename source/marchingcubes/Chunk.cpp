#include "Chunk.h"

#include <glbinding/gl/enum.h>

#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Texture.h>

using namespace gl;
using namespace glm;
using namespace globjects;

const ivec3 dimensions(32, 32, 32);

Chunk::Chunk(glm::vec3 offset)
    : AbstractDrawable()
    , m_densities()
    , m_list()
    , m_vertexPositions()
    , m_vertexNormals()
    , m_densitiesTexture()
    , m_offset(offset)
    , m_triangleCount(0)
    , m_isEmpty(true)
    , m_isValid(false)
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
    m_vao->drawArrays(GL_TRIANGLES, 0, m_triangleCount * 3);
    
    m_vao->unbind();
}

void Chunk::setupRendering()
{
    m_vao = new VertexArray;

    // Setup positions binding

    ref_ptr<VertexAttributeBinding> positionsBinding = m_vao->binding(0);
    positionsBinding->setAttribute(0);
    positionsBinding->setFormat(4, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    // Setup normals binding

    ref_ptr<VertexAttributeBinding> normalsBinding = m_vao->binding(1);
    normalsBinding->setAttribute(1);
    normalsBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(1);
}

void Chunk::setTriangleCount(unsigned triangleCount)
{
    m_triangleCount = triangleCount;

    m_isEmpty = triangleCount <= 0;

    // Vertices buffer can be freed if chunk is empty
    if (m_isEmpty)
    {
        m_list = nullptr;
        m_vertexPositions = nullptr;
        m_vertexNormals = nullptr;
        m_densities = nullptr;
        m_densitiesTexture = nullptr;
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

void Chunk::setupListGeneration(unsigned verticesSize)
{
    m_list = new Buffer();

    m_list->setData(verticesSize * 5 * sizeof(uint), nullptr, GL_STATIC_COPY);

    m_densitiesTexture = new Texture(GL_TEXTURE_BUFFER);
    m_densitiesTexture->bindActive(GL_TEXTURE0);
    m_densitiesTexture->texBuffer(GL_R32F, m_densities);
}

void Chunk::teardownListGeneration()
{
}

void Chunk::setupMeshGeneration(VertexArray * meshVao)
{
    meshVao->binding(1)->setBuffer(m_list.get(), 0, sizeof(uint));

    m_vertexPositions = new Buffer();
    m_vertexPositions->setData(m_triangleCount * 3 * sizeof(vec4), nullptr, GL_STATIC_COPY);

    m_vertexNormals = new Buffer();
    m_vertexNormals->setData(m_triangleCount * 3 * sizeof(vec3), nullptr, GL_STATIC_COPY);

    m_densitiesTexture->bindActive(GL_TEXTURE0);
    //m_densitiesTexture->texBuffer(GL_R32F, m_densities);
}

void Chunk::teardownMeshGeneration()
{
    m_vao->binding(0)->setBuffer(m_vertexPositions, 0, sizeof(vec4));
    m_vao->binding(1)->setBuffer(m_vertexNormals, 0, sizeof(vec3));
    
    // Density and list are not needed anymore

    m_list = nullptr;
    m_densities = nullptr;
    m_densitiesTexture = nullptr;
}

void Chunk::addTerrainPosition(glm::vec3 terrainPosition)
{
    m_terrainPositions.push_back(terrainPosition);
    m_isValid = false;
}