#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/VertexArray.h>

#include <gloperate/painter/Painter.h>

#include <reflectionzeug/Color.h>

namespace globjects
{
    class Program;
	class VertexArray;
	class Buffer;
	class Texture;
    class TransformFeedback;
}

namespace gloperate
{
    class AdaptiveGrid;
    class AbstractTargetFramebufferCapability;
    class AbstractViewportCapability;
    class AbstractPerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class MarchingCubes : public gloperate::Painter
{
public:
	MarchingCubes(gloperate::ResourceManager & resourceManager);
	virtual ~MarchingCubes();

    void setupProjection();

    reflectionzeug::Color cubeColor() const;
    void setCubeColor(reflectionzeug::Color cubeColor);

protected:
    virtual void onInitialize() override;
    virtual void onPaint() override;

protected:
    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;
    globjects::ref_ptr<globjects::Program> m_program;
    gl::GLint m_transformLocation;

	globjects::ref_ptr<globjects::VertexArray> m_vao;

	globjects::ref_ptr<globjects::VertexArray> m_positionVao;

	globjects::ref_ptr<globjects::Buffer> m_positions;

    reflectionzeug::Color m_cubeColor;

	gl::GLsizei m_size;

	globjects::ref_ptr<globjects::Buffer> m_densities;
    glm::ivec3 m_dimension;
    globjects::ref_ptr<globjects::Buffer> m_edgeConnectList;
    std::vector<int> m_caseToNumPolys;
    std::vector<glm::ivec2> m_edgeToVertices;

    globjects::ref_ptr<globjects::TransformFeedback> m_transformFeedback;
    globjects::ref_ptr<globjects::Program> m_transformFeedbackProgram;
};
