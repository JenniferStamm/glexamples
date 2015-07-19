#include "ChunkRenderer.h"

#include <glm/vec3.hpp>
#include <vec3_hash.h>

#include <glbinding/gl/enum.h>

#include <globjects/VertexAttributeBinding.h>
#include <globjects/globjects.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Query.h>
#include <globjects/TransformFeedback.h>
#include <globjects/VertexAttributeBinding.h>

#include <loggingzeug/logging.h>

#include "LookUpData.h"
#include "Chunk.h"

using namespace gl;
using namespace glm;
using namespace globjects;

ChunkRenderer::ChunkRenderer()
    : m_transform()
	, m_colorTexture(nullptr)
	, m_groundTexture(nullptr)
{
    setupProgram();
    setupRendering();

}

ChunkRenderer::~ChunkRenderer() = default;

void ChunkRenderer::render(std::unordered_map<vec3, ref_ptr<Chunk>> chunks)
{
	if (!m_groundTexture || !m_colorTexture)
	{
		loggingzeug::warning("ChunkRenderer") << "Missing textures";
		return;
	}

	m_groundTexture->bindActive(GL_TEXTURE0);
	m_colorTexture->bindActive(GL_TEXTURE1);

    m_renderProgram->use();
    m_renderProgram->setUniform(m_transformLocation, m_transform);

    for (auto chunk : chunks)
    {
        m_renderProgram->setUniform(m_offsetLocation, chunk.first);    
        chunk.second->draw();
    }

    m_renderProgram->release();
	m_groundTexture->unbind();
	m_colorTexture->unbind();
}

void ChunkRenderer::setColorTexture(globjects::ref_ptr<globjects::Texture> colorTexture)
{
	m_colorTexture = colorTexture;
}

void ChunkRenderer::setGroundTexture(globjects::ref_ptr<globjects::Texture> groundTexture)
{
	m_groundTexture = groundTexture;
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
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/marchingcubes/marchingcubes.frag")
    );

    m_transformLocation = m_renderProgram->getUniformLocation("transform");
    m_offsetLocation = m_renderProgram->getUniformLocation("a_offset");
}

void ChunkRenderer::setupRendering()
{
	m_renderProgram->setUniform("ground", 0);
	m_renderProgram->setUniform("colorTex", 1);
}

void ChunkRenderer::updateTexture(bool useMipMap)
{
	if (useMipMap)
	{
		m_groundTexture->generateMipmap();
		m_groundTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		m_colorTexture->generateMipmap();
		m_colorTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_colorTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} 
	else
	{
		m_groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_colorTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}
