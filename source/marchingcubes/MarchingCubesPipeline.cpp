#include "MarchingCubesPipeline.h"

#include <globjects/Texture.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/resources/ResourceManager.h>

#include "RenderStage.h"


MarchingCubesPipeline::MarchingCubesPipeline()
: AbstractPipeline("MarchingCubes")
, viewport(nullptr)
, useMipMap(true)
, targetFBO(nullptr)
{
    auto renderStage = new RenderStage();

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;
    renderStage->targetFBO = targetFBO;
    renderStage->useMipMap = useMipMap;
	renderStage->colorTexture = colorTexture;
	renderStage->groundTexture = groundTexture;

    addStages(
		std::move(renderStage)
	);
}
