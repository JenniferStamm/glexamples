#pragma once

#include <memory>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/VertexArray.h>

#include <gloperate/painter/Painter.h>


namespace globjects
{
    class Program;
	class VertexArray;
	class Buffer;
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

	globjects::ref_ptr<globjects::Buffer> m_vertices;

	gl::GLsizei m_size;
};
