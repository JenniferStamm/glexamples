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
, rotationVector1(glm::vec3(1, 0.3, 0.5))
, rotationVector2(glm::vec3(0.1, 0.5, 0.3))
, warpFactor(3.4)
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
    renderStage->rotationVector1 = rotationVector1;
    renderStage->rotationVector2 = rotationVector2;
    renderStage->warpFactor = warpFactor;
	renderStage->colorTexture = colorTexture;
	renderStage->groundTexture = groundTexture;
    renderStage->chunksToAdd = addChunksStage->chunksToAdd;

    addStages(
        std::move(addChunksStage),
		std::move(renderStage)
	);
}

