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
const int margin(1);

Chunk::Chunk(glm::vec3 offset)
    : m_densities()
    , m_densitiesTexture()
    , m_offset(offset)
{
    setupRendering();
}

Chunk::~Chunk() = default;

void Chunk::draw(VertexArray * vao, gl::GLsizei positionsSize)
{
    m_densitiesTexture->bindActive(GL_TEXTURE0);
    m_densitiesTexture->texBuffer(GL_R32F, m_densities);

    vao->drawArrays(GL_POINTS, 0, positionsSize);
}

void Chunk::setupRendering()
{
    m_densitiesTexture = new Texture(GL_TEXTURE_BUFFER);
}

void Chunk::setupTransformFeedback(unsigned int densitySize)
{

    // Setup result buffer

    m_densities = new Buffer();
    m_densities->setData(densitySize * sizeof(float), nullptr, GL_STATIC_READ);
    
}

