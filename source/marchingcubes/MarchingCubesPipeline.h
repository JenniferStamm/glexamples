#pragma once

#include <globjects/base/ref_ptr.h>
#include <globjects/Texture.h>

#include <gloperate/pipeline/AbstractPipeline.h>
#include <gloperate/pipeline/Data.h>

namespace gloperate
{

class AbstractTargetFramebufferCapability;
class AbstractPerspectiveProjectionCapability;
class AbstractCameraCapability;
class AbstractViewportCapability;
class AbstractVirtualTimeCapability;
class AbstractTypedRenderTargetCapability;

} // namespace gloperate



class MarchingCubesPipeline : public gloperate::AbstractPipeline
{
public:
    MarchingCubesPipeline();
    virtual ~MarchingCubesPipeline() = default;

public:
    gloperate::Data<gloperate::AbstractViewportCapability *> viewport;
    gloperate::Data<gloperate::AbstractCameraCapability *> camera;
    gloperate::Data<gloperate::AbstractPerspectiveProjectionCapability *> projection;

    gloperate::Data<bool> useMipMap;

    globjects::ref_ptr<globjects::Texture> colorTexture;
    globjects::ref_ptr<globjects::Texture> groundTexture;

    gloperate::Data<gloperate::AbstractTargetFramebufferCapability *> targetFBO;
};
