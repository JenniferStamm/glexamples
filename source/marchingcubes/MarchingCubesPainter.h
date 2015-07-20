#pragma once

#include <gloperate/pipeline/PipelinePainter.h>

#include "MarchingCubesPipeline.h"

namespace gloperate
{
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

protected:
    /* pipeline*/
    MarchingCubesPipeline m_pipeline;

    /* capabilities */
    gloperate::AbstractTargetFramebufferCapability * m_targetFramebufferCapability;
    gloperate::AbstractViewportCapability * m_viewportCapability;
    gloperate::PerspectiveProjectionCapability * m_projectionCapability;
    gloperate::AbstractCameraCapability * m_cameraCapability;

};
