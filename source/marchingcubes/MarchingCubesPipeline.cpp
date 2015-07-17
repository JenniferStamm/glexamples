#include "MarchingCubesPipeline.h"

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>

#include "RenderStage.h"


MarchingCubesPipeline::MarchingCubesPipeline()
: AbstractPipeline("MarchingCubes")
, viewport(nullptr)
, targetFBO(nullptr)
{
    auto renderStage = new RenderStage();

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;

    //addStages();
}
