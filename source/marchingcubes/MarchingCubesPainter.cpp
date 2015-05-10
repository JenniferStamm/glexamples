#include "MarchingCubesPainter.h"

#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/VertexAttributeBinding.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>

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
,   m_vao()
,   m_cubeColor(255, 0, 0)
,   m_vertices()
,   m_densities()
, m_dimension(32, 32, 32)
{
    addProperty<reflectionzeug::Color>("cubeColor", this, &MarchingCubes::cubeColor, &MarchingCubes::setCubeColor);
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

reflectionzeug::Color MarchingCubes::cubeColor() const
{
    return m_cubeColor;
}

void MarchingCubes::setCubeColor(reflectionzeug::Color cubeColor)
{
    m_cubeColor = cubeColor;
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
        Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/marchingcubes.vert"),
        Shader::fromFile(GLenum::GL_GEOMETRY_SHADER, "data/marchingcubes/marchingcubes.geom"),
        Shader::fromFile(GL_FRAGMENT_SHADER, "data/marchingcubes/marchingcubes.frag")
    );

    m_transformLocation = m_program->getUniformLocation("transform");

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    m_grid = new gloperate::AdaptiveGrid{};
    m_grid->setColor({ 0.6f, 0.6f, 0.6f });

    setupProjection();

	std::vector<vec3> vertices;

    // Calculate densities
    m_densities = globjects::Texture::createDefault(GL_TEXTURE_3D);

    static const vec3 sphereCenter(15, 15, 15);
    static const float sphereRadius = 10.f;

    std::vector<float> densities;

    for (int z = 0; z < m_dimension.z + 1; ++z)
    {
        for (int y = 0; y < m_dimension.y + 1; ++y)
        {
            for (int x = 0; x < m_dimension.x + 1; ++x)
            {
                if (x != m_dimension.x && y != m_dimension.y && z != m_dimension.z)
                {
                    vertices.push_back(vec3(x, y, z));
                }

                float density = (sphereCenter.x - x) * (sphereCenter.x - x) + (sphereCenter.y - y) * (sphereCenter.y - y) + (sphereCenter.z - z) * (sphereCenter.z - z) - sphereRadius * sphereRadius;
                densities.push_back(density);
            }
        }
    }
    
    m_densities->image3D(0, GL_R32F, m_dimension.x + 1, m_dimension.y + 1, m_dimension.z +1, 0, GL_RED, GL_FLOAT, densities.data());
	
	m_vertices->setData(vertices, gl::GL_STATIC_DRAW);

	auto vertexBinding = m_vao->binding(0);
	vertexBinding->setAttribute(0);
	vertexBinding->setBuffer(m_vertices, 0, sizeof(vec3));
	vertexBinding->setFormat(3, gl::GL_FLOAT, gl::GL_FALSE, 0);
	m_vao->enable(0);
    m_size = vertices.size();
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

    m_grid->update(eye, transform);
    m_grid->draw();

    m_program->use();
    m_program->setUniform(m_transformLocation, transform);
    m_program->setUniform("a_cubeColor", vec4(m_cubeColor.red() / 255.f, m_cubeColor.green() / 255.f, m_cubeColor.blue() / 255.f, m_cubeColor.alpha() / 255.f));
    m_program->setUniform("a_dim", m_dimension);
    m_densities->bindActive(GL_TEXTURE0);
    m_program->setUniform("densities", 0);

	m_vao->bind();
	m_vao->drawArrays(GL_POINTS, 0, m_size);
	m_vao->unbind();

    m_program->release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
