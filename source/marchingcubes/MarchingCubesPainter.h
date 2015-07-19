#pragma once

#include <memory>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <vec3_hash.h>

#include <glbinding/gl/types.h>

#include <globjects/base/ref_ptr.h>
#include <globjects/VertexArray.h>

#include <gloperate/pipeline/PipelinePainter.h>

#include <reflectionzeug/base/Color.h>

#include "MarchingCubesPipeline.h"


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
    class PerspectiveProjectionCapability;
    class AbstractCameraCapability;
}

class MarchingCubes : public gloperate::PipelinePainter
{
public:
	MarchingCubes(gloperate::ResourceManager & resourceManager, const std::string & relDataPath);
	virtual ~MarchingCubes();

    virtual void onInitialize() override;
    virtual void onPaint() override;

protected:
    /* pipeline*/
    MarchingCubesPipeline m_pipeline;

    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::PerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

};
