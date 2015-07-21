#include "ChunkRenderer.h"

#include <vec3_hash.h>

#include <glbinding/gl/enum.h>

#include <globjects/globjects.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/Query.h>

#include <loggingzeug/logging.h>

#include "Chunk.h"

using namespace gl;
using namespace glm;
using namespace globjects;

ChunkRenderer::ChunkRenderer()
    : m_transform()
	, m_striationTexture(nullptr)
	, m_groundTexture(nullptr)
    , m_useShadow(false)
    , m_useOcclusion(false)
    , m_useGroundTexture(false)
    , m_useStriationTexture(false)
{
    setupProgram();
    setupRendering();

}

ChunkRenderer::~ChunkRenderer() = default;

void ChunkRenderer::render(std::unordered_map<vec3, ref_ptr<Chunk>> chunks)
{
	if (!m_groundTexture || !m_striationTexture)
	{
		loggingzeug::warning("ChunkRenderer") << "Missing textures";
		return;
	}

	m_groundTexture->bindActive(GL_TEXTURE0);
	m_striationTexture->bindActive(GL_TEXTURE1);

    m_renderProgram->use();
    m_renderProgram->setUniform(m_transformLocation, m_transform);
    m_renderProgram->setUniform("useShadow", m_useShadow);
    m_renderProgram->setUniform("useOcclusion", m_useOcclusion);
    m_renderProgram->setUniform("useGroundTexture", m_useGroundTexture);
    m_renderProgram->setUniform("useStriationTexture", m_useStriationTexture);


    for (auto chunk : chunks)
    {
        m_renderProgram->setUniform(m_offsetLocation, chunk.first);    
        chunk.second->draw();
    }

    m_renderProgram->release();
	m_groundTexture->unbind();
	m_striationTexture->unbind();
}

void ChunkRenderer::setStriationTexture(globjects::ref_ptr<globjects::Texture> striationTexture)
{
    m_striationTexture = striationTexture;
}

void ChunkRenderer::setGroundTexture(globjects::ref_ptr<globjects::Texture> groundTexture)
{
	m_groundTexture = groundTexture;
}

void ChunkRenderer::setUseShadow(bool useShadow)
{
    m_useShadow = useShadow;
}

void ChunkRenderer::setUseOcclusion(bool useOcclusion)
{
    m_useOcclusion = useOcclusion;
}

void ChunkRenderer::setUseGroundTexture(bool useGroundTexture)
{
    m_useGroundTexture = useGroundTexture;
}

void ChunkRenderer::setUseStriationTexture(bool useStriationTexture)
{
    m_useStriationTexture = useStriationTexture;
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
	m_renderProgram->setUniform("striation", 1);
}

void ChunkRenderer::updateTexture(bool useMipMap)
{
	if (useMipMap)
	{
		m_groundTexture->generateMipmap();
		m_groundTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		m_striationTexture->generateMipmap();
		m_striationTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		m_striationTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} 
	else
	{
		m_groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		m_striationTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}
