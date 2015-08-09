#include "MarchingCubes.h"

#include <reflectionzeug/property/extensions/GlmProperties.h>

#include <globjects/globjects.h>

#include <gloperate/base/RenderTargetType.h>
#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/TypedRenderTargetCapability.h>
#include <gloperate/painter/InputCapability.h>
#include <gloperate/resources/ResourceManager.h>
#include <gloperate/navigation/CoordinateProvider.h>

#include <glbinding/gl/enum.h>

using namespace gl;
using namespace glm;
using namespace globjects;

MarchingCubes::MarchingCubes(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
:   PipelinePainter("MarchingCubesExample", resourceManager, relDataPath, m_pipeline)
,   m_targetFramebufferCapability{ addCapability(new gloperate::TargetFramebufferCapability()) }
,   m_viewportCapability{addCapability(new gloperate::ViewportCapability())}
,   m_projectionCapability{addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability))}
,   m_cameraCapability{addCapability(new gloperate::CameraCapability())}
,   m_inputCapability{ addCapability(new gloperate::InputCapability()) }
, m_renderTargetCapability{ addCapability(new gloperate::TypedRenderTargetCapability()) }
{
    globjects::init();


    m_coordinateProvider = new gloperate::CoordinateProvider(
        m_cameraCapability, m_projectionCapability, m_viewportCapability, m_renderTargetCapability);

    m_pipeline.targetFBO.setData(m_targetFramebufferCapability);
    m_pipeline.viewport.setData(m_viewportCapability);
    m_pipeline.camera.setData(m_cameraCapability);
	m_pipeline.projection.setData(m_projectionCapability); 
    m_pipeline.resourceManager.setData(&m_resourceManager);
    m_pipeline.renderTargets.setData(m_renderTargetCapability);
    m_pipeline.input.setData(m_inputCapability);
    m_pipeline.coordinateProvider.setData(m_coordinateProvider);

    m_targetFramebufferCapability->changed.connect([this]() { m_pipeline.targetFBO.invalidate(); });
    m_viewportCapability->changed.connect([this]() { m_pipeline.viewport.invalidate(); });
    m_cameraCapability->changed.connect([this]() { m_pipeline.camera.invalidate(); });
    m_projectionCapability->changed.connect([this]() { m_pipeline.projection.invalidate(); });
    m_renderTargetCapability->changed.connect([this]() { m_pipeline.renderTargets.invalidate(); });
    m_inputCapability->changed.connect([this]() { m_pipeline.input.invalidate(); });

        
    //m_pipeline.targetFBO.invalidated

    //m_renderTargetCapability->changed.connect([this]() { this->onTargetFramebufferChanged(); });
    //m_cameraCapability->changed.connect([this]() { this->onTargetFramebufferChanged(); });

    auto debugGroup = addGroup("Debugging");
    debugGroup->addProperty(createProperty("Show Grid", m_pipeline.showGrid));
    debugGroup->addProperty(createProperty("Show Wireframe", m_pipeline.showWireframe));
    debugGroup->addProperty(createProperty("Freeze Chunk Loading", m_pipeline.freezeChunkLoading));

    auto terrainGroup = addGroup("Terrain Generation");
    terrainGroup->addProperty(createProperty("Terrain Type", m_pipeline.terrainType));
    terrainGroup->addProperty(createProperty("Base Texture", m_pipeline.userBaseTextureFilePath));
    terrainGroup->addProperty(createProperty("Extra Texture", m_pipeline.userExtraTextureFilePath));
    terrainGroup->addProperty(createProperty("Density Generation Shader", m_pipeline.userDensityGenererationShaderFilePath));
    terrainGroup->addProperty(createProperty("Fragment Shader", m_pipeline.userFragmentShaderFilePath));
    terrainGroup->addProperty(createProperty("Rotation Vector 1", m_pipeline.userRotationVector1));
    terrainGroup->addProperty(createProperty("Rotation Vector 2", m_pipeline.userRotationVector2));
    terrainGroup->addProperty(createProperty("Warp Factor", m_pipeline.userWarpFactor));
    terrainGroup->addProperty(createProperty("Remove Floaters", m_pipeline.removeFloaters));

    auto prettyTerrainGroup = addGroup("Pretty Terrain");
    prettyTerrainGroup->addProperty(createProperty("Light", m_pipeline.useShadow));
    prettyTerrainGroup->addProperty(createProperty("Occlusion", m_pipeline.useOcclusion));
    prettyTerrainGroup->addProperty(createProperty("Use Ground Texture", m_pipeline.useGroundTexture));
    prettyTerrainGroup->addProperty(createProperty("Mip Mapping", m_pipeline.useMipMap));
    prettyTerrainGroup->addProperty(createProperty("Use Striation Texture", m_pipeline.useStriationTexture));

    auto terrainModificationGroup = addGroup("Terrain Modification");
    auto radiusProperty = createProperty("Modification Radius", m_pipeline.modificationRadius);
    radiusProperty->setOptions({
        { "minimum", reflectionzeug::Variant(0.05f) },
        { "maximum", reflectionzeug::Variant(1.0f) },
        { "step", reflectionzeug::Variant(0.05f) }
    });
    terrainModificationGroup->addProperty(radiusProperty);
}

MarchingCubes::~MarchingCubes() = default;