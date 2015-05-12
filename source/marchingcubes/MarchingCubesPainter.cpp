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
,   m_dimension(32, 32, 32)
,   m_edgeConnectList()
,   m_caseToNumPolys()
,   m_edgeToVertices()
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

                float density = - ((sphereCenter.x - x) * (sphereCenter.x - x) + (sphereCenter.y - y) * (sphereCenter.y - y) + (sphereCenter.z - z) * (sphereCenter.z - z) - sphereRadius * sphereRadius);
                densities.push_back(density);
            }
        }
    }
    
    m_densities->image3D(0, GL_R32F, m_dimension.x + 1, m_dimension.y + 1, m_dimension.z +1, 0, GL_RED, GL_FLOAT, densities.data());
	
    m_caseToNumPolys = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4,
        2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1,
        3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0 };

    m_edgeToVertices = { ivec2(0, 1), ivec2(1, 2), ivec2(2, 3), ivec2(3, 0),
        ivec2(4, 5), ivec2(5, 6), ivec2(6, 7), ivec2(7, 4),
        ivec2(0, 4), ivec2(1, 5), ivec2(2, 6), ivec2(3, 7) };

    m_edgeConnectList = { ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 1, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 8, 3), ivec3(9, 8, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 3), ivec3(1, 2, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 2, 10), ivec3(0, 2, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 8, 3), ivec3(2, 10, 8), ivec3(10, 9, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 11, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 11, 2), ivec3(8, 11, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 9, 0), ivec3(2, 3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 11, 2), ivec3(1, 9, 11), ivec3(9, 8, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 10, 1), ivec3(11, 10, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 10, 1), ivec3(0, 8, 10), ivec3(8, 11, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 9, 0), ivec3(3, 11, 9), ivec3(11, 10, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 8, 10), ivec3(10, 8, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 7, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 3, 0), ivec3(7, 3, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 1, 9), ivec3(8, 4, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 1, 9), ivec3(4, 7, 1), ivec3(7, 3, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 10), ivec3(8, 4, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 4, 7), ivec3(3, 0, 4), ivec3(1, 2, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 2, 10), ivec3(9, 0, 2), ivec3(8, 4, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 10, 9), ivec3(2, 9, 7), ivec3(2, 7, 3), ivec3(7, 9, 4), ivec3(-1, -1, -1),
        ivec3(8, 4, 7), ivec3(3, 11, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(11, 4, 7), ivec3(11, 2, 4), ivec3(2, 0, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 0, 1), ivec3(8, 4, 7), ivec3(2, 3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 7, 11), ivec3(9, 4, 11), ivec3(9, 11, 2), ivec3(9, 2, 1), ivec3(-1, -1, -1),
        ivec3(3, 10, 1), ivec3(3, 11, 10), ivec3(7, 8, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 11, 10), ivec3(1, 4, 11), ivec3(1, 0, 4), ivec3(7, 11, 4), ivec3(-1, -1, -1),
        ivec3(4, 7, 8), ivec3(9, 0, 11), ivec3(9, 11, 10), ivec3(11, 0, 3), ivec3(-1, -1, -1),
        ivec3(4, 7, 11), ivec3(4, 11, 9), ivec3(9, 11, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 5, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 5, 4), ivec3(0, 8, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 5, 4), ivec3(1, 5, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 5, 4), ivec3(8, 3, 5), ivec3(3, 1, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 10), ivec3(9, 5, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 0, 8), ivec3(1, 2, 10), ivec3(4, 9, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 2, 10), ivec3(5, 4, 2), ivec3(4, 0, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 10, 5), ivec3(3, 2, 5), ivec3(3, 5, 4), ivec3(3, 4, 8), ivec3(-1, -1, -1),
        ivec3(9, 5, 4), ivec3(2, 3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 11, 2), ivec3(0, 8, 11), ivec3(4, 9, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 5, 4), ivec3(0, 1, 5), ivec3(2, 3, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 1, 5), ivec3(2, 5, 8), ivec3(2, 8, 11), ivec3(4, 8, 5), ivec3(-1, -1, -1),
        ivec3(10, 3, 11), ivec3(10, 1, 3), ivec3(9, 5, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 9, 5), ivec3(0, 8, 1), ivec3(8, 10, 1), ivec3(8, 11, 10), ivec3(-1, -1, -1),
        ivec3(5, 4, 0), ivec3(5, 0, 11), ivec3(5, 11, 10), ivec3(11, 0, 3), ivec3(-1, -1, -1),
        ivec3(5, 4, 8), ivec3(5, 8, 10), ivec3(10, 8, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 7, 8), ivec3(5, 7, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 3, 0), ivec3(9, 5, 3), ivec3(5, 7, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 7, 8), ivec3(0, 1, 7), ivec3(1, 5, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 5, 3), ivec3(3, 5, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 7, 8), ivec3(9, 5, 7), ivec3(10, 1, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 1, 2), ivec3(9, 5, 0), ivec3(5, 3, 0), ivec3(5, 7, 3), ivec3(-1, -1, -1),
        ivec3(8, 0, 2), ivec3(8, 2, 5), ivec3(8, 5, 7), ivec3(10, 5, 2), ivec3(-1, -1, -1),
        ivec3(2, 10, 5), ivec3(2, 5, 3), ivec3(3, 5, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(7, 9, 5), ivec3(7, 8, 9), ivec3(3, 11, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 5, 7), ivec3(9, 7, 2), ivec3(9, 2, 0), ivec3(2, 7, 11), ivec3(-1, -1, -1),
        ivec3(2, 3, 11), ivec3(0, 1, 8), ivec3(1, 7, 8), ivec3(1, 5, 7), ivec3(-1, -1, -1),
        ivec3(11, 2, 1), ivec3(11, 1, 7), ivec3(7, 1, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 5, 8), ivec3(8, 5, 7), ivec3(10, 1, 3), ivec3(10, 3, 11), ivec3(-1, -1, -1),
        ivec3(5, 7, 0), ivec3(5, 0, 9), ivec3(7, 11, 0), ivec3(1, 0, 10), ivec3(11, 10, 0),
        ivec3(11, 10, 0), ivec3(11, 0, 3), ivec3(10, 5, 0), ivec3(8, 0, 7), ivec3(5, 7, 0),
        ivec3(11, 10, 5), ivec3(7, 11, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 6, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 3), ivec3(5, 10, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 0, 1), ivec3(5, 10, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 8, 3), ivec3(1, 9, 8), ivec3(5, 10, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 6, 5), ivec3(2, 6, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 6, 5), ivec3(1, 2, 6), ivec3(3, 0, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 6, 5), ivec3(9, 0, 6), ivec3(0, 2, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 9, 8), ivec3(5, 8, 2), ivec3(5, 2, 6), ivec3(3, 2, 8), ivec3(-1, -1, -1),
        ivec3(2, 3, 11), ivec3(10, 6, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(11, 0, 8), ivec3(11, 2, 0), ivec3(10, 6, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 1, 9), ivec3(2, 3, 11), ivec3(5, 10, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 10, 6), ivec3(1, 9, 2), ivec3(9, 11, 2), ivec3(9, 8, 11), ivec3(-1, -1, -1),
        ivec3(6, 3, 11), ivec3(6, 5, 3), ivec3(5, 1, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 11), ivec3(0, 11, 5), ivec3(0, 5, 1), ivec3(5, 11, 6), ivec3(-1, -1, -1),
        ivec3(3, 11, 6), ivec3(0, 3, 6), ivec3(0, 6, 5), ivec3(0, 5, 9), ivec3(-1, -1, -1),
        ivec3(6, 5, 9), ivec3(6, 9, 11), ivec3(11, 9, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 10, 6), ivec3(4, 7, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 3, 0), ivec3(4, 7, 3), ivec3(6, 5, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 9, 0), ivec3(5, 10, 6), ivec3(8, 4, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 6, 5), ivec3(1, 9, 7), ivec3(1, 7, 3), ivec3(7, 9, 4), ivec3(-1, -1, -1),
        ivec3(6, 1, 2), ivec3(6, 5, 1), ivec3(4, 7, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 5), ivec3(5, 2, 6), ivec3(3, 0, 4), ivec3(3, 4, 7), ivec3(-1, -1, -1),
        ivec3(8, 4, 7), ivec3(9, 0, 5), ivec3(0, 6, 5), ivec3(0, 2, 6), ivec3(-1, -1, -1),
        ivec3(7, 3, 9), ivec3(7, 9, 4), ivec3(3, 2, 9), ivec3(5, 9, 6), ivec3(2, 6, 9),
        ivec3(3, 11, 2), ivec3(7, 8, 4), ivec3(10, 6, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 10, 6), ivec3(4, 7, 2), ivec3(4, 2, 0), ivec3(2, 7, 11), ivec3(-1, -1, -1),
        ivec3(0, 1, 9), ivec3(4, 7, 8), ivec3(2, 3, 11), ivec3(5, 10, 6), ivec3(-1, -1, -1),
        ivec3(9, 2, 1), ivec3(9, 11, 2), ivec3(9, 4, 11), ivec3(7, 11, 4), ivec3(5, 10, 6),
        ivec3(8, 4, 7), ivec3(3, 11, 5), ivec3(3, 5, 1), ivec3(5, 11, 6), ivec3(-1, -1, -1),
        ivec3(5, 1, 11), ivec3(5, 11, 6), ivec3(1, 0, 11), ivec3(7, 11, 4), ivec3(0, 4, 11),
        ivec3(0, 5, 9), ivec3(0, 6, 5), ivec3(0, 3, 6), ivec3(11, 6, 3), ivec3(8, 4, 7),
        ivec3(6, 5, 9), ivec3(6, 9, 11), ivec3(4, 7, 9), ivec3(7, 11, 9), ivec3(-1, -1, -1),
        ivec3(10, 4, 9), ivec3(6, 4, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 10, 6), ivec3(4, 9, 10), ivec3(0, 8, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 0, 1), ivec3(10, 6, 0), ivec3(6, 4, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 3, 1), ivec3(8, 1, 6), ivec3(8, 6, 4), ivec3(6, 1, 10), ivec3(-1, -1, -1),
        ivec3(1, 4, 9), ivec3(1, 2, 4), ivec3(2, 6, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 0, 8), ivec3(1, 2, 9), ivec3(2, 4, 9), ivec3(2, 6, 4), ivec3(-1, -1, -1),
        ivec3(0, 2, 4), ivec3(4, 2, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 3, 2), ivec3(8, 2, 4), ivec3(4, 2, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 4, 9), ivec3(10, 6, 4), ivec3(11, 2, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 2), ivec3(2, 8, 11), ivec3(4, 9, 10), ivec3(4, 10, 6), ivec3(-1, -1, -1),
        ivec3(3, 11, 2), ivec3(0, 1, 6), ivec3(0, 6, 4), ivec3(6, 1, 10), ivec3(-1, -1, -1),
        ivec3(6, 4, 1), ivec3(6, 1, 10), ivec3(4, 8, 1), ivec3(2, 1, 11), ivec3(8, 11, 1),
        ivec3(9, 6, 4), ivec3(9, 3, 6), ivec3(9, 1, 3), ivec3(11, 6, 3), ivec3(-1, -1, -1),
        ivec3(8, 11, 1), ivec3(8, 1, 0), ivec3(11, 6, 1), ivec3(9, 1, 4), ivec3(6, 4, 1),
        ivec3(3, 11, 6), ivec3(3, 6, 0), ivec3(0, 6, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(6, 4, 8), ivec3(11, 6, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(7, 10, 6), ivec3(7, 8, 10), ivec3(8, 9, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 7, 3), ivec3(0, 10, 7), ivec3(0, 9, 10), ivec3(6, 7, 10), ivec3(-1, -1, -1),
        ivec3(10, 6, 7), ivec3(1, 10, 7), ivec3(1, 7, 8), ivec3(1, 8, 0), ivec3(-1, -1, -1),
        ivec3(10, 6, 7), ivec3(10, 7, 1), ivec3(1, 7, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 6), ivec3(1, 6, 8), ivec3(1, 8, 9), ivec3(8, 6, 7), ivec3(-1, -1, -1),
        ivec3(2, 6, 9), ivec3(2, 9, 1), ivec3(6, 7, 9), ivec3(0, 9, 3), ivec3(7, 3, 9),
        ivec3(7, 8, 0), ivec3(7, 0, 6), ivec3(6, 0, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(7, 3, 2), ivec3(6, 7, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 3, 11), ivec3(10, 6, 8), ivec3(10, 8, 9), ivec3(8, 6, 7), ivec3(-1, -1, -1),
        ivec3(2, 0, 7), ivec3(2, 7, 11), ivec3(0, 9, 7), ivec3(6, 7, 10), ivec3(9, 10, 7),
        ivec3(1, 8, 0), ivec3(1, 7, 8), ivec3(1, 10, 7), ivec3(6, 7, 10), ivec3(2, 3, 11),
        ivec3(11, 2, 1), ivec3(11, 1, 7), ivec3(10, 6, 1), ivec3(6, 7, 1), ivec3(-1, -1, -1),
        ivec3(8, 9, 6), ivec3(8, 6, 7), ivec3(9, 1, 6), ivec3(11, 6, 3), ivec3(1, 3, 6),
        ivec3(0, 9, 1), ivec3(11, 6, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(7, 8, 0), ivec3(7, 0, 6), ivec3(3, 11, 0), ivec3(11, 6, 0), ivec3(-1, -1, -1),
        ivec3(7, 11, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(7, 6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 0, 8), ivec3(11, 7, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 1, 9), ivec3(11, 7, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 1, 9), ivec3(8, 3, 1), ivec3(11, 7, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 1, 2), ivec3(6, 11, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 10), ivec3(3, 0, 8), ivec3(6, 11, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 9, 0), ivec3(2, 10, 9), ivec3(6, 11, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(6, 11, 7), ivec3(2, 10, 3), ivec3(10, 8, 3), ivec3(10, 9, 8), ivec3(-1, -1, -1),
        ivec3(7, 2, 3), ivec3(6, 2, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(7, 0, 8), ivec3(7, 6, 0), ivec3(6, 2, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 7, 6), ivec3(2, 3, 7), ivec3(0, 1, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 6, 2), ivec3(1, 8, 6), ivec3(1, 9, 8), ivec3(8, 7, 6), ivec3(-1, -1, -1),
        ivec3(10, 7, 6), ivec3(10, 1, 7), ivec3(1, 3, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 7, 6), ivec3(1, 7, 10), ivec3(1, 8, 7), ivec3(1, 0, 8), ivec3(-1, -1, -1),
        ivec3(0, 3, 7), ivec3(0, 7, 10), ivec3(0, 10, 9), ivec3(6, 10, 7), ivec3(-1, -1, -1),
        ivec3(7, 6, 10), ivec3(7, 10, 8), ivec3(8, 10, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(6, 8, 4), ivec3(11, 8, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 6, 11), ivec3(3, 0, 6), ivec3(0, 4, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 6, 11), ivec3(8, 4, 6), ivec3(9, 0, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 4, 6), ivec3(9, 6, 3), ivec3(9, 3, 1), ivec3(11, 3, 6), ivec3(-1, -1, -1),
        ivec3(6, 8, 4), ivec3(6, 11, 8), ivec3(2, 10, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 10), ivec3(3, 0, 11), ivec3(0, 6, 11), ivec3(0, 4, 6), ivec3(-1, -1, -1),
        ivec3(4, 11, 8), ivec3(4, 6, 11), ivec3(0, 2, 9), ivec3(2, 10, 9), ivec3(-1, -1, -1),
        ivec3(10, 9, 3), ivec3(10, 3, 2), ivec3(9, 4, 3), ivec3(11, 3, 6), ivec3(4, 6, 3),
        ivec3(8, 2, 3), ivec3(8, 4, 2), ivec3(4, 6, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 4, 2), ivec3(4, 6, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 9, 0), ivec3(2, 3, 4), ivec3(2, 4, 6), ivec3(4, 3, 8), ivec3(-1, -1, -1),
        ivec3(1, 9, 4), ivec3(1, 4, 2), ivec3(2, 4, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 1, 3), ivec3(8, 6, 1), ivec3(8, 4, 6), ivec3(6, 10, 1), ivec3(-1, -1, -1),
        ivec3(10, 1, 0), ivec3(10, 0, 6), ivec3(6, 0, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 6, 3), ivec3(4, 3, 8), ivec3(6, 10, 3), ivec3(0, 3, 9), ivec3(10, 9, 3),
        ivec3(10, 9, 4), ivec3(6, 10, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 9, 5), ivec3(7, 6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 3), ivec3(4, 9, 5), ivec3(11, 7, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 0, 1), ivec3(5, 4, 0), ivec3(7, 6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(11, 7, 6), ivec3(8, 3, 4), ivec3(3, 5, 4), ivec3(3, 1, 5), ivec3(-1, -1, -1),
        ivec3(9, 5, 4), ivec3(10, 1, 2), ivec3(7, 6, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(6, 11, 7), ivec3(1, 2, 10), ivec3(0, 8, 3), ivec3(4, 9, 5), ivec3(-1, -1, -1),
        ivec3(7, 6, 11), ivec3(5, 4, 10), ivec3(4, 2, 10), ivec3(4, 0, 2), ivec3(-1, -1, -1),
        ivec3(3, 4, 8), ivec3(3, 5, 4), ivec3(3, 2, 5), ivec3(10, 5, 2), ivec3(11, 7, 6),
        ivec3(7, 2, 3), ivec3(7, 6, 2), ivec3(5, 4, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 5, 4), ivec3(0, 8, 6), ivec3(0, 6, 2), ivec3(6, 8, 7), ivec3(-1, -1, -1),
        ivec3(3, 6, 2), ivec3(3, 7, 6), ivec3(1, 5, 0), ivec3(5, 4, 0), ivec3(-1, -1, -1),
        ivec3(6, 2, 8), ivec3(6, 8, 7), ivec3(2, 1, 8), ivec3(4, 8, 5), ivec3(1, 5, 8),
        ivec3(9, 5, 4), ivec3(10, 1, 6), ivec3(1, 7, 6), ivec3(1, 3, 7), ivec3(-1, -1, -1),
        ivec3(1, 6, 10), ivec3(1, 7, 6), ivec3(1, 0, 7), ivec3(8, 7, 0), ivec3(9, 5, 4),
        ivec3(4, 0, 10), ivec3(4, 10, 5), ivec3(0, 3, 10), ivec3(6, 10, 7), ivec3(3, 7, 10),
        ivec3(7, 6, 10), ivec3(7, 10, 8), ivec3(5, 4, 10), ivec3(4, 8, 10), ivec3(-1, -1, -1),
        ivec3(6, 9, 5), ivec3(6, 11, 9), ivec3(11, 8, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 6, 11), ivec3(0, 6, 3), ivec3(0, 5, 6), ivec3(0, 9, 5), ivec3(-1, -1, -1),
        ivec3(0, 11, 8), ivec3(0, 5, 11), ivec3(0, 1, 5), ivec3(5, 6, 11), ivec3(-1, -1, -1),
        ivec3(6, 11, 3), ivec3(6, 3, 5), ivec3(5, 3, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 10), ivec3(9, 5, 11), ivec3(9, 11, 8), ivec3(11, 5, 6), ivec3(-1, -1, -1),
        ivec3(0, 11, 3), ivec3(0, 6, 11), ivec3(0, 9, 6), ivec3(5, 6, 9), ivec3(1, 2, 10),
        ivec3(11, 8, 5), ivec3(11, 5, 6), ivec3(8, 0, 5), ivec3(10, 5, 2), ivec3(0, 2, 5),
        ivec3(6, 11, 3), ivec3(6, 3, 5), ivec3(2, 10, 3), ivec3(10, 5, 3), ivec3(-1, -1, -1),
        ivec3(5, 8, 9), ivec3(5, 2, 8), ivec3(5, 6, 2), ivec3(3, 8, 2), ivec3(-1, -1, -1),
        ivec3(9, 5, 6), ivec3(9, 6, 0), ivec3(0, 6, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 5, 8), ivec3(1, 8, 0), ivec3(5, 6, 8), ivec3(3, 8, 2), ivec3(6, 2, 8),
        ivec3(1, 5, 6), ivec3(2, 1, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 3, 6), ivec3(1, 6, 10), ivec3(3, 8, 6), ivec3(5, 6, 9), ivec3(8, 9, 6),
        ivec3(10, 1, 0), ivec3(10, 0, 6), ivec3(9, 5, 0), ivec3(5, 6, 0), ivec3(-1, -1, -1),
        ivec3(0, 3, 8), ivec3(5, 6, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 5, 6), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(11, 5, 10), ivec3(7, 5, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(11, 5, 10), ivec3(11, 7, 5), ivec3(8, 3, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 11, 7), ivec3(5, 10, 11), ivec3(1, 9, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(10, 7, 5), ivec3(10, 11, 7), ivec3(9, 8, 1), ivec3(8, 3, 1), ivec3(-1, -1, -1),
        ivec3(11, 1, 2), ivec3(11, 7, 1), ivec3(7, 5, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 3), ivec3(1, 2, 7), ivec3(1, 7, 5), ivec3(7, 2, 11), ivec3(-1, -1, -1),
        ivec3(9, 7, 5), ivec3(9, 2, 7), ivec3(9, 0, 2), ivec3(2, 11, 7), ivec3(-1, -1, -1),
        ivec3(7, 5, 2), ivec3(7, 2, 11), ivec3(5, 9, 2), ivec3(3, 2, 8), ivec3(9, 8, 2),
        ivec3(2, 5, 10), ivec3(2, 3, 5), ivec3(3, 7, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 2, 0), ivec3(8, 5, 2), ivec3(8, 7, 5), ivec3(10, 2, 5), ivec3(-1, -1, -1),
        ivec3(9, 0, 1), ivec3(5, 10, 3), ivec3(5, 3, 7), ivec3(3, 10, 2), ivec3(-1, -1, -1),
        ivec3(9, 8, 2), ivec3(9, 2, 1), ivec3(8, 7, 2), ivec3(10, 2, 5), ivec3(7, 5, 2),
        ivec3(1, 3, 5), ivec3(3, 7, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 7), ivec3(0, 7, 1), ivec3(1, 7, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 0, 3), ivec3(9, 3, 5), ivec3(5, 3, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 8, 7), ivec3(5, 9, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 8, 4), ivec3(5, 10, 8), ivec3(10, 11, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(5, 0, 4), ivec3(5, 11, 0), ivec3(5, 10, 11), ivec3(11, 3, 0), ivec3(-1, -1, -1),
        ivec3(0, 1, 9), ivec3(8, 4, 10), ivec3(8, 10, 11), ivec3(10, 4, 5), ivec3(-1, -1, -1),
        ivec3(10, 11, 4), ivec3(10, 4, 5), ivec3(11, 3, 4), ivec3(9, 4, 1), ivec3(3, 1, 4),
        ivec3(2, 5, 1), ivec3(2, 8, 5), ivec3(2, 11, 8), ivec3(4, 5, 8), ivec3(-1, -1, -1),
        ivec3(0, 4, 11), ivec3(0, 11, 3), ivec3(4, 5, 11), ivec3(2, 11, 1), ivec3(5, 1, 11),
        ivec3(0, 2, 5), ivec3(0, 5, 9), ivec3(2, 11, 5), ivec3(4, 5, 8), ivec3(11, 8, 5),
        ivec3(9, 4, 5), ivec3(2, 11, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 5, 10), ivec3(3, 5, 2), ivec3(3, 4, 5), ivec3(3, 8, 4), ivec3(-1, -1, -1),
        ivec3(5, 10, 2), ivec3(5, 2, 4), ivec3(4, 2, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 10, 2), ivec3(3, 5, 10), ivec3(3, 8, 5), ivec3(4, 5, 8), ivec3(0, 1, 9),
        ivec3(5, 10, 2), ivec3(5, 2, 4), ivec3(1, 9, 2), ivec3(9, 4, 2), ivec3(-1, -1, -1),
        ivec3(8, 4, 5), ivec3(8, 5, 3), ivec3(3, 5, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 4, 5), ivec3(1, 0, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(8, 4, 5), ivec3(8, 5, 3), ivec3(9, 0, 5), ivec3(0, 3, 5), ivec3(-1, -1, -1),
        ivec3(9, 4, 5), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 11, 7), ivec3(4, 9, 11), ivec3(9, 10, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 8, 3), ivec3(4, 9, 7), ivec3(9, 11, 7), ivec3(9, 10, 11), ivec3(-1, -1, -1),
        ivec3(1, 10, 11), ivec3(1, 11, 4), ivec3(1, 4, 0), ivec3(7, 4, 11), ivec3(-1, -1, -1),
        ivec3(3, 1, 4), ivec3(3, 4, 8), ivec3(1, 10, 4), ivec3(7, 4, 11), ivec3(10, 11, 4),
        ivec3(4, 11, 7), ivec3(9, 11, 4), ivec3(9, 2, 11), ivec3(9, 1, 2), ivec3(-1, -1, -1),
        ivec3(9, 7, 4), ivec3(9, 11, 7), ivec3(9, 1, 11), ivec3(2, 11, 1), ivec3(0, 8, 3),
        ivec3(11, 7, 4), ivec3(11, 4, 2), ivec3(2, 4, 0), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(11, 7, 4), ivec3(11, 4, 2), ivec3(8, 3, 4), ivec3(3, 2, 4), ivec3(-1, -1, -1),
        ivec3(2, 9, 10), ivec3(2, 7, 9), ivec3(2, 3, 7), ivec3(7, 4, 9), ivec3(-1, -1, -1),
        ivec3(9, 10, 7), ivec3(9, 7, 4), ivec3(10, 2, 7), ivec3(8, 7, 0), ivec3(2, 0, 7),
        ivec3(3, 7, 10), ivec3(3, 10, 2), ivec3(7, 4, 10), ivec3(1, 10, 0), ivec3(4, 0, 10),
        ivec3(1, 10, 2), ivec3(8, 7, 4), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 9, 1), ivec3(4, 1, 7), ivec3(7, 1, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 9, 1), ivec3(4, 1, 7), ivec3(0, 8, 1), ivec3(8, 7, 1), ivec3(-1, -1, -1),
        ivec3(4, 0, 3), ivec3(7, 4, 3), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(4, 8, 7), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 10, 8), ivec3(10, 11, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 0, 9), ivec3(3, 9, 11), ivec3(11, 9, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 1, 10), ivec3(0, 10, 8), ivec3(8, 10, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 1, 10), ivec3(11, 3, 10), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 2, 11), ivec3(1, 11, 9), ivec3(9, 11, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 0, 9), ivec3(3, 9, 11), ivec3(1, 2, 9), ivec3(2, 11, 9), ivec3(-1, -1, -1),
        ivec3(0, 2, 11), ivec3(8, 0, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(3, 2, 11), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 3, 8), ivec3(2, 8, 10), ivec3(10, 8, 9), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(9, 10, 2), ivec3(0, 9, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(2, 3, 8), ivec3(2, 8, 10), ivec3(0, 1, 8), ivec3(1, 10, 8), ivec3(-1, -1, -1),
        ivec3(1, 10, 2), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(1, 3, 8), ivec3(9, 1, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 9, 1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(0, 3, 8), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1),
        ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1), ivec3(-1, -1, -1) };


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
    m_program->setUniform("a_edgeConnectList", m_edgeConnectList);
    m_program->setUniform("a_caseToNumPolys", m_caseToNumPolys);
    m_program->setUniform("a_edgeToVertices", m_edgeToVertices);
    m_densities->bindActive(GL_TEXTURE0);
    m_program->setUniform("densities", 0);

	m_vao->bind();
	m_vao->drawArrays(GL_POINTS, 0, m_size);
	m_vao->unbind();

    m_program->release();

    Framebuffer::unbind(GL_FRAMEBUFFER);
}
