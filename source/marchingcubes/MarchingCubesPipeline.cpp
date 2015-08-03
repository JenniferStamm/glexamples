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
#include "TerrainModificationStage.h"


MarchingCubesPipeline::MarchingCubesPipeline()
: AbstractPipeline("MarchingCubes")
, viewport(nullptr)
, showWireframe(false)
, freezeChunkLoading(false)
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
    auto terrainModificationStage = new TerrainModificationStage();
    auto manageChunksStage = new ManageChunksStage();
    auto renderStage = new RenderStage();

    addChunksStage->camera = camera;
    addChunksStage->freezeChunkLoading = freezeChunkLoading;

    terrainModificationStage->input = input;

    manageChunksStage->camera = camera;
    manageChunksStage->coordinateProvider = coordinateProvider;
    manageChunksStage->addPosition = terrainModificationStage->addPosition;
    manageChunksStage->chunksToAdd = addChunksStage->chunksToAdd;
    manageChunksStage->rotationVector1 = rotationVector1;
    manageChunksStage->rotationVector2 = rotationVector2;
    manageChunksStage->warpFactor = warpFactor;
    manageChunksStage->removeFloaters = removeFloaters;
    manageChunksStage->freezeChunkLoading = freezeChunkLoading;

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;
    renderStage->targetFBO = targetFBO;
    renderStage->renderTargets = renderTargets;
    renderStage->showWireframe = showWireframe;
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
        std::move(terrainModificationStage),
        std::move(manageChunksStage),
		std::move(renderStage)
	);
}

