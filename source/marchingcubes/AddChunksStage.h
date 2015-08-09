#pragma once

#include <queue>

#include <glm/vec3.hpp>

#include <gloperate/pipeline/AbstractStage.h>
#include <gloperate/pipeline/Data.h>
#include <gloperate/pipeline/InputSlot.h>


namespace gloperate
{
    class AbstractCameraCapability;
    class PerspectiveProjectionCapability;
}

class Chunk;

class AddChunksStage : public gloperate::AbstractStage
{
public:
    AddChunksStage();

    virtual void initialize() override;

public:
    gloperate::InputSlot<gloperate::AbstractCameraCapability *> camera;
    gloperate::InputSlot<gloperate::PerspectiveProjectionCapability *> projection;
    gloperate::InputSlot<bool> freezeChunkLoading;

    gloperate::Data<std::queue<glm::vec3>> chunksToAdd;

protected:
    virtual void process() override;

};

