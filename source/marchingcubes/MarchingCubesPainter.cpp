#include "MarchingCubesPainter.h"

#include <reflectionzeug/property/extensions/GlmProperties.h>

#include <globjects/globjects.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/resources/ResourceManager.h>

using namespace gl;
using namespace glm;
using namespace globjects;

MarchingCubes::MarchingCubes(gloperate::ResourceManager & resourceManager, const std::string & relDataPath)
:   PipelinePainter("MarchingCubesExample", resourceManager, relDataPath, m_pipeline)
,   m_targetFramebufferCapability{ addCapability(new gloperate::TargetFramebufferCapability()) }
,   m_viewportCapability{addCapability(new gloperate::ViewportCapability())}
,   m_projectionCapability{addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability))}
,   m_cameraCapability{addCapability(new gloperate::CameraCapability())}
{
    globjects::init();

    m_pipeline.targetFBO.setData(m_targetFramebufferCapability);
    m_pipeline.viewport.setData(m_viewportCapability);
    m_pipeline.camera.setData(m_cameraCapability);
	m_pipeline.projection.setData(m_projectionCapability); 
    m_pipeline.resourceManager.setData(&m_resourceManager);

    m_targetFramebufferCapability->changed.connect([this]() { m_pipeline.targetFBO.invalidate(); });
    m_viewportCapability->changed.connect([this]() { m_pipeline.viewport.invalidate(); });
    m_cameraCapability->changed.connect([this]() { m_pipeline.camera.invalidate(); });
    m_projectionCapability->changed.connect([this]() { m_pipeline.projection.invalidate(); });


    auto renderingGroup = addGroup("Rendering");
    renderingGroup->addProperty(createProperty("UseMipMap", m_pipeline.useMipMap));

    auto terrainGroup = addGroup("Terrain Generation");
    terrainGroup->addProperty(createProperty("Rotation Vector 1", m_pipeline.rotationVector1));
    terrainGroup->addProperty(createProperty("Rotation Vector 2", m_pipeline.rotationVector2));
    terrainGroup->addProperty(createProperty("Warp Factor", m_pipeline.warpFactor));
    terrainGroup->addProperty(createProperty("Remove Floaters", m_pipeline.removeFloaters));


}

MarchingCubes::~MarchingCubes() = default;