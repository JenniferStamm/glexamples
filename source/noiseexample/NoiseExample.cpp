#include "NoiseExample.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

NoiseExample::NoiseExample(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability{addCapability(make_unique<gloperate::TargetFramebufferCapability>())}
,   m_viewportCapability{addCapability(make_unique<gloperate::ViewportCapability>())}
,   m_projectionCapability{addCapability(make_unique<gloperate::PerspectiveProjectionCapability>(m_viewportCapability))}
,   m_cameraCapability{addCapability(make_unique<gloperate::CameraCapability>())}
{
}

NoiseExample::~NoiseExample() = default;

void NoiseExample::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));
}

void NoiseExample::onInitialize()
{
    // create program

    globjects::init();

#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif


    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/NoiseExample/icosahedron.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/NoiseExample/icosahedron.frag")
    );

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad{m_program};

    m_transformLocation = m_program->getUniformLocation("transform");

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    setupProjection();
}

void NoiseExample::onPaint()
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

    glDisable(GL_DEPTH_TEST);

    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();

    m_screenAlignedQuad->draw();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
