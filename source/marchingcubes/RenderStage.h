#pragma once

#include <globjects/base/ref_ptr.h>


#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>


namespace gloperate
{
    class AbstractViewportCapability;
    class AbstractVirtualTimeCapability;
    class AbstractProjectionCapability;
    class AbstractCameraCapability;
}


class RenderStage : public gloperate::AbstractStage
{
public:
    RenderStage();

    virtual void initialize() override;

public:
    gloperate::InputSlot<gloperate::AbstractViewportCapability *> viewport;
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<gloperate::AbstractProjectionCapability *> projection;


protected:
    virtual void process() override;

    void render();


protected:


};

