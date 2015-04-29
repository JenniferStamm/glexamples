#include "MarchingCubesPainter.h"

#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/VertexAttributeBinding.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>


using namespace gl;
using namespace glm;
using namespace globjects;

using widgetzeug::make_unique;

MarchingCubes::MarchingCubes(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability{addCapability(make_unique<gloperate::TargetFramebufferCapability>())}
,   m_viewportCapability{addCapability(make_unique<gloperate::ViewportCapability>())}
,   m_projectionCapability{addCapability(make_unique<gloperate::PerspectiveProjectionCapability>(m_viewportCapability))}
,   m_cameraCapability{addCapability(make_unique<gloperate::CameraCapability>())}
, m_vao()
, m_vertices()
{
}

MarchingCubes::~MarchingCubes() = default;

void MarchingCubes::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 15.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));
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

	m_vao = new VertexArray();
	m_vertices = new Buffer();
    m_program = new Program{};
    m_program->attach(
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/icosahedron.vert"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/marchingcubes/icosahedron.frag")
    );

    m_transformLocation = m_program->getUniformLocation("transform");

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    setupProjection();

	std::vector<vec3> vertices;
	vertices.push_back(vec3(0.f, 0.f, 0.f));
	vertices.push_back(vec3(1.f, 0.f, 0.f));
	vertices.push_back(vec3(1.f, 1.f, 0.f));
	vertices.push_back(vec3(1.f, 1.f, 1.f));

	m_size = vertices.size();

	m_vertices->setData(vertices, gl::GL_STATIC_DRAW);

	auto vertexBinding = m_vao->binding(0);
	vertexBinding->setAttribute(0);
	vertexBinding->setBuffer(m_vertices, 0, sizeof(vec3));
	vertexBinding->setFormat(3, gl::GL_FLOAT, gl::GL_FALSE, 0);
	m_vao->enable(0);
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

    glEnable(GL_DEPTH_TEST);

    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();


    m_program->use();
    m_program->setUniform(m_transformLocation, transform);

	m_vao->bind();
	m_vao->drawArrays(GL_POINTS, 0, m_size);
	m_vao->unbind();

    m_program->release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
