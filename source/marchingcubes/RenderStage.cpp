#include "RenderStage.h"

#include <glm/trigonometric.hpp>

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>

#include <globjects/globjects.h>
#include <globjects/Texture.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/Camera.h>
#include <gloperate/primitives/AdaptiveGrid.h>

#include "Chunk.h"
#include "ChunkRenderer.h"

using namespace gl;
using namespace glm;
using namespace globjects;

RenderStage::RenderStage()
:   AbstractStage("Render")
, m_chunkRenderer()
, m_chunkQueue()
, m_initialized(false)
{
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("projection", projection);
    addInput("targetFBO", targetFBO);

    addInput("useMipMap", useMipMap);

	alwaysProcess(true);
}

void RenderStage::initialize()
{

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    setupGrid();
    setupProjection();
    setupOpenGLState();

	if (groundTexture.data() && colorTexture.data())
	{
		groundTexture.data()->setName("GroundTexture");
		groundTexture.data()->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		groundTexture.data()->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (useMipMap.data())
		{
			groundTexture.data()->generateMipmap();
			groundTexture.data()->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			groundTexture.data()->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}

		colorTexture.data()->setName("ColorTexture");
		colorTexture.data()->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		colorTexture.data()->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (useMipMap.data())
		{
			colorTexture.data()->generateMipmap();
			colorTexture.data()->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			colorTexture.data()->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
	}

	
    m_chunkRenderer = new ChunkRenderer();
	m_chunkRenderer->setGroundTexture(groundTexture.data());
	m_chunkRenderer->setColorTexture(colorTexture.data());

    m_chunks.clear();

	//render();
}

void RenderStage::process()
{
    auto rerender = true;

    if (viewport.hasChanged())
    {
        glViewport(
            viewport.data()->x(),
            viewport.data()->y(),
            viewport.data()->width(),
            viewport.data()->height());

        viewport.data()->setChanged(false);

        rerender = true;
    }

    if (camera.hasChanged() || projection.hasChanged() || targetFBO.hasChanged())
    {
        rerender = true;
    }

	if (groundTexture.hasChanged())
	{
		m_chunkRenderer->setGroundTexture(groundTexture.data());
	}

	if (colorTexture.hasChanged())
	{
		m_chunkRenderer->setColorTexture(colorTexture.data());
	}

    /*if (useMipMap.hasChanged())
    {
        m_chunkRenderer->updateTexture(useMipMap.data());
    }*/

    if (rerender)
    {
        render();

        invalidateOutputs();
    }

}

void RenderStage::render()
{
    auto fbo = targetFBO.data()->framebuffer();

    if (!fbo)
        fbo = globjects::Framebuffer::defaultFBO();

    fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto transform = projection.data()->projection() * camera.data()->view();
    const auto eye = camera.data()->eye();
	
    m_chunkRenderer->setTransform(transform);

    m_chunkRenderer->render(m_chunks);
	
    m_grid->update(eye, transform);
    m_grid->draw();

    Framebuffer::unbind(GL_FRAMEBUFFER);
	
    float distanceForAdding = 4.f;

    auto offset = ivec3(eye - distanceForAdding);

    for (int z = 0; z < distanceForAdding * 2; ++z)
    {
        for (int y = 0; y < distanceForAdding * 2; ++y)
        {
            for (int x = 0; x < distanceForAdding * 2; ++x)
            {
                auto newOffset = vec3(x, y, z) + vec3(offset);
                if (m_chunkQueue.size() < 100 && m_chunks.find(newOffset) == m_chunks.end())
                {
                    m_chunkQueue.push(newOffset);
                }
            }
        }
    }

    // Remove unneeded chunks

    float distanceForRemoving = 7.f;

    std::vector<vec3> chunksToRemove;

    for (auto chunk : m_chunks)
    {
        auto currentOffset = chunk.first;
        if (distance(currentOffset, eye) > distanceForRemoving)
        {
            chunksToRemove.push_back(chunk.first);
        }
    }

    for (auto chunkToRemove : chunksToRemove)
    {
        m_chunks.erase(chunkToRemove);
    }



    // Generate new non-empty chunks
    const unsigned int chunksToGenerate = 3u;

    for (int i = 0; i < chunksToGenerate;)
    {
        if (m_chunkQueue.empty())
            break;
        auto newOffset = m_chunkQueue.front();
        m_chunkQueue.pop();

        // Don't add chunk if it was already generated
        if (m_chunks.find(newOffset) != m_chunks.end())
            continue;

        auto newChunk = new Chunk(newOffset);
        m_chunkRenderer->generateDensities(newChunk);
        m_chunkRenderer->generateList(newChunk);
        if (!newChunk->isEmpty())
            m_chunkRenderer->generateMesh(newChunk);

        m_chunks[newOffset] = newChunk;

        if (!newChunk->isEmpty())
            ++i;
    }

	invalidateOutputs();
}

void RenderStage::setupGrid()
{
    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({ 0.6f, 0.6f, 0.6f });
}

void RenderStage::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 100.f, fovy = 50.f;

    projection.data()->setZNear(zNear);
    projection.data()->setZFar(zFar);
    projection.data()->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void RenderStage::setupOpenGLState()
{
    glClearColor(0.f, 0.f, 0.2f, 1.0f);
    glEnable(GL_CULL_FACE);
}
