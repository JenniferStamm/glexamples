#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/VertexArray.h>

#include <gloperate/painter/Painter.h>

#include <reflectionzeug/Color.h>

class Chunk;
class ChunkRenderer;

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
    
protected:
    void setupGrid();
    void setupProjection();
    void setupOpenGLState();

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

    std::vector<globjects::ref_ptr<Chunk>> m_chunks;
    globjects::ref_ptr<ChunkRenderer> m_chunkRenderer;
};
