#pragma once

#include <gloperate/pipeline/AbstractPipeline.h>
#include <gloperate/pipeline/Data.h>

#include <glm/vec3.hpp>

namespace gloperate
{

class AbstractTargetFramebufferCapability;
class PerspectiveProjectionCapability;
class AbstractCameraCapability;
class AbstractViewportCapability;
class AbstractVirtualTimeCapability;
class AbstractTypedRenderTargetCapability;
class ResourceManager;

} // namespace gloperate



class MarchingCubesPipeline : public gloperate::AbstractPipeline
{
public:
	MarchingCubesPipeline();
    virtual ~MarchingCubesPipeline() = default;

public:
    gloperate::Data<gloperate::AbstractViewportCapability *> viewport;
    gloperate::Data<gloperate::AbstractCameraCapability *> camera;
    gloperate::Data<gloperate::PerspectiveProjectionCapability *> projection;
    gloperate::Data<gloperate::ResourceManager *> resourceManager;

    gloperate::Data<bool> useMipMap;
    gloperate::Data<glm::vec3> rotationVector1;
    gloperate::Data<glm::vec3> rotationVector2;
    gloperate::Data<float> warpFactor;
    gloperate::Data<bool> removeFloaters;

    gloperate::Data<gloperate::AbstractTargetFramebufferCapability *> targetFBO;

};
