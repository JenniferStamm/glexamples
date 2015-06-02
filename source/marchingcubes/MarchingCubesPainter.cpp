#include "MarchingCubesPainter.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>

#include "Chunk.h"

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
{
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

void MarchingCubes::setupGrid()
{
    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({ 0.6f, 0.6f, 0.6f });
}

void MarchingCubes::setupOpenGLState()
{
    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);
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

    m_chunks = {};
    int size = 3;
    for (int z = 0; z < size; ++z)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                m_chunks.push_back(new Chunk(vec3(x, y, z)));
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

    auto fbo = m_targetFramebufferCapability->framebuffer();

    if (!fbo)
        fbo = globjects::Framebuffer::defaultFBO();

    fbo->bind(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();

    m_grid->update(eye, transform);
    m_grid->draw();

    for (auto chunk : m_chunks)
    {
        chunk->setTransform(transform);
        chunk->draw();
    }

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
