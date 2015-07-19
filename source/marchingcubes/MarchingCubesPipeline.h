#pragma once

#include <globjects/base/ref_ptr.h>
#include <globjects/Texture.h>

#include <gloperate/pipeline/AbstractPipeline.h>
#include <gloperate/pipeline/Data.h>

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

    gloperate::Data<bool> useMipMap;
    gloperate::Data<glm::vec3> rotationVector1;
    gloperate::Data<glm::vec3> rotationVector2;
    gloperate::Data<float> warpFactor;

	gloperate::Data<globjects::ref_ptr<globjects::Texture>> colorTexture;
	gloperate::Data<globjects::ref_ptr<globjects::Texture>> groundTexture;

    gloperate::Data<gloperate::AbstractTargetFramebufferCapability *> targetFBO;

};
