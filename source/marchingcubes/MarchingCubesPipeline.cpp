#include "MarchingCubesPipeline.h"

#include <globjects/Texture.h>

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/resources/ResourceManager.h>

#include "AddChunksStage.h"
#include "RenderStage.h"


MarchingCubesPipeline::MarchingCubesPipeline()
: AbstractPipeline("MarchingCubes")
, viewport(nullptr)
, useMipMap(true)
, targetFBO(nullptr)
{
    auto addChunksStage = new AddChunksStage();
    auto renderStage = new RenderStage();

    addChunksStage->camera = camera;

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;
    renderStage->targetFBO = targetFBO;
    renderStage->useMipMap = useMipMap;
	renderStage->colorTexture = colorTexture;
	renderStage->groundTexture = groundTexture;
    renderStage->chunksToAdd = addChunksStage->chunksToAdd;

    addStages(
        std::move(addChunksStage),
		std::move(renderStage)
	);
}

