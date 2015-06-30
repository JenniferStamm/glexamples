#include "MarchingCubesPainter.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Texture.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/resources/ResourceManager.h>

#include <gloperate/primitives/AdaptiveGrid.h>

#include "Chunk.h"
#include "ChunkRenderer.h"

using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

MarchingCubes::MarchingCubes(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
, m_targetFramebufferCapability{ addCapability(new gloperate::TargetFramebufferCapability()) }
,   m_viewportCapability{addCapability(new gloperate::ViewportCapability())}
,   m_projectionCapability{addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability))}
,   m_cameraCapability{addCapability(new gloperate::CameraCapability())}
,   m_chunks()
,   m_chunkRenderer()
,   m_chunkQueue()
,   m_useMipMap(true)
,   m_useMipMapChanged(false)
{
	addProperty<bool>("useMipMap", this,
		&MarchingCubes::useMipMap, &MarchingCubes::setUseMipMap);
}

MarchingCubes::~MarchingCubes() = default;

void MarchingCubes::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 100.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

bool MarchingCubes::useMipMap() const
{
	return m_useMipMap;
}

void MarchingCubes::setUseMipMap(bool useMipMap)
{
	m_useMipMapChanged = m_useMipMap != useMipMap;
	m_useMipMap = useMipMap;
}

void MarchingCubes::setupGrid()
{
    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({ 0.6f, 0.6f, 0.6f });
}

void MarchingCubes::setupOpenGLState()
{
    glClearColor(0.f, 0.f, 0.2f, 1.0f);
    glEnable(GL_CULL_FACE);
}

void MarchingCubes::onInitialize()
{
    // create program

    globjects::init();

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    setupGrid();
    setupProjection();
    setupOpenGLState();

	auto groundTexture = m_resourceManager.load<Texture>("data/marchingcubes/ground.png");
	groundTexture->setName("GroundTexture");
	groundTexture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	groundTexture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	if (m_useMipMap)
	{
		groundTexture->generateMipmap();
		groundTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		groundTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	auto colorTexture = m_resourceManager.load<Texture>("data/marchingcubes/terrain_color.jpg");
	colorTexture->setName("ColorTexture");
	colorTexture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	colorTexture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	if (m_useMipMap)
	{
		colorTexture->generateMipmap();
		colorTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		colorTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

    m_chunkRenderer = new ChunkRenderer();
	m_chunkRenderer->setGroundTexture(groundTexture);
	m_chunkRenderer->setColorTexture(colorTexture);

    m_chunks = {};
    int size = 7;
    for (int z = 0; z < size; ++z)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                m_chunkQueue.push(vec3(x, y, z));
            }
        }
    }
}

void MarchingCubes::onPaint()
{
    if (m_viewportCapability->hasChanged())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
    }

	if (m_useMipMapChanged)
	{
		m_chunkRenderer->updateTexture(m_useMipMap);
	}

    auto fbo = m_targetFramebufferCapability->framebuffer();

    if (!fbo)
        fbo = globjects::Framebuffer::defaultFBO();

    fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();

    m_chunkRenderer->setTransform(transform);

    m_chunkRenderer->render(m_chunks);

    m_grid->update(eye, transform);
    m_grid->draw();

    Framebuffer::unbind(GL_FRAMEBUFFER);


    // Generate up to 3 chunks
    for (int i = 0; i < 3; ++i)
    {
        if (m_chunkQueue.empty())
            break;
        auto offset = m_chunkQueue.front();
        auto newChunk = new Chunk(offset);
        m_chunkRenderer->generateDensities(newChunk);
        m_chunkRenderer->generateMesh(newChunk);
        m_chunks.push_back(newChunk);

        m_chunkQueue.pop();
    }

}
