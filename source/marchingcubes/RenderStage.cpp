#include "RenderStage.h"


#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>
#include <gloperate/painter/AbstractProjectionCapability.h>
#include <gloperate/painter/Camera.h>

using namespace globjects;

RenderStage::RenderStage()
:   AbstractStage("Render")
{
    addInput("viewport", viewport);
    addInput("camera", camera);
    addInput("projection", projection);
}

void RenderStage::initialize()
{

}

void RenderStage::process()
{
    auto rerender = false;

    if (viewport.hasChanged())
    {
        rerender = true;
    }

    if (camera.hasChanged() || projection.hasChanged())
    {
        rerender = true;
    }

    if (rerender)
    {
        render();

        invalidateOutputs();
    }

}

void RenderStage::render()
{

}
