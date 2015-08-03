#include "MarchingCubesPipeline.h"

#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractVirtualTimeCapability.h>
#include <gloperate/painter/AbstractTargetFramebufferCapability.h>
#include <gloperate/painter/InputCapability.h>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/navigation/CoordinateProvider.h>

#include "AddChunksStage.h"
#include "ManageChunksStage.h"
#include "RenderStage.h"


MarchingCubesPipeline::MarchingCubesPipeline()
: AbstractPipeline("MarchingCubes")
, viewport(nullptr)
, useMipMap(false)
, rotationVector1(glm::vec3(1, 0.3, 0.5))
, rotationVector2(glm::vec3(0.1, 0.5, 0.3))
, warpFactor(3.4f)
, removeFloaters(false)
, targetFBO(nullptr)
, renderTargets(nullptr)
, useShadow(false)
, useOcclusion(false)
, groundTextureFilePath("data/marchingcubes/ground.png")
, useGroundTexture(false)
, striationTextureFilePath("data/marchingcubes/terrain_color.jpg")
, useStriationTexture(false)
{
    auto addChunksStage = new AddChunksStage();
    auto manageChunksStage = new ManageChunksStage();
    auto renderStage = new RenderStage();

    addChunksStage->camera = camera;

    manageChunksStage->camera = camera;
    manageChunksStage->input = input;
    manageChunksStage->coordinateProvider = coordinateProvider;
    manageChunksStage->chunksToAdd = addChunksStage->chunksToAdd;
    manageChunksStage->rotationVector1 = rotationVector1;
    manageChunksStage->rotationVector2 = rotationVector2;
    manageChunksStage->warpFactor = warpFactor;
    manageChunksStage->removeFloaters = removeFloaters;

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;
    renderStage->targetFBO = targetFBO;
    renderStage->renderTargets = renderTargets;
    renderStage->useMipMap = useMipMap;
    renderStage->resourceManager = resourceManager;
    renderStage->useShadow = useShadow;
    renderStage->useOcclusion = useOcclusion;
    renderStage->groundTextureFilePath = groundTextureFilePath;
    renderStage->useGroundTexture = useGroundTexture;
    renderStage->striationTextureFilePath = striationTextureFilePath;
    renderStage->useStriationTexture = useStriationTexture;
    renderStage->chunks = manageChunksStage->chunks;

    addStages(
        std::move(addChunksStage),
        std::move(manageChunksStage),
		std::move(renderStage)
	);
}

