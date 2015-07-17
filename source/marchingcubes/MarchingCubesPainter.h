#pragma once

#include <memory>
#include <queue>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <vec3_hash.h>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/VertexArray.h>

#include <gloperate/pipeline/PipelinePainter.h>

#include <reflectionzeug/base/Color.h>

#include "MarchingCubesPipeline.h"

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

class MarchingCubes : public gloperate::PipelinePainter
{
public:
	MarchingCubes(gloperate::ResourceManager & resourceManager, const std::string & relDataPath);
	virtual ~MarchingCubes();

    virtual void onInitialize() override;
    virtual void onPaint() override;

	bool useMipMap() const;
	void setUseMipMap(bool useMipMap);
    
protected:
    void setupGrid();
    void setupProjection();
    void setupOpenGLState();


protected:
    /* pipeline*/
    MarchingCubesPipeline m_pipeline;

    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::AbstractPerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

    /* members */
    globjects::ref_ptr<gloperate::AdaptiveGrid> m_grid;

    std::unordered_map<glm::vec3, globjects::ref_ptr<Chunk>> m_chunks;
    globjects::ref_ptr<ChunkRenderer> m_chunkRenderer;

    std::queue<glm::vec3> m_chunkQueue;

	/* parameters */
	bool m_useMipMap;
	bool m_useMipMapChanged;
};
