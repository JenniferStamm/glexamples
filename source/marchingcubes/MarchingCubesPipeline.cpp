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
#include "TerrainTypeStage.h"
#include "TerrainModificationStage.h"


MarchingCubesPipeline::MarchingCubesPipeline()
: AbstractPipeline("MarchingCubes")
, viewport(nullptr)
, showGrid(false)
, showWireframe(false)
, freezeChunkLoading(false)
, modificationRadius(0.25f)
, useMipMap(true)
, userRotationVector1(glm::vec3(1, 0.3, 0.5))
, userRotationVector2(glm::vec3(0.1, 0.5, 0.3))
, userWarpFactor(3.4f)
, removeFloaters(false)
, targetFBO(nullptr)
, renderTargets(nullptr)
, useShadow(true)
, useOcclusion(true)
, userBaseTextureFilePath("data/marchingcubes/ground.png")
, useGroundTexture(true)
, userExtraTextureFilePath("data/marchingcubes/terrain_color.jpg")
, useStriationTexture(true)
, terrainType(TerrainType::Mossy)
, userFragmentShaderFilePath("data/marchingcubes/marchingcubes.frag")
, userDensityGenererationShaderFilePath("data/marchingcubes/densitygeneration.vert")
{
    auto terrainTypeStage = new TerrainTypeStage();
    auto addChunksStage = new AddChunksStage();
    auto terrainModificationStage = new TerrainModificationStage();
    auto manageChunksStage = new ManageChunksStage();
    auto renderStage = new RenderStage();

    terrainTypeStage->terrainType = terrainType;
    terrainTypeStage->userBaseTextureFilePath = userBaseTextureFilePath;
    terrainTypeStage->userExtraTextureFilePath = userExtraTextureFilePath;
    terrainTypeStage->userFragmentShaderFilePath = userFragmentShaderFilePath;
    terrainTypeStage->userDensityGenererationShaderFilePath = userDensityGenererationShaderFilePath;
    terrainTypeStage->userRotationVector1 = userRotationVector1;
    terrainTypeStage->userRotationVector2 = userRotationVector2;
    terrainTypeStage->userWarpFactor = userWarpFactor;

    addChunksStage->camera = camera;
    addChunksStage->projection = projection;
    addChunksStage->freezeChunkLoading = freezeChunkLoading;

    terrainModificationStage->input = input;

    manageChunksStage->camera = camera;
    manageChunksStage->coordinateProvider = coordinateProvider;
    manageChunksStage->addPosition = terrainModificationStage->addPosition;
    manageChunksStage->removePosition = terrainModificationStage->removePosition;
    manageChunksStage->chunksToAdd = addChunksStage->chunksToAdd;
    manageChunksStage->rotationVector1 = terrainTypeStage->rotationVector1;
    manageChunksStage->rotationVector2 = terrainTypeStage->rotationVector2;
    manageChunksStage->warpFactor = terrainTypeStage->warpFactor;
    manageChunksStage->removeFloaters = removeFloaters;
    manageChunksStage->freezeChunkLoading = freezeChunkLoading;
    manageChunksStage->modificationRadius = modificationRadius;
    manageChunksStage->densityGenererationShaderFilePath = terrainTypeStage->densityGenererationShaderFilePath;

    renderStage->viewport = viewport;
    renderStage->camera = camera;
    renderStage->projection = projection;
    renderStage->targetFBO = targetFBO;
    renderStage->renderTargets = renderTargets;
    renderStage->showGrid = showGrid;
    renderStage->showWireframe = showWireframe;
    renderStage->useMipMap = useMipMap;
    renderStage->resourceManager = resourceManager;
    renderStage->useShadow = useShadow;
    renderStage->useOcclusion = useOcclusion;
    renderStage->baseTextureFilePath = terrainTypeStage->baseTextureFilePath;
    renderStage->useBaseTexture = useGroundTexture;
    renderStage->extraTextureFilePath = terrainTypeStage->extraTextureFilePath;
    renderStage->useExtraTexture = useStriationTexture;
    renderStage->fragmentShaderFilePath = terrainTypeStage->fragmentShaderTextureFilePath;
    renderStage->chunks = manageChunksStage->chunks;

    addStages(
        std::move(terrainTypeStage),
        std::move(addChunksStage),
        std::move(terrainModificationStage),
        std::move(manageChunksStage),
		std::move(renderStage)
	);
}

