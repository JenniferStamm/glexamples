#include "MarchingCubesPainter.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>
#include <globjects/TransformFeedback.h>
#include <globjects/VertexAttributeBinding.h>

#include <widgetzeug/make_unique.hpp>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>

#include "LookUpData.h"

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
,	m_positionVao()
,   m_cubeColor(255, 0, 0)
,   m_positions()
,   m_densities()
,   m_dimension(32, 32, 32)
,   m_edgeConnectList()
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
	m_positions = new Buffer();
    m_edgeConnectList = new Buffer();
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


    // Calculate densities
    m_densities = new Buffer();

    m_transformFeedbackProgram = new Program();
    m_transformFeedbackProgram->attach(Shader::fromFile(GL_VERTEX_SHADER, "data/marchingcubes/transformfeedback.vert"));

    m_transformFeedbackProgram->link();

    m_transformFeedback = new TransformFeedback();
	m_transformFeedback->setVaryings(m_transformFeedbackProgram, { { "out_density" } }, GL_INTERLEAVED_ATTRIBS);

	

    static const vec3 sphereCenter(15, 15, 15);
    static const float sphereRadius = 10.f;

    //std::vector<float> densities;

    std::vector<vec3> positions;
    for (int z = 0; z < m_dimension.z + 1; ++z)
    {
        for (int y = 0; y < m_dimension.y + 1; ++y)
        {
            for (int x = 0; x < m_dimension.x + 1; ++x)
            {
                if (x != m_dimension.x && y != m_dimension.y && z != m_dimension.z)
                {
                    positions.push_back(vec3(x, y, z));
                }

                // float density = - ((sphereCenter.x - x) * (sphereCenter.x - x) + (sphereCenter.y - y) * (sphereCenter.y - y) + (sphereCenter.z - z) * (sphereCenter.z - z) - sphereRadius * sphereRadius);
                // densities.push_back(density);
            }
        }
    }

    m_positions->setData(positions, gl::GL_STATIC_DRAW);

    m_size = positions.size();

    
    m_densities->setData(m_size * sizeof(float), nullptr, GL_STATIC_READ);
    
    m_positionVao = new VertexArray();

    auto positionsBinding = m_positionVao->binding(0);
    positionsBinding->setAttribute(0);
    positionsBinding->setBuffer(m_positions, 0, sizeof(vec3));
    positionsBinding->setFormat(3, GL_FLOAT);
    m_positionVao->enable(0);

    m_positionVao->unbind();

    m_positionVao->bind();
	m_transformFeedback->bind();
    m_densities->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    glEnable(GL_RASTERIZER_DISCARD);

    m_transformFeedbackProgram->use();
    m_transformFeedback->begin(GL_POINTS);
	m_positionVao->drawArrays(GL_POINTS, 0, m_size);
    m_transformFeedback->end();
    m_transformFeedback->unbind();
    m_transformFeedbackProgram->release();

    glDisable(GL_RASTERIZER_DISCARD);

	m_positionVao->unbind();

    /*float feedback[32 * 32 * 32];
    m_densities->getSubData(0, sizeof(feedback), feedback);

    for (int i = 0; i < 10; ++i)
    {
        debug() << feedback[i];
    }*/

    m_edgeConnectList->bind(GL_UNIFORM_BUFFER);
	m_edgeConnectList->setData(sizeof(ivec4) * LookUpData::m_edgeConnectList.size(), LookUpData::m_edgeConnectList.data(), GL_STATIC_DRAW);
    m_edgeConnectList->unbind(GL_UNIFORM_BUFFER);



	auto vertexBinding = m_vao->binding(0);
	vertexBinding->setAttribute(0);
	vertexBinding->setBuffer(m_positions, 0, sizeof(vec3));
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

    m_grid->update(eye, transform);
    m_grid->draw();

    m_program->use();
    m_program->setUniform(m_transformLocation, transform);
    m_program->setUniform("a_cubeColor", vec4(m_cubeColor.red() / 255.f, m_cubeColor.green() / 255.f, m_cubeColor.blue() / 255.f, m_cubeColor.alpha() / 255.f));
    m_program->setUniform("a_dim", m_dimension);


    auto ubo = m_program->uniformBlock("edgeConnectList");
    m_edgeConnectList->bindBase(GL_UNIFORM_BUFFER, 1);
    ubo->setBinding(1);


    auto ubo2 = m_program->uniformBlock("densityUniform");
    m_densities->bindBase(GL_UNIFORM_BUFFER, 2);
    ubo2->setBinding(2);

    m_program->setUniform("a_caseToNumPolys", LookUpData::m_caseToNumPolys);
    m_program->setUniform("a_edgeToVertices", LookUpData::m_edgeToVertices);
    //m_program->setUniform("densities", 5);

    //m_vao->binding(5)->setBuffer(m_densities, 0, sizeof(float));
    //m_vao->binding(5)->setFormat(1, gl::GL_FLOAT, gl::GL_FALSE, 0);

	m_vao->bind();
	m_vao->drawArrays(GL_POINTS, 0, m_size);
	m_vao->unbind();

    m_program->release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
